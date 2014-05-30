mkdir prt4maya\bin\plug-ins\prt_lib
mkdir prt4maya\scripts\startup
copy "%ProgramFiles%\Autodesk\Maya2012\bin\plug-ins\prt4maya.mll" prt4maya\bin\plug-ins
copy "%ProgramFiles%\Autodesk\Maya2012\bin\plug-ins\prt_lib\*.*" prt4maya\bin\plug-ins\prt_lib
copy "%ProgramFiles%\Autodesk\Maya2012\scripts\startup\prt4maya*.mel" prt4maya\scripts\startup
copy "%ProgramFiles%\Autodesk\Maya2012\scripts\startup\AEprtTemplate.mel" prt4maya\scripts\startup
copy "%ProgramFiles%\Autodesk\Maya2012\bin\com.esri.prt.*" prt4maya\bin
copy "%ProgramFiles%\Autodesk\Maya2012\bin\flexnet_prt.dll" prt4maya\bin

