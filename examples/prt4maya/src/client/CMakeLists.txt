cmake_minimum_required(VERSION 3.13)
cmake_policy(SET CMP0015 NEW)


### project definition

project(prt4maya CXX)
include(${CMAKE_CURRENT_LIST_DIR}/../common.cmake) # TODO: could be moved to top-level script


### setup build target

set(CMAKE_INCLUDE_CURRENT_DIR ON) # in case we generate source files
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)
set(CMAKE_SKIP_RPATH FALSE)
set(CMAKE_INSTALL_RPATH "\$ORIGIN/")

add_library(${PROJECT_NAME} SHARED
		node/PRTAttrs.cpp
		node/PRTCreate.cpp
		node/PRTMaterials.cpp
		node/PRTNode.cpp
		node/Utilities.cpp
		node/MayaCallbacks.cpp)

if(WIN32)
    # On Windows, Maya requires plugins to have the extension "mll"
    set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".mll")
endif()


### setup compiler flags

if(WIN32)
	target_compile_options(${PROJECT_NAME} PRIVATE
			-GR -GS -EHsc -Zi -c
			$<$<CONFIG:DEBUG>:-Gm -RTC1>)

	target_compile_definitions(${PROJECT_NAME} PRIVATE
			-DWIN32 -D_WINDOWS -D_AFXDLL -D_MBCS -DNT_PLUGIN -DREQUIRE_IOSTREAM -DBits64_
			$<$<CONFIG:DEBUG>:-D_DEBUG>
			$<$<CONFIG:RELEASE>:-DNDEBUG>)

	target_link_libraries(${PROJECT_NAME} PRIVATE
			-export:initializePlugin
			-export:uninitializePlugin
			$<$<CONFIG:DEBUG>:-NODEFAULTLIB:LIBCMT>
			$<$<CONFIG:RELEASE>:-NODEFAULTLIB:CPMTD>)

else()
	target_compile_options(${PROJECT_NAME} PRIVATE
			-std=c++11 -Wl,--exclude-libs,ALL
			-fvisibility=hidden -fvisibility-inlines-hidden
			-m64 -fPIC -fno-strict-aliasing -funsigned-char -Wno-reorder -fno-gnu-keywords -ftemplate-depth=50 -pthread
			-Wall -Wextra -Wno-deprecated -Wno-multichar -Wno-comment -Wno-sign-compare
			$<$<CONFIG:DEBUG>:-ggdb>
			$<$<CONFIG:RELEASE>:-O2 -s>)

	target_compile_definitions(${PROJECT_NAME} PRIVATE
			-DBits64_ -DUNIX -D_BOOL -DLINUX -DFUNCPROTO -D_GNU_SOURCE -DLINUX_64 -DREQUIRE_IOSTREAM)
endif()


target_include_directories(${PROJECT_NAME} PRIVATE
		${CMAKE_SOURCE_DIR} # TODO: needed?
		${PRT_INCLUDE_PATH}
		${maya_INCLUDE_PATH})

target_link_libraries(${PROJECT_NAME} PRIVATE
		${maya_LINK_LIBRARIES} ${PRT_LINK_LIBRARIES})


### setup install target

install(TARGETS ${PROJECT_NAME} DESTINATION plug-ins)
install(FILES ${PRT_LIBRARIES} DESTINATION plug-ins)
install(FILES ${PRT_EXT_LIBRARIES} DESTINATION plug-ins/ext)
install(FILES scripts/AEprtTemplate.mel DESTINATION scripts)
install(FILES scripts/prt4mayaCreateUI.mel DESTINATION scripts)
install(FILES scripts/prt4mayaDeleteUI.mel DESTINATION scripts)
install(FILES ${PROJECT_SOURCE_DIR}/../../../../LICENSE DESTINATION .)
install(FILES ${PROJECT_SOURCE_DIR}/../../../../EULA.pdf DESTINATION .)
if(WIN32)
	install(FILES ${PROJECT_SOURCE_DIR}/../../README_windows.md DESTINATION .)
else()
	install(FILES ${PROJECT_SOURCE_DIR}/../../README_linux_osx.md DESTINATION .)
endif()


### package target

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
	set(CPACK_GENERATOR					ZIP) # TODO: use tgz
	set(CPACK_SOURCE_GENERATOR			ZIP)
	set(CPACK_TGZ_COMPONENT_INSTALL		1)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	set(CPACK_GENERATOR					ZIP) # TODO: use tgz
	set(CPACK_SOURCE_GENERATOR			ZIP)
	set(CPACK_TGZ_COMPONENT_INSTALL		1)
endif()

include(CPack)
