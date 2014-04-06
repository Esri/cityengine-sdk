cmake_minimum_required( VERSION 2.8.5 )

project(PRT4PY)

set(Python_ADDITIONAL_VERSIONS "2.7")

set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_LIST_DIR}/cmake )
include( UseCython )

add_custom_target( ReplicatePythonSourceTree ALL
  ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/ReplicatePythonSourceTree.cmake ${CMAKE_CURRENT_BINARY_DIR}
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set(prt_DIR "/home/shaegler/procedural/dev/eclipse_workspaces/prt_eclipse_workspace/com.esri.prt.build/build/PRT_BC_REL-PRT_CC_OPT-PRT_TC_GCC/prt/cmake")
#set(prt_DIR "/home/shaegler/procedural/dev/builds/cesdk/esri_ce_sdk_1_1_1375_rhel6_gcc44_rel_opt/cmake")
find_package(prt REQUIRED)
include_directories(${PRT_INCLUDE_PATH})

include_directories( ${PRT4PY_SOURCE_DIR}/include )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -std=c++11")
set(CMAKE_CXX_FLAGS_Release "${CMAKE_CXX_FLAGS_Release} -O3")
set(CMAKE_CXX_FLAGS_Debug "${CMAKE_CXX_FLAGS_Debug} -ggdb")


enable_testing()
find_file( NOSETESTS_EXECUTABLE nosetests )
add_test( nosetests "${NOSETESTS_EXECUTABLE}" -v --with-xunit )

add_subdirectory( src )
add_subdirectory( bin )
