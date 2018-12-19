###
### common helpers for both codec and client targets
###

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose one of: Debug Release RelWithDebInfo MinSizeRel")
endif()


### look for the PRT libraries

# prt_DIR must point to the cmake subfolder of the desired SDK installation
if(NOT prt_DIR)
	set(prt_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../prt/cmake")
endif()

find_package(prt CONFIG REQUIRED)
set(CESDK_VERSION "cesdk_${PRT_VERSION_MAJOR}_${PRT_VERSION_MINOR}_${PRT_VERSION_MICRO}")
message(STATUS "Using prt_DIR = ${prt_DIR} with version ${PRT_VERSION_MAJOR}.${PRT_VERSION_MINOR}.${PRT_VERSION_MICRO}")


### autodesk maya installation location

if(NOT maya_DIR)
	if(WIN32)
		set(maya_DIR "C:/Program Files/Autodesk/Maya2018")
	else()
		set(maya_DIR "/opt/autodesk/maya2018")
	endif()
endif()
message(STATUS "Using maya_DIR = ${maya_DIR} (use '-Dmaya_DIR=xxx' to override)")

find_path(maya_INCLUDE_PATH NAMES "maya/MApiVersion.h" PATHS "${maya_DIR}/include" NO_DEFAULT_PATH)

set(MAYA_LIB_DIR "${maya_DIR}/lib")
find_library(maya_LINK_LIB_FOUNDATION NAMES "Foundation" PATHS "${MAYA_LIB_DIR}")
find_library(maya_LINK_LIB_OPENMAYA   NAMES "OpenMaya"   PATHS "${MAYA_LIB_DIR}")
find_library(maya_LINK_LIB_OPENMAYAUI NAMES "OpenMayaUI" PATHS "${MAYA_LIB_DIR}")
list(APPEND maya_LINK_LIBRARIES ${maya_LINK_LIB_FOUNDATION} ${maya_LINK_LIB_OPENMAYA} ${maya_LINK_LIB_OPENMAYAUI})


### plugin installation location

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/../install" CACHE PATH "default install path" FORCE)
endif()
message(STATUS "Installing into ${CMAKE_INSTALL_PREFIX}")
