###
### common helpers for both codec and client projects
###


### look for the PRT libraries

# prt_DIR must point to the cmake subfolder of the desired SDK installation
if(NOT prt_DIR)
	set(prt_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../prt/cmake")
endif()

find_package(prt CONFIG REQUIRED)
set(CESDK_VERSION "cesdk_${PRT_VERSION_MAJOR}_${PRT_VERSION_MINOR}_${PRT_VERSION_MICRO}")


### autodesk maya installation location

if(NOT maya_DIR)
	message(FATAL_ERROR "maya_DIR has not been set. please use '-Dmaya_DIR=xxx' to set it to the location of maya (must include devkit).")
endif()
message(STATUS "detected maya_DIR: ${maya_DIR}")

find_path(maya_INCLUDE_PATH NAMES "maya/MApiVersion.h" PATHS "${maya_DIR}/include" NO_DEFAULT_PATH)
message(STATUS "detected maya include dir: ${maya_INCLUDE_PATH}")

find_library(maya_LINK_LIB_FOUNDATION NAMES "Foundation" PATHS "${maya_DIR}/lib")
find_library(maya_LINK_LIB_OPENMAYA   NAMES "OpenMaya"   PATHS "${maya_DIR}/lib")
find_library(maya_LINK_LIB_OPENMAYAUI NAMES "OpenMayaUI" PATHS "${maya_DIR}/lib")
list(APPEND maya_LINK_LIBRARIES ${maya_LINK_LIB_FOUNDATION} ${maya_LINK_LIB_OPENMAYA} ${maya_LINK_LIB_OPENMAYAUI})


### plugin installation location

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_LIST_DIR}/../install" CACHE PATH "default install path" FORCE)
endif()
message(STATUS "Installing into ${CMAKE_INSTALL_PREFIX}")
