# prt_DIR must point to the cmake subfolder of the desired SDK installation
#set(prt_DIR		"${PROJECT_SOURCE_DIR}/../../../../prt/cmake")

set(prt_DIR		"C:/fst/dec/ce_server_ws/com.esri.prt.build/cmake")



# boost location
if (WIN32)
	set(ENV_BOOST_INC "$ENV{Boost_INCLUDE_DIR}")
	set(ENV_BOOST_LIB "$ENV{Boost_LIBRARY_DIR}")
	if(ENV_BOOST_INC)
		set(Boost_INCLUDE_DIR ${ENV_BOOST_INC})
	else()
		set(Boost_INCLUDE_DIR "C:/fst/dec/ce_server_ws/org.boost_1_53_0/include")
	endif()
	if(ENV_BOOST_LIB)
		set(Boost_LIBRARY_DIR ${ENV_BOOST_LIB})	
	else()
		set(Boost_LIBRARY_DIR "C:/fst/dec/ce_server_ws/org.boost_1_53_0/lib/win32.x86_64")
	endif()
endif()

# autodesk maya installation location, with maya 2012 as default
set(ENV_MAYA_DIR "$ENV{maya_DIR}")
if(ENV_MAYA_DIR)
	set(maya_DIR "${ENV_MAYA_DIR}")
else()
	if(WIN32)
		set(maya_DIR	"C:/Program Files/Autodesk/Maya2012")
	else()
		set(maya_DIR	"/usr/autodesk/maya2012-x64")
	endif()
endif()
set(maya_include_path	"${maya_DIR}/include")
set(maya_library_path	"${maya_DIR}/lib")

# plugin installation location
set(MAYA_PLUGIN_INSTALL_PATH "${PROJECT_SOURCE_DIR}/../install")

# helper macros
macro (add_sources)
	foreach (_src ${ARGN})
		list (APPEND SOURCES "${_src}")
	endforeach()
	set(SOURCES ${SOURCES} PARENT_SCOPE)
endmacro()
