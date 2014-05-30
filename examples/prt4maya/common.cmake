# prt_DIR must point to the cmake subfolder of the desired SDK installation
set(prt_DIR		"${PROJECT_SOURCE_DIR}/../../../../prt/cmake")

# boost location
if (WIN32)
	set(ENV_BOOST_INC "$ENV{Boost_INCLUDE_DIR}")
	set(ENV_BOOST_LIB "$ENV{Boost_LIBRARY_DIR}")
	if(ENV_BOOST_INC)
		set(Boost_INCLUDE_DIR ${ENV_BOOST_INC})
	else()
		message(FATAL_ERROR "Boost_INCLUDE_DIR environment variable has not been found")
	endif()
	if(ENV_BOOST_LIB)
		set(Boost_LIBRARY_DIR ${ENV_BOOST_LIB})
	else()
		message(FATAL_ERROR "Boost_LIBRARY_DIR environment variable has not been found")
	endif()
endif()

# autodesk maya installation location, with maya 2012 as default
set(ENV_MAYA_DIR "$ENV{maya_DIR}")
if(ENV_MAYA_DIR)
	set(maya_DIR "${ENV_MAYA_DIR}")
endif()
if(NOT maya_DIR)
	message(FATAL_ERROR "maya_DIR environment variable has not been found")
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
