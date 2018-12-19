cmake_minimum_required(VERSION 3.13)

project(prt4maya_parent)

add_subdirectory(codec)
add_subdirectory(client)

add_dependencies(prt4maya prt4maya_codec)