cmake_minimum_required(VERSION 3.8)

project(prt4maya_parent)

set(PRT4MAYA_CODEC_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/codec) # TODO: still needed

add_subdirectory(codec)
add_subdirectory(client)

add_dependencies(prt4maya prt4maya_codec)