cmake_minimum_required(VERSION 3.2)
cmake_policy(SET CMP0015 NEW)


### project definition

set(PROJECT prt4maya_client)
project(${PROJECT})

if(MAYA_PLUGIN_INSTALL_PATH)
	set(CMAKE_INSTALL_PREFIX ${MAYA_PLUGIN_INSTALL_PATH})
else()
	set(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/../install")
endif()


### dependencies

include_directories(${maya_include_path})
link_directories(${maya_library_path})

#set(prt4maya_codec_DIR "../codec/install/cmake")
#find_package(prt4maya_codec CONFIG REQUIRED)
#include_directories(${PRT4MAYA_CODEC_INCLUDE_PATH})

set(MAYA_DESC "maya${MAYA_VERSION}")


### setup compiler flags

if(WIN32)
	set(CMAKE_C_FLAGS "")
	set(CMAKE_CXX_FLAGS "")

	#set(CMAKE_CXX_FLAGS_DEBUG "/Zi /nologo /W3 /WX- /Od -DWIN32 -D_DEBUG -D_WINDOWS -D_AFXDLL -D_MBCS -DNT_PLUGIN -DREQUIRE_IOSTREAM -DBits64_ -D_WINDLL /Gm- /EHsc /MDd /GS /fp:precise /Zc:wchar_t /Zc:forScope /Gd /errorReport:queue")
	set(CMAKE_CXX_FLAGS_DEBUG "/Gm /GR /GS /EHsc /Zi /DWIN32 /D_DEBUG /RTC1 /c /D_WINDOWS /D_AFXDLL /D_MBCS /DNT_PLUGIN /DREQUIRE_IOSTREAM /DBits64_")

	#set(CMAKE_CXX_FLAGS_RELEASE "/Zi /nologo /W3 /WX- /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D "NT_PLUGIN" /D "REQUIRE_IOSTREAM" /D "Bits64_" /D "_WINDLL" /Gm- /EHsc /MD /GS /fp:precise /Zc:wchar_t /Zc:forScope /Fp"Release/prt4maya.pch" /Fa"Release\" /Fo"Release\" /Fd"Release\vc100.pdb" /Gd /errorReport:queue")
	set(CMAKE_CXX_FLAGS_RELEASE "/GR /GS /EHsc /c /DWIN32 /DNDEBUG /D_WINDOWS /D_AFXDLL /D_MBCS /DNT_PLUGIN /DREQUIRE_IOSTREAM /DBits64_")

	set(CMAKE_SHARED_LINKER_FLAGS_DEBUG="/NODEFAULTLIB:LIBCMT /export:initializePlugin /export:uninitializePlugin")
	set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "/NODEFAULTLIB:CPMTD /export:initializePlugin /export:uninitializePlugin")
	#SET( CMAKE_SHARED_LINKER_FLAGS_DEBUG "/ALLOWISOLATION /MANIFESTUAC:\"level='asInvoker' uiAccess='false'\" /TLBID:1 /DYNAMICBASE:NO /MACHINE:X64 /ERRORREPORT:QUEUE")
	#SET( CMAKE_SHARED_LINKER_FLAGS_DEBUG "/subsystem:windows /dll /debug  /MANIFESTUAC:\"level='asInvoker' uiAccess='false'\" /TLBID:1 /DYNAMICBASE:NO /MACHINE:X64 /ERRORREPORT:QUEUE")
else()
	# build settings required by maya
	set(CMAKE_C_FLAGS "-Wall -Wextra -DBits64_ -m64 -DUNIX -D_BOOL -DLINUX -DFUNCPROTO -D_GNU_SOURCE -DLINUX_64 -fPIC -fno-strict-aliasing -DREQUIRE_IOSTREAM -Wno-deprecated -Wall -Wno-multichar -Wno-comment -Wno-sign-compare -funsigned-char -Wno-reorder -fno-gnu-keywords -ftemplate-depth=50 -pthread")
	set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=c++11 -fvisibility=hidden -fvisibility-inlines-hidden -Wl,--exclude-libs,ALL -Wall -Wno-deprecated -fno-gnu-keywords")

	set(CMAKE_CXX_FLAGS_DEBUG "-ggdb")
	set(CMAKE_CXX_FLAGS_RELEASE "-O2 -s")

endif()


### setup build target

include_directories(${CMAKE_SOURCE_DIR})
set(CMAKE_INCLUDE_CURRENT_DIR ON) # in case we generate source files
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)
set(CMAKE_SKIP_RPATH FALSE)
set(CMAKE_INSTALL_RPATH "\$ORIGIN/")

add_library(${PROJECT} SHARED
	node/PRTAttrs.cpp
	node/PRTCreate.cpp
	node/PRTMaterials.cpp
	node/PRTNode.cpp
	node/Utilities.cpp
	node/MayaCallbacks.cpp
)
#target_compile_options(${PROJECT} PUBLIC "-std=c++11 -Wl,--exclude-libs,ALL") # $<$<CONFIG:RELEASE>:${P4M_COMPILE_FLAGS}>
target_link_libraries(${PROJECT} Foundation OpenMaya OpenMayaUI ${PRT_LINK_LIBRARIES})
set_target_properties(${PROJECT} PROPERTIES LINKER_LANGUAGE CXX)

if(WIN32)
	add_custom_command(TARGET ${PROJECT} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E rename "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT}.dll" "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT}.mll"
	)
endif()


### setup install target

install(CODE "file(REMOVE_RECURSE ${CMAKE_INSTALL_PREFIX})")
install(FILES ${PRT_LIBRARIES} DESTINATION plug-ins)
install(FILES ${PRT_EXT_LIBRARIES} DESTINATION plug-ins/ext)
install(FILES ${PRT4MAYA_CODEC_EXT_LIBRARIES} DESTINATION plug-ins/ext)
install(FILES scripts/AEprtTemplate.mel DESTINATION scripts)
install(FILES scripts/prt4mayaCreateUI.mel DESTINATION scripts)
install(FILES scripts/prt4mayaDeleteUI.mel DESTINATION scripts)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/../../../../license DESTINATION .)
if(WIN32)
	install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT}.mll" DESTINATION plug-ins)
	install(FILES ${PROJECT_SOURCE_DIR}/../../README.windows DESTINATION .)
else()
	install(TARGETS ${PROJECT} DESTINATION plug-ins)
	install(FILES ${PROJECT_SOURCE_DIR}/../../README.linux_osx DESTINATION .)
endif()


### package target

string(TOLOWER ${CMAKE_SYSTEM_NAME} PACKAGE_SYSTEM)
if(CMAKE_CXX_SIZEOF_DATA_PTR EQUAL 8)
	set(PACKAGE_ARCH "x86_64")
else()
	set(PACKAGE_ARCH "x86")
endif()

set(CPACK_PACKAGE_NAME					"esri_prt4maya")
set(CPACK_PACKAGE_VENDOR				"Esri R&D Center Zurich")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY	"Maya Plugin Example for CityEngine SDK")
set(CPACK_PACKAGE_VERSION_MAJOR			${PRT4MAYA_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR			${PRT4MAYA_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH			${PRT4MAYA_VERSION_MICRO})
set(CPACK_PACKAGE_INSTALL_DIRECTORY		"${CPACK_PACKAGE_NAME}")
set(CPACK_PACKAGE_FILE_NAME				"${CPACK_PACKAGE_INSTALL_DIRECTORY}")

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	set(CPACK_GENERATOR					ZIP)
	set(CPACK_SOURCE_GENERATOR			ZIP)
	set(CPACK_ZIP_COMPONENT_INSTALL		1)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
	set(CPACK_GENERATOR					ZIP)
	set(CPACK_SOURCE_GENERATOR			ZIP)
	set(CPACK_TGZ_COMPONENT_INSTALL		1)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	set(CPACK_GENERATOR					ZIP)
	set(CPACK_SOURCE_GENERATOR			ZIP)
	set(CPACK_TGZ_COMPONENT_INSTALL		1)
endif()

include(CPack)

