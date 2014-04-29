using UnityEngine;
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

public class Prt4Unity : MonoBehaviour
{
    // Note: Unity's serialization does not support uint; hence int is used throughout this example. Owing to Unity's inability to serialize derived classes
    // (unless using ScriptableObject as base class), attributes are simply represented by a single catch-all class (Prt4Unity.Attribute).

    #region Interface of prt4unity.dll
    [DllImport("prt4unity", CallingConvention = CallingConvention.Cdecl)]
    public static extern bool Initialize();
    [DllImport("prt4unity", CallingConvention = CallingConvention.Cdecl)]
    public static extern IntPtr CreateContext();
    [DllImport("prt4unity", CallingConvention = CallingConvention.Cdecl)]
    public static extern void DeleteContext(IntPtr ctx);
    [DllImport("prt4unity", CallingConvention = CallingConvention.Cdecl)]
    public static extern bool SetRulePackage(IntPtr ctx, [MarshalAs(UnmanagedType.LPWStr)]string filename, [MarshalAs(UnmanagedType.LPWStr)]string unpackPath);
    [DllImport("prt4unity", CallingConvention = CallingConvention.Cdecl)]
    public static extern int GetRuleFileCount(IntPtr ctx);
    [DllImport("prt4unity", CallingConvention = CallingConvention.Cdecl)]
    [return: MarshalAs(UnmanagedType.BStr)]
    public static extern string GetRuleFile(IntPtr ctx, int index);
    [DllImport("prt4unity", CallingConvention = CallingConvention.Cdecl)]
    public static extern bool SetRuleFile(IntPtr ctx, int index);
    [DllImport("prt4unity", CallingConvention = CallingConvention.Cdecl)]
    public static extern int GetStartRuleCount(IntPtr ctx);
    [DllImport("prt4unity", CallingConvention = CallingConvention.Cdecl)]
    [return: MarshalAs(UnmanagedType.BStr)]
    public static extern string GetStartRuleName(IntPtr ctx, int index);
    [DllImport("prt4unity", CallingConvention = CallingConvention.Cdecl)]
    public static extern bool SetStartRule(IntPtr ctx, int index);
    [DllImport("prt4unity", CallingConvention = CallingConvention.Cdecl)]
    public static extern void SetLogMessageCallback(IntPtr callback);

    [DllImport("prt4unity", CallingConvention = CallingConvention.Cdecl)]
    public static extern int GetAttributeCount(IntPtr ctx);
    [DllImport("prt4unity", CallingConvention = CallingConvention.Cdecl)]
    public static extern int GetAttributeType(IntPtr ctx, int attrIndex);
    [DllImport("prt4unity", CallingConvention = CallingConvention.Cdecl)]
    [return: MarshalAs(UnmanagedType.BStr)]
    public static extern string GetAttributeName(IntPtr ctx, int attrIndex);
    [DllImport("prt4unity", CallingConvention = CallingConvention.Cdecl)]
    public static extern bool GetBoolAttributeValue(IntPtr ctx, int attrIndex, out bool value);
    [DllImport("prt4unity", CallingConvention = CallingConvention.Cdecl)]
    public static extern bool GetFloatAttributeValue(IntPtr ctx, int attrIndex, out double value, out double minValue, out double maxValue);
    [DllImport("prt4unity", CallingConvention = CallingConvention.Cdecl)]
    public static extern bool GetStringAttributeValue(IntPtr ctx, int attrIndex, out IntPtr value);
    [DllImport("prt4unity", CallingConvention = CallingConvention.Cdecl)]
    public static extern bool GetDirectoryAttributeValue(IntPtr ctx, int attrIndex, out IntPtr value);
    [DllImport("prt4unity", CallingConvention = CallingConvention.Cdecl)]
    public static extern bool GetFileAttributeValue(IntPtr ctx, int attrIndex, out IntPtr value, out IntPtr ext);
    [DllImport("prt4unity", CallingConvention = CallingConvention.Cdecl)]
    public static extern bool GetColorAttributeValue(IntPtr ctx, int attrIndex, out IntPtr value);
    [DllImport("prt4unity", CallingConvention = CallingConvention.Cdecl)]
    public static extern bool GetEnumAttributeValue(IntPtr ctx, int attrIndex, out int value);
    [DllImport("prt4unity", CallingConvention = CallingConvention.Cdecl)]
    public static extern int GetEnumAttributeFieldCount(IntPtr ctx, int attrIndex);
    [DllImport("prt4unity", CallingConvention = CallingConvention.Cdecl)]
    public static extern bool GetEnumAttributeField(IntPtr ctx, int attrIndex, int fieldIndex, out IntPtr value);

    [DllImport("prt4unity", CallingConvention = CallingConvention.Cdecl)]
    public static extern bool SetBoolAttributeValue(IntPtr ctx, int attrIndex, bool value);
    [DllImport("prt4unity", CallingConvention = CallingConvention.Cdecl)]
    public static extern bool SetFloatAttributeValue(IntPtr ctx, int attrIndex, double value);
    [DllImport("prt4unity", CallingConvention = CallingConvention.Cdecl)]
    public static extern bool SetStringAttributeValue(IntPtr ctx, int attrIndex, [MarshalAs(UnmanagedType.LPWStr)]string value);
    [DllImport("prt4unity", CallingConvention = CallingConvention.Cdecl)]
    public static extern bool SetDirectoryAttributeValue(IntPtr ctx, int attrIndex, [MarshalAs(UnmanagedType.LPWStr)]string value);
    [DllImport("prt4unity", CallingConvention = CallingConvention.Cdecl)]
    public static extern bool SetFileAttributeValue(IntPtr ctx, int attrIndex, [MarshalAs(UnmanagedType.LPWStr)]string value);
    [DllImport("prt4unity", CallingConvention = CallingConvention.Cdecl)]
    public static extern bool SetColorAttributeValue(IntPtr ctx, int attrIndex, [MarshalAs(UnmanagedType.LPWStr)]string value);
    [DllImport("prt4unity", CallingConvention = CallingConvention.Cdecl)]
    public static extern bool SetEnumAttributeValue(IntPtr ctx, int attrIndex, int value);

    [DllImport("prt4unity", CallingConvention = CallingConvention.Cdecl)]
    public static extern bool Generate(IntPtr ctx, [MarshalAs(UnmanagedType.LPArray)] Vector3[] vertices, int numVertices,
        [MarshalAs(UnmanagedType.LPArray)] int[] indices, int numIndices, [MarshalAs(UnmanagedType.LPWStr)]string specialMaterial);

    [DllImport("prt4unity", CallingConvention = CallingConvention.Cdecl)]
    public static extern int GetMaterialCount(IntPtr ctx);
    [DllImport("prt4unity", CallingConvention = CallingConvention.Cdecl)]
    public static extern bool GetMaterial(IntPtr ctx, int materialIndex, out IntPtr name, out IntPtr color, out IntPtr diffuseTexture);
    [DllImport("prt4unity", CallingConvention = CallingConvention.Cdecl)]
    public static extern int GetMeshCount(IntPtr ctx);
    [DllImport("prt4unity", CallingConvention = CallingConvention.Cdecl)]
    public static extern bool GetMesh(IntPtr ctx, int meshIndex, out IntPtr name, out int numVertices, out IntPtr vertices, out IntPtr normals, out IntPtr texcoords);
    [DllImport("prt4unity", CallingConvention = CallingConvention.Cdecl)]
    public static extern int GetSubMeshCount(IntPtr ctx, int meshIndex);
    [DllImport("prt4unity", CallingConvention = CallingConvention.Cdecl)]
    public static extern bool GetSubMesh(IntPtr ctx, int meshIndex, int subMeshIndex, out IntPtr indices, out int numIndices, out int materialIndex);
    [DllImport("prt4unity", CallingConvention = CallingConvention.Cdecl)]
    public static extern void ReleaseMeshesAndMaterials(IntPtr ctx);
    #endregion

    static Prt4Unity()
    {
        Initialize();
    }

    [Serializable]
    public class Attribute
    {
        public enum Type
        {
            BoolAttribute,
            FloatAttribute,
            StringAttribute,
            DirectoryAttribute,
            FileAttribute,
            ColorAttribute,
            EnumAttribute
        }

        // common attribute data
        public int index;
        public string name;
        public Type type;

        // attribute value (only one used at a time, depends on type)
        public bool boolValue;
        public double floatValue;
        public string stringValue;
        public string fileExt;
        public int enumValue;

        // attribute details
        public double floatMin, floatMax;
        public string[] enumFields;
    }

    public class OutputMesh
    {
        public Mesh mesh;
        public Material[] materials;
    }

    #region Context
    // Encapsulates PRT state and provides C# interface to Unity encoder (via prt4unity.dll)
    class Context : IDisposable
    {
        private IntPtr ctx;

        public Context()
        {
            ctx = CreateContext();
        }

        ~Context()
        {
            Dispose(false);
        }

        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        private void Dispose(bool disposing)
        {
            DeleteContext(ctx);
        }

        public bool SetRulePackage(string filename, string unpackPath)
        {
            return Prt4Unity.SetRulePackage(ctx, filename, unpackPath);
        }

        public string[] GetRuleFiles()
        {
            int numFiles = Prt4Unity.GetRuleFileCount(ctx);
            string[] files = new string[numFiles];
            for(int i = 0; i < numFiles; i++)
                files[i] = Prt4Unity.GetRuleFile(ctx, i);
            return files;
        }

        public bool SetRuleFile(int index)
        {
            return Prt4Unity.SetRuleFile(ctx, index);
        }

        public string[] GetStartRules()
        {
            int numRules = Prt4Unity.GetStartRuleCount(ctx);
            string[] rules = new string[numRules];
            for(int i = 0; i < numRules; i++)
                rules[i] = Prt4Unity.GetStartRuleName(ctx, i);
            return rules;
        }

        public bool SetStartRule(int index)
        {
            return Prt4Unity.SetStartRule(ctx, index);
        }

        enum AttributeType
        {
            ATTR_INVALID = 0,
            ATTR_BOOL = 1 << 0,
            ATTR_FLOAT = 1 << 1,
            ATTR_STRING = 1 << 2,
            ATTR_ENUM = 1 << 3,
            ATTR_ENUM_BOOL = ATTR_ENUM | ATTR_BOOL,
            ATTR_ENUM_FLOAT = ATTR_ENUM | ATTR_FLOAT,
            ATTR_ENUM_STRING = ATTR_ENUM | ATTR_STRING,
            ATTR_DIRECTORY = 1 << 4 | ATTR_STRING,
            ATTR_FILE = 1 << 5 | ATTR_STRING,
            ATTR_COLOR = 1 << 6 | ATTR_STRING
        };
        public Attribute[] GetAttributes()
        {
            int numAttributes = Prt4Unity.GetAttributeCount(ctx);
            Attribute[] attributes = new Attribute[numAttributes];
            for(int i = 0; i < numAttributes; i++)
            {
                string name = Prt4Unity.GetAttributeName(ctx, i);
                int type = Prt4Unity.GetAttributeType(ctx, i);
                Attribute attr = new Attribute();
                attr.name = name;
                attr.index = i;
                attributes[i] = attr;
                switch((AttributeType)type)
                {
                    case AttributeType.ATTR_BOOL:
                        {
                            attr.type = Prt4Unity.Attribute.Type.BoolAttribute;
                            Prt4Unity.GetBoolAttributeValue(ctx, i, out attr.boolValue);
                            break;
                        }
                    case AttributeType.ATTR_FLOAT:
                        {
                            attr.type = Prt4Unity.Attribute.Type.FloatAttribute;
                            Prt4Unity.GetFloatAttributeValue(ctx, i, out attr.floatValue, out attr.floatMin, out attr.floatMax);
                            break;
                        }
                    case AttributeType.ATTR_STRING:
                        {
                            attr.type = Prt4Unity.Attribute.Type.StringAttribute;
                            IntPtr ptr;
                            Prt4Unity.GetStringAttributeValue(ctx, i, out ptr);
                            attr.stringValue = Marshal.PtrToStringUni(ptr);
                            break;
                        }
                    case AttributeType.ATTR_DIRECTORY:
                        {
                            attr.type = Prt4Unity.Attribute.Type.DirectoryAttribute;
                            IntPtr ptr;
                            Prt4Unity.GetDirectoryAttributeValue(ctx, i, out ptr);
                            attr.stringValue = Marshal.PtrToStringUni(ptr);
                            break;
                        }
                    case AttributeType.ATTR_FILE:
                        {
                            attr.type = Prt4Unity.Attribute.Type.FileAttribute;
                            IntPtr ptr, ptrExt;
                            Prt4Unity.GetFileAttributeValue(ctx, i, out ptr, out ptrExt);
                            attr.stringValue = Marshal.PtrToStringUni(ptr);
                            attr.fileExt = Marshal.PtrToStringUni(ptrExt);
                            break;
                        }
                    case AttributeType.ATTR_COLOR:
                        {
                            attr.type = Prt4Unity.Attribute.Type.ColorAttribute;
                            IntPtr ptr;
                            Prt4Unity.GetColorAttributeValue(ctx, i, out ptr);
                            attr.stringValue = Marshal.PtrToStringUni(ptr);
                            break;
                        }
                    case AttributeType.ATTR_ENUM_BOOL:
                    case AttributeType.ATTR_ENUM_FLOAT:
                    case AttributeType.ATTR_ENUM_STRING:
                        {
                            attr.type = Prt4Unity.Attribute.Type.EnumAttribute;
                            Prt4Unity.GetEnumAttributeValue(ctx, i, out attr.enumValue);
                            int numFields = Prt4Unity.GetEnumAttributeFieldCount(ctx, i);
                            attr.enumFields = new string[numFields];
                            for(int j = 0; j < numFields; j++)
                            {
                                IntPtr ptr;
                                Prt4Unity.GetEnumAttributeField(ctx, i, j, out ptr);
                                attr.enumFields[j] = Marshal.PtrToStringUni(ptr);
                            }
                            break;
                        }
                }
            }
            return attributes;
        }

        public void SetAttributes(Attribute[] attributes)
        {
            foreach(Attribute attr in attributes)
            {
                switch(attr.type)
                {
                    case Attribute.Type.BoolAttribute:
                        SetBoolAttributeValue(ctx, attr.index, attr.boolValue);
                        break;
                    case Attribute.Type.FloatAttribute:
                        SetFloatAttributeValue(ctx, attr.index, attr.floatValue);
                        break;
                    case Attribute.Type.StringAttribute:
                        SetStringAttributeValue(ctx, attr.index, attr.stringValue);
                        break;
                    case Attribute.Type.DirectoryAttribute:
                        SetDirectoryAttributeValue(ctx, attr.index, attr.stringValue);
                        break;
                    case Attribute.Type.FileAttribute:
                        SetFileAttributeValue(ctx, attr.index, attr.stringValue);
                        break;
                    case Attribute.Type.ColorAttribute:
                        SetColorAttributeValue(ctx, attr.index, attr.stringValue);
                        break;
                    case Attribute.Type.EnumAttribute:
                        SetEnumAttributeValue(ctx, attr.index, attr.enumValue);
                        break;
                }
            }
        }

        public bool Generate(Mesh startShape, Transform loc2WorldTrafo, Shader shader, out OutputMesh[] meshes, string specialMaterial)
        {
            Vector3[] vertices = new Vector3[startShape.vertices.Length];
            for (int i = 0; i < vertices.Length; i++) 
                vertices[i] = loc2WorldTrafo.TransformPoint(startShape.vertices[i]);
            if (!Prt4Unity.Generate(ctx, vertices, vertices.Length, startShape.triangles, startShape.triangles.Length, specialMaterial))
            {
                meshes = null;
                return false;
            }
            Material[] materials = GetMaterials(shader);
            GetMeshes(materials, loc2WorldTrafo, out meshes);
            Prt4Unity.ReleaseMeshesAndMaterials(ctx);
            return true;
        }

        // Retrieves materials from last generate result and creates according Unity materials
        Material[] GetMaterials(Shader shader)
        {
            int numMaterials = Prt4Unity.GetMaterialCount(ctx);
            Material[] materials = new Material[numMaterials];
            for(int i = 0; i < numMaterials; i++)
            {
                IntPtr name, colorPtr, diffuseTexture;
                Prt4Unity.GetMaterial(ctx, i, out name, out colorPtr, out diffuseTexture);
                Material material = new Material(shader);
                material.name = Marshal.PtrToStringUni(name);
                materials[i] = material;

                float[] color = new float[3];
                Marshal.Copy(colorPtr, color, 0, 3);
                material.SetColor("_Color", new Color(color[0], color[1], color[2]));

                string diffuseTextureName = Marshal.PtrToStringUni(diffuseTexture);
                if(!String.IsNullOrEmpty(diffuseTextureName))
                {
                    diffuseTextureName = diffuseTextureName.Substring(diffuseTextureName.IndexOf("/Assets/") + 1);
                    Texture tex = Resources.LoadAssetAtPath<Texture>(diffuseTextureName);
                    if(tex != null)
                        material.mainTexture = tex;
                    else
                        Debug.LogWarning("Cannot assign diffuse texture " + diffuseTextureName);
                }
            }
            return materials;
        }

        // Retrieves meshes from last generate result and creates according Unity meshes
        void GetMeshes(Material[] materials, Transform loc2WorldTrafo, out OutputMesh[] meshes)
        {
            int numMeshes = Prt4Unity.GetMeshCount(ctx);
            meshes = new OutputMesh[numMeshes];
            for(int i = 0; i < numMeshes; i++)
            {
                IntPtr name, pVertices, pNormals, pTexcoords;
                int numVertices;
                Prt4Unity.GetMesh(ctx, i, out name, out numVertices, out pVertices, out pNormals, out pTexcoords);
                OutputMesh outputMesh = new OutputMesh();
                meshes[i] = outputMesh;
                outputMesh.mesh = new Mesh();
                outputMesh.mesh.name = Marshal.PtrToStringUni(name);
                float[] floats = new float[numVertices * 3];
                Marshal.Copy(pVertices, floats, 0, numVertices * 3);
                Vector3[] vertices = new Vector3[numVertices];
                for (int v = 0; v < numVertices; v++) {
                    vertices[v] = new Vector3(floats[3 * v + 0], floats[3 * v + 1], floats[3 * v + 2]);
                    vertices[v] = loc2WorldTrafo.InverseTransformPoint(vertices[v]);
                }
                outputMesh.mesh.vertices = vertices;
                Marshal.Copy(pNormals, floats, 0, numVertices * 3);
                Vector3[] normals = new Vector3[numVertices];
                for(int v = 0; v < numVertices; v++)
                    normals[v] = new Vector3(floats[3 * v + 0], floats[3 * v + 1], -floats[3 * v + 2]);
                outputMesh.mesh.normals = normals;
                if(pTexcoords != IntPtr.Zero)
                {
                    Marshal.Copy(pTexcoords, floats, 0, numVertices * 2);
                    Vector2[] texcoords = new Vector2[numVertices];
                    for(int v = 0; v < numVertices; v++)
                        texcoords[v] = new Vector2(floats[2 * v + 0], floats[2 * v + 1]);
                    outputMesh.mesh.uv = texcoords;
                }

                int numSubMeshes = Prt4Unity.GetSubMeshCount(ctx, i);
                outputMesh.mesh.subMeshCount = (int)numSubMeshes;
                outputMesh.materials = new Material[numSubMeshes];
                for(int s = 0; s < numSubMeshes; s++)
                {
                    IntPtr pIndices;
                    int numIndices, materialIndex;
                    Prt4Unity.GetSubMesh(ctx, i, s, out pIndices, out numIndices, out materialIndex);
                    int[] indices = new int[numIndices];
                    Marshal.Copy(pIndices, indices, 0, numIndices);
                    outputMesh.mesh.SetTriangles(indices, (int)s);
                    outputMesh.materials[s] = materials[materialIndex];
                }
            }
        }
    }
    #endregion

    private Context ctx;
    private bool loadedRulePackage = false;

    #region Persistent state
    [SerializeField]
    private List<GameObject> proceduralObjects = new List<GameObject>();   // game objects created for PRT output
    [SerializeField]
    private string rulePackage;                                            // filename of rule package
    [SerializeField]
    private string unpackPath;                                             // path to which rule package is unpacked
    [SerializeField]
    private Attribute[] attributes;                                        // current attributes
    public string collisionMaterial;
    public bool autoGenerate = false;
    #endregion

    Prt4Unity()
    {
        ctx = new Context();
        SetCallbacks();
    }

    public bool IsInitialized()
    {
        return String.IsNullOrEmpty(rulePackage) || loadedRulePackage;
    }

    public bool EnsureInit()
    {
        if(IsInitialized())
            return true;

        this.loadedRulePackage = true;
        return ctx.SetRulePackage(rulePackage, unpackPath) && ctx.SetRuleFile(0) && ctx.SetStartRule(0);
    }

    public bool LoadRulePackage(string filename, string unpackPath)
    {
        this.rulePackage = filename;
        this.unpackPath = unpackPath;
        this.attributes = null;
        this.loadedRulePackage = true;
        return ctx.SetRulePackage(filename, unpackPath) && ctx.SetRuleFile(0) && ctx.SetStartRule(0);
    }

    public string GetRulePackage()
    {
        return rulePackage;
    }

    public string GetRuleFile()
    {
        string[] files = ctx.GetRuleFiles();
        return files.Length == 0 ? null : files[0];
    }

    public string GetStartRule()
    {
        string[] rules = ctx.GetStartRules();
        return rules.Length == 0 ? null : rules[0];
    }

    public Attribute[] GetAttributes()
    {
        if(attributes == null)
            attributes = ctx.GetAttributes();
        return attributes;
    }

    public bool Generate(Mesh startShape, Transform loc2WorldTrafo, Shader shader, out OutputMesh[] meshes)
    {
        ctx.SetAttributes(attributes);
        return ctx.Generate(startShape, loc2WorldTrafo, shader, out meshes, collisionMaterial);
    }

    public void AddProceduralObject(GameObject obj)
    {
        proceduralObjects.Add(obj);
    }

    public void ClearProceduralObjects()
    {
        foreach(GameObject obj in proceduralObjects)
            GameObject.DestroyImmediate(obj);
        proceduralObjects.Clear();
    }

    #region Logging
    // Callback for PRT log handler
    [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
    public delegate void PostLogMessage([In][MarshalAs(UnmanagedType.LPWStr)]string message, int level);

    static PostLogMessage postLogMessageDelegate;
    static object classLock = new object();

    private void SetCallbacks()
    {
        lock(classLock)
            if(Prt4Unity.postLogMessageDelegate == null)
            {
                Prt4Unity.postLogMessageDelegate = new PostLogMessage(LogMessage);
                IntPtr p = Marshal.GetFunctionPointerForDelegate(Prt4Unity.postLogMessageDelegate);
                SetLogMessageCallback(p);
            }
    }

    enum LogLevel
    {
        LOG_TRACE = 0, LOG_DEBUG = 1, LOG_INFO = 2, LOG_WARNING = 3, LOG_ERROR = 4, LOG_FATAL = 5, LOG_NO = 1000
    };
    static void LogMessage([In][MarshalAs(UnmanagedType.LPWStr)]string message, int level)
    {
        switch((LogLevel)level)
        {
            case LogLevel.LOG_ERROR:
            case LogLevel.LOG_FATAL:
                Debug.LogError(message);
                break;
            case LogLevel.LOG_WARNING:
                Debug.LogWarning(message);
                break;
            default:
                Debug.Log(message);
                break;
        }
    }
    #endregion
}
