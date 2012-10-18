cmake_minimum_required(VERSION 2.8)
cmake_policy(SET CMP0015 NEW)

# --- project -----------------------------------------------------------------

set(PROJECT prt4maya)
project(${PROJECT})


# --- dependencies ------------------------------------------------------------

set(CMAKE_MODULE_PATH
	"${CMAKE_CURRENT_SOURCE_DIR}/../../com.esri.prt.core/cmake"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../com.esri.prt.codecs/cmake"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../com.esri.prt.codecs.maya/cmake"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../com.esri.prt.adaptors.fs/cmake"
	${CMAKE_MODULE_PATH}
)

find_package(com.esri.prt.core)
include_directories(${com.esri.prt.core_INCLUDE_PATH})

find_package(com.esri.prt.codecs)
include_directories(${com.esri.prt.codecs_INCLUDE_PATH})

find_package(com.esri.prt.codecs.maya)
include_directories(${com.esri.prt.codecs.maya_INCLUDE_PATH})

find_package(com.esri.prt.adaptors.fs)
include_directories(${com.esri.prt.adaptors.fs_INCLUDE_PATH})

# hacky maya api integration, todo: write cmake module
if(WIN32)
    include_directories("C:/Program Files/Autodesk/Maya2012/include")
    link_directories("C:/Program Files/Autodesk/Maya2012/lib")
else()
	include_directories("/usr/autodesk/maya2012-x64/include")
	link_directories("/usr/autodesk/maya2012-x64/lib")
endif()


# --- compiler flags ----------------------------------------------------------

if(WIN32)
	set(CMAKE_C_FLAGS "")
	set(CMAKE_CXX_FLAGS "")
	set(CMAKE_CXX_FLAGS_DEBUG "/Zi /nologo /W3 /WX- /Od -DWIN32 -D_DEBUG -D_WINDOWS -D_AFXDLL -D_MBCS -DNT_PLUGIN -DREQUIRE_IOSTREAM -DBits64_ -D_WINDLL /Gm- /EHsc /MDd /GS /fp:precise /Zc:wchar_t /Zc:forScope /Gd /errorReport:queue")
#	set(CMAKE_CXX_FLAGS_RELEASE "/Zi /nologo /W3 /WX- /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D "NT_PLUGIN" /D "REQUIRE_IOSTREAM" /D "Bits64_" /D "_WINDLL" /Gm- /EHsc /MD /GS /fp:precise /Zc:wchar_t /Zc:forScope /Fp"Release/prt4maya.pch" /Fa"Release\" /Fo"Release\" /Fd"Release\vc100.pdb" /Gd /errorReport:queue")

#	SET( CMAKE_SHARED_LINKER_FLAGS_DEBUG "/ALLOWISOLATION /MANIFESTUAC:\"level='asInvoker' uiAccess='false'\" /TLBID:1 /DYNAMICBASE:NO /MACHINE:X64 /ERRORREPORT:QUEUE")
	SET( CMAKE_SHARED_LINKER_FLAGS_DEBUG "/MANIFESTUAC:\"level='asInvoker' uiAccess='false'\" /TLBID:1 /DYNAMICBASE:NO /MACHINE:X64 /ERRORREPORT:QUEUE")
else()
    # build settings required by maya
    # NOTE: make sure to compile the maya plugin with gcc 4.1.2 (for maya 2012/2013)
	set(CMAKE_C_FLAGS "-Wall -Wextra -DBits64_ -m64 -DUNIX -D_BOOL -DLINUX -DFUNCPROTO -D_GNU_SOURCE -DLINUX_64 -fPIC -fno-strict-aliasing -DREQUIRE_IOSTREAM -Wno-deprecated -O3 -Wall -Wno-multichar -Wno-comment -Wno-sign-compare -funsigned-char -Wno-reorder -fno-gnu-keywords -ftemplate-depth-25 -pthread")
    set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -Wno-deprecated -fno-gnu-keywords")
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Bsymbolic -Bsymbolic-functions")
	set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Bsymbolic -Bsymbolic-functions")
endif()


# --- helper macros -----------------------------------------------------------

macro (add_sources)
	foreach (_src ${ARGN})
		list (APPEND SOURCES "${_src}")
	endforeach()
	set(SOURCES ${SOURCES} PARENT_SCOPE)
endmacro()

macro (add_subdirectory_source_group thedir)
	add_subdirectory(${thedir})
	add_sf_sources_to_source_group(${thedir})				# msvc sln
endmacro()

macro (add_sf_sources)
	file(GLOB HDR "*.h")
	add_sources(${HDR})

	file(GLOB FS_FILESH RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.h")
	file(GLOB FS_FILESC RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cpp")

	set(FS_FILES ${FS_FILESH} ${FS_FILESC})

	foreach (_src ${FS_FILES})
		list (APPEND SF_SOURCEStmp "${_src}")
	endforeach()
	set(SF_SOURCES ${SF_SOURCEStmp} PARENT_SCOPE)
endmacro()

function (add_sf_sources_to_source_group prefix)
	foreach (_src ${SF_SOURCES})
		list (APPEND SF_EXPANDED "${prefix}/${_src}")
	endforeach()
	STRING(REGEX REPLACE "/" "\\\\" SourceGroup ${prefix} )
	SOURCE_GROUP(src\\${SourceGroup} FILES ${SF_EXPANDED})
endfunction()


# --- build target ------------------------------------------------------------

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)
set(CMAKE_SKIP_RPATH FALSE)
SET(CMAKE_INSTALL_RPATH "\$ORIGIN/")

SOURCE_GROUP(src REGULAR_EXPRESSION ".*.[h|cpp]")	# msvc sln
add_subdirectory_source_group(prt4maya)
add_subdirectory_source_group(wrapper)

add_library(${PROJECT} SHARED ${SOURCES})
target_link_libraries(${PROJECT} Foundation OpenMaya ${PRT_CORE_LIBRARY} )

set_target_properties(${PROJECT} PROPERTIES LINKER_LANGUAGE CXX)


# --- install target ----------------------------------------------------------

install(CODE "MESSAGE(\"wiping installation dir\")")
install(CODE "file(REMOVE_RECURSE ${CMAKE_INSTALL_PREFIX})" )

install(TARGETS ${PROJECT} DESTINATION .)

install(FILES ${PRT_CORE_LIBRARIES} DESTINATION .)
install(FILES ${PRT_CODECS_LIBRARIES} DESTINATION prt_lib)
install(FILES ${PRT_FS_LIBRARIES} DESTINATION prt_lib)
install(FILES ${PRT_CODECS_MAYA_LIBRARIES} DESTINATION prt_lib)
install(FILES conf/prt_config.xml DESTINATION prt_lib)

install(FILES scripts/AEprtTemplate.mel DESTINATION .)
install(FILES scripts/prt4mayaCreateUI.mel DESTINATION .)
install(FILES scripts/prt4mayaDeleteUI.mel DESTINATION .)
