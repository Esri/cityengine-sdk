# prt_DIR must point to the cmake subfolder of the desired SDK installation
set(prt_DIR		"${PROJECT_SOURCE_DIR}/../../../../prt/cmake")

# boost location
if (WIN32)
	set(Boost_INCLUDE_DIR "P:/local/boost_1_55_0")
	set(Boost_LIBRARY_DIR "P:/local/boost_1_55_0/lib64-msvc-10.0")
endif()

# autodesk maya installation location
if(WIN32)
	set(maya_DIR	"C:/Program Files/Autodesk/Maya2012")
else()
	set(maya_DIR	"/usr/autodesk/maya2012-x64")
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
