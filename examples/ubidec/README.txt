Inofficial Esri CityEngine SDK Custom Decoder/Adaptor Example
-------------------------------------------------------------

Note: The custom UBIDecoder/UBIAdaptor example code does nothing "visible" in the current state,
      see the various TODOs in the source files.


Basic Concept
-------------

This is a workaround for the CityEngine 2014.0 limitation of only being able to resolve assets
from a 'file:' URI.

The custom adaptor will read a short text file and use its content to access the
'real' data source, i.e. produce the 'real' data byte stream which is then fed into a custom
decoder to generate the corresponding prtx::Geometry object.

These 'redirect' files could be produced by a preprocess step run with python or outside CityEngine.


Compilation/Usage Instructions for RHEL 6 (or compatible)
---------------------------------------------------------

1. cd into the "ubidec" directory
2. create a "build" directory and cd into it
3. run cmake:
   cmake -Dprt_DIR=<your path to>/esri_ce_sdk_1_1_1471_rhel6_gcc44_rel_opt/cmake -DCMAKE_BUILD_TYPE=Debug ../src
4. compile it:
   make install
5. copy the resulting library into your CityEngine 2014.0 installation:
   cp ../install/lib/libubi.so <your path to CityEngine>/plugins/com.esri.prt.clients.ce.gtk.linux.x86_64_1.0.0/lib/
6. start CityEngine from a shell,
   in Help->About->Installation Details->System Info->PRT
   you should find the UBIDecoder (Adaptors are not listed atm)
7. create a new project/scene and create an text file with the extension ".ubi" in the assets folder,
   e.g. 'assets/redirect.ubi'
8. create a test rule file with the following content and assign it to a shape:
   version "2014.0"
   Init --> i("redirect.ubi")
9. if you generate, you should see some debug output on the shell where you started CityEngine:
   UBIAdaptor create stream: "<your ws path>/<your project>/assets/redirect.ubi"
   UBIDecoder::decode
   UBIAdaptor::destroyStream

