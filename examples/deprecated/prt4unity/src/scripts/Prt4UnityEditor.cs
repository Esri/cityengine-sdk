using UnityEngine;
using UnityEditor;
using System;

[CustomEditor(typeof(Prt4Unity))]
public class Prt4UnityEditor : Editor
{
    private Prt4Unity shape;
    private bool showAttributes = true;

    public void OnEnable()
    {
        shape = (Prt4Unity)target;
        if(!shape.IsInitialized())
        {
            EditorUtility.DisplayProgressBar("Prt4Unity", "Reinitializing ...", 0.0f);
            shape.EnsureInit();
            EditorUtility.ClearProgressBar();
        }
    }

    public override void OnInspectorGUI()
    {
        // UI: Rule package
        string inputRulePackage = shape.GetRulePackage();
        if(inputRulePackage == null)
            inputRulePackage = "";
        EditorGUILayout.BeginHorizontal();
        string rulePackage = EditorGUILayout.TextField("Rule package", inputRulePackage);
        if(GUILayout.Button("...", GUILayout.MaxWidth(30)))
        {
            string filename = EditorUtility.OpenFilePanel("Select rule package", inputRulePackage, "rpk");
            if(!System.String.IsNullOrEmpty(filename) && filename != inputRulePackage)
                rulePackage = filename;
        }
        EditorGUILayout.EndHorizontal();
        if(rulePackage != inputRulePackage)
        {
            EditorUtility.DisplayProgressBar("Prt4Unity", "Loading rule package ...", 0.0f);
            shape.LoadRulePackage(rulePackage, Application.dataPath);
            EditorUtility.ClearProgressBar();
        }

        // UI: Rule file and start rule
        string ruleFile = shape.GetRuleFile();
        bool canGenerate = false;
        if(!String.IsNullOrEmpty(ruleFile))
        {
            int start = ruleFile.IndexOf('/') + 1;
            int end = ruleFile.LastIndexOf('.');
            if(end < start)
                end = ruleFile.Length;
            EditorGUILayout.Popup("Rule file", 0, new string[] { ruleFile.Substring(start, end - start) });

            string startRule = shape.GetStartRule();
            if(!String.IsNullOrEmpty(startRule))
            {
                EditorGUILayout.Popup("Start rule", 0, new string[] { startRule.Substring(startRule.IndexOf('$') + 1) });
                canGenerate = true;
            }
        }

        // UI: Attributes
        bool generate = false;
        if(canGenerate)
        {
            bool dirty = false;
            showAttributes = EditorGUILayout.Foldout(showAttributes, "Attributes");
            if(showAttributes)
            {
                EditorGUI.indentLevel++;
                foreach(Prt4Unity.Attribute attr in shape.GetAttributes())
                {
                    string name = attr.name.Substring(attr.name.IndexOf('$') + 1);

                    switch(attr.type)
                    {
                        case Prt4Unity.Attribute.Type.BoolAttribute:
                            {
                                bool value = EditorGUILayout.Toggle(name, attr.boolValue);
                                if(value != attr.boolValue)
                                {
                                    attr.boolValue = value;
                                    dirty = true;
                                }
                            }
                            break;
                        case Prt4Unity.Attribute.Type.FloatAttribute:
                            {
                                float input = (float)attr.floatValue;
                                double value = EditorGUILayout.FloatField(name, input);
                                if(!double.IsNaN(attr.floatMin) && value < attr.floatMin)
                                    value = attr.floatMin;
                                else if(!double.IsNaN(attr.floatMax) && value > attr.floatMax)
                                    value = attr.floatMax;
                                if(value != input)
                                {
                                    attr.floatValue = value;
                                    dirty = true;
                                }
                            }
                            break;
                        case Prt4Unity.Attribute.Type.StringAttribute:
                            {
                                string value = EditorGUILayout.TextField(name, attr.stringValue);
                                if(value != attr.stringValue)
                                {
                                    attr.stringValue = value;
                                    dirty = true;
                                }
                            }
                            break;
                        case Prt4Unity.Attribute.Type.DirectoryAttribute:
                            {
                                string value = EditorGUILayout.TextField(name, attr.stringValue);
                                if(value != attr.stringValue)
                                {
                                    attr.stringValue = value;
                                    dirty = true;
                                }
                            }
                            break;
                        case Prt4Unity.Attribute.Type.FileAttribute:
                            {
                                EditorGUILayout.BeginHorizontal();
                                string value = EditorGUILayout.TextField(name, attr.stringValue);
                                if(GUILayout.Button("...", GUILayout.MaxWidth(30)))
                                {
                                    string filename = EditorUtility.OpenFilePanel("Select file", value, attr.fileExt);
                                    if(!System.String.IsNullOrEmpty(filename) && filename != value)
                                        value = filename;
                                }
                                EditorGUILayout.EndHorizontal();
                                if(value != attr.stringValue)
                                {
                                    attr.stringValue = value;
                                    dirty = true;
                                }
                            }
                            break;
                        case Prt4Unity.Attribute.Type.ColorAttribute:
                            {
                                Color input = HexToColor(attr.stringValue);
                                Color color = EditorGUILayout.ColorField(name, input);
                                if(!color.Equals(input))
                                {
                                    attr.stringValue = ColorToHex(color);
                                    dirty = true;
                                }
                            }
                            break;
                        case Prt4Unity.Attribute.Type.EnumAttribute:
                            {
                                int value = EditorGUILayout.Popup(name, attr.enumValue, attr.enumFields);
                                if(value != attr.enumValue)
                                {
                                    attr.enumValue = value;
                                    dirty = true;
                                }
                            }
                            break;
                    }
                }
                EditorGUI.indentLevel--;
            }

            if(shape.collisionMaterial == null)
                shape.collisionMaterial = "";
            string collisionMaterial = EditorGUILayout.TextField("Collision material", shape.collisionMaterial);
            if(collisionMaterial != shape.collisionMaterial)
            {
                shape.collisionMaterial = collisionMaterial;
                dirty = true;
            }

            shape.autoGenerate = EditorGUILayout.Toggle("Auto generate", shape.autoGenerate);
            if(dirty && shape.autoGenerate)
                generate = true;
        }

        // UI: Action buttons
        EditorGUILayout.BeginHorizontal();
        if(!canGenerate)
            GUI.enabled = false;
        if(GUILayout.Button("Generate"))
            generate = true;
        if(!canGenerate)
            GUI.enabled = true;
        if(GUILayout.Button("Reset"))
        {
            shape.ClearProceduralObjects();
            shape.GetComponent<MeshRenderer>().enabled = true;
        }
        EditorGUILayout.EndHorizontal();

        // Generate procedural content using PRT and create according game objects
        if(generate)
        {
            shape.ClearProceduralObjects();
            shape.GetComponent<MeshRenderer>().enabled = false;

            EditorUtility.DisplayProgressBar("Prt4Unity", "Generating ...", 0.0f);

            Prt4Unity.OutputMesh[] meshes;
            Transform xformShape = shape.GetComponent<Transform>();
            if(shape.Generate(shape.GetComponent<MeshFilter>().sharedMesh, xformShape, Shader.Find("Diffuse"), out meshes))
            {
                int i = 0;
                foreach(Prt4Unity.OutputMesh outputMesh in meshes)
                {
                    GameObject obj = new GameObject(outputMesh.mesh.name + "_" + (++i));
                    Transform xform = obj.GetComponent<Transform>();
                    xform.parent = xformShape;
                    xform.localPosition = Vector3.zero;
                    xform.localRotation = Quaternion.identity;
                    xform.localScale = new Vector3(1.0f / xformShape.localScale.x, 1.0f / xformShape.localScale.y, 1.0f / xformShape.localScale.z);

                    shape.AddProceduralObject(obj);

                    obj.AddComponent<MeshFilter>();
                    obj.AddComponent<MeshRenderer>();
                    obj.GetComponent<MeshFilter>().mesh = outputMesh.mesh;
                    obj.GetComponent<MeshRenderer>().materials = outputMesh.materials;

                    if(!String.IsNullOrEmpty(shape.collisionMaterial) && outputMesh.materials.Length == 1 && outputMesh.materials[0].name == shape.collisionMaterial)
                    {
                        obj.AddComponent<MeshCollider>();
                        obj.GetComponent<MeshCollider>().sharedMesh = outputMesh.mesh;
                        obj.GetComponent<MeshRenderer>().enabled = false;
                    }
                }
            }

            EditorUtility.ClearProgressBar();
        }
    }

    #region Conversion helpers
    int FromHex(char c)
    {
        switch(c)
        {
            case '0': return 0;
            case '1': return 1;
            case '2': return 2;
            case '3': return 3;
            case '4': return 4;
            case '5': return 5;
            case '6': return 6;
            case '7': return 7;
            case '8': return 8;
            case '9': return 9;
            case 'a':
            case 'A': return 0xa;
            case 'b':
            case 'B': return 0xb;
            case 'c':
            case 'C': return 0xc;
            case 'd':
            case 'D': return 0xd;
            case 'e':
            case 'E': return 0xe;
            case 'f':
            case 'F': return 0xf;
            default: return 0;
        }
    }

    Color HexToColor(string s)
    {
        float r = 0.0f;
        float g = 0.0f;
        float b = 0.0f;
        if(s[0] == '#' && s.Length >= 7)
        {
            r = (float)((FromHex(s[1]) << 4) + FromHex(s[2])) / 255.0f;
            g = (float)((FromHex(s[3]) << 4) + FromHex(s[4])) / 255.0f;
            b = (float)((FromHex(s[5]) << 4) + FromHex(s[6])) / 255.0f;
        }
        return new Color(r, g, b);
    }

    string ColorToHex(Color c)
    {
        int r = Math.Min(Math.Max((int)(c.r * 255.0f), 0), 255);
        int g = Math.Min(Math.Max((int)(c.g * 255.0f), 0), 255);
        int b = Math.Min(Math.Max((int)(c.b * 255.0f), 0), 255);

        return "#" + r.ToString("X2") + g.ToString("X2") + b.ToString("X2");
    }
    #endregion
}
