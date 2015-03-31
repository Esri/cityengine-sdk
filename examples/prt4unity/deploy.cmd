@ECHO ON
setlocal

set BT=%~1
set PRT=%~2
set PRT_VERSION_MAJOR=%~3
set PRT_VERSION_MINOR=%~4
set PRT_VERSION_MICRO=%~5
set PRT4UNITY_BUILD_ID=%~6

pushd examples\prt4unity\install\%BT%

mkdir MyUnityProject\Assets\Plugins
mkdir MyUnityProject\Assets\Prt4Unity\Editor
mkdir MyUnityProject\Assets\Prt4Unity\Scripts

move prt4unity.dll MyUnityProject\Assets\Plugins\
copy ..\..\src\scripts\Prt4UnityEditor.cs MyUnityProject\Assets\Prt4Unity\Editor\
move ..\..\src\scripts\Prt4Unity.cs MyUnityProject\Assets\Prt4Unity\Scripts\

mkdir ProgramFiles(x86)\Unity\Editor\prt

copy %PRT%\bin\com.esri.prt.core.dll ProgramFiles(x86)\Unity\Editor\
copy %PRT%\bin\com.esri.prt.loader.dll ProgramFiles(x86)\Unity\Editor\
copy %PRT%\bin\flexnet_prt.dll ProgramFiles(x86)\Unity\Editor\

move com.esri.prt.examples.unity.dll ProgramFiles(x86)\Unity\Editor\prt
copy %PRT%\lib\com.esri.prt.codecs.dll ProgramFiles(x86)\Unity\Editor\prt
copy %PRT%\lib\com.esri.prt.adaptors.dll ProgramFiles(x86)\Unity\Editor\prt
copy %PRT%\lib\VueExport.dll ProgramFiles(x86)\Unity\Editor\prt

copy ..\..\README.windows .
xcopy /S ..\..\..\..\license license\

REM 7z a esri_prt4unity_%PRT4UNITY_BUILD_ID%_cesdk_%PRT_VERSION_MAJOR%_%PRT_VERSION_MINOR%_%PRT_VERSION_MICRO%_%BT%_win_x86.zip MyUnityProject\* ProgramFiles(x86)\* license\* README.windows
7z a esri_prt4unity_win32_x86.zip MyUnityProject\* ProgramFiles(x86)\* license\* README.windows
popd

endlocal
