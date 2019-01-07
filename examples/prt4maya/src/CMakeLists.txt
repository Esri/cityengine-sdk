cmake_minimum_required(VERSION 3.13)

project(prt4maya CXX)

include(common.cmake)

set(CODEC_TARGET prt4maya_codec)
set(PRT4MAYA_TARGET prt4maya)

add_subdirectory(codec)
add_subdirectory(client)

add_dependencies(${PRT4MAYA_TARGET} ${CODEC_TARGET})