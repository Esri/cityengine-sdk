###
### common helpers for both codec and client projects


### look for the PRT libraries

# prt_DIR must point to the cmake subfolder of the desired SDK installation
if(NOT prt_DIR)
	set(prt_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../prt/cmake")
endif()

find_package(prt CONFIG REQUIRED) # TODO: make sure prt >= 1.6
include_directories(${PRT_INCLUDE_PATH})
set(CESDK_VERSION "cesdk_${PRT_VERSION_MAJOR}_${PRT_VERSION_MINOR}_${PRT_VERSION_MICRO}")


### autodesk maya installation location

set(ENV_MAYA_DIR "$ENV{maya_DIR}")
if(ENV_MAYA_DIR)
	set(maya_DIR "${ENV_MAYA_DIR}")
endif()
if(NOT maya_DIR)
	message(FATAL_ERROR "maya_DIR environment variable has not been found")
endif()
set(maya_include_path	"${maya_DIR}/include")
set(maya_library_path	"${maya_DIR}/lib")
message(STATUS "maya_DIR = ${maya_DIR}")


### plugin installation location

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_LIST_DIR}/../install" CACHE PATH "default install path" FORCE)
endif()
message(STATUS "Installing into ${CMAKE_INSTALL_PREFIX}")
