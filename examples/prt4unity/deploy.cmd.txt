@ECHO ON

pushd examples\prt4unity\install\Release

mkdir MyUnityProject\Assets\Plugins
mkdir MyUnityProject\Assets\Prt4Unity\Editor
mkdir MyUnityProject\Assets\Prt4Unity\Scripts

move prt4unity.dll MyUnityProject\Assets\Plugins\
move Prt4UnityEditor.cs MyUnityProject\Assets\Prt4Unity\Editor\
move Prt4Unity.cs MyUnityProject\Assets\Prt4Unity\Scripts\

mkdir ProgramFiles(x86)\Unity\Editor
mkdir ProgramFiles(x86)\Unity\prt

move com.esri.prt.core.dll ProgramFiles(x86)\Unity\Editor\
move com.esri.prt.loader.dll ProgramFiles(x86)\Unity\Editor\
move flexnet_prt.dll ProgramFiles(x86)\Unity\Editor\

move com.esri.prt.examples.unity.dll ProgramFiles(x86)\Unity\prt
move com.esri.prt.codecs.dll ProgramFiles(x86)\Unity\prt
move com.esri.prt.adaptors.dll ProgramFiles(x86)\Unity\prt
move VueExport.dll ProgramFiles(x86)\Unity\prt

popd
