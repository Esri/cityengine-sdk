@ECHO ON

set BT=%~1
set PRT=%~2
set PRT_BUILD=%~3

pushd examples\prt4unity\install\%BT%

mkdir MyUnityProject\Assets\Plugins
mkdir MyUnityProject\Assets\Prt4Unity\Editor
mkdir MyUnityProject\Assets\Prt4Unity\Scripts

move prt4unity.dll MyUnityProject\Assets\Plugins\
move Prt4UnityEditor.cs MyUnityProject\Assets\Prt4Unity\Editor\
move Prt4Unity.cs MyUnityProject\Assets\Prt4Unity\Scripts\

mkdir ProgramFiles(x86)\Unity\Editor
mkdir ProgramFiles(x86)\Unity\prt

copy %PRT%\bin\com.esri.prt.core.dll ProgramFiles(x86)\Unity\Editor\
copy %PRT%\bin\com.esri.prt.loader.dll ProgramFiles(x86)\Unity\Editor\
copy %PRT%\bin\flexnet_prt.dll ProgramFiles(x86)\Unity\Editor\

move com.esri.prt.examples.unity.dll ProgramFiles(x86)\Unity\prt
copy %PRT%\lib\com.esri.prt.codecs.dll ProgramFiles(x86)\Unity\prt
copy %PRT%\lib\com.esri.prt.adaptors.dll ProgramFiles(x86)\Unity\prt
copy %PRT%\lib\VueExport.dll ProgramFiles(x86)\Unity\prt

7z a prt4unity_win32_%PRT_BUILD%_%BT%.zip MyUnityProject\* ProgramFiles(x86)\*
popd