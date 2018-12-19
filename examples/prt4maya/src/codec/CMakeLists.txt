cmake_minimum_required(VERSION 3.13)
cmake_policy(SET CMP0015 NEW)


### project definition

project(prt4maya_codec)
include(${CMAKE_CURRENT_LIST_DIR}/../common.cmake) # TODO: could be moved to top-level script


### setup build target

add_library(${PROJECT_NAME} SHARED
	codecs_maya.cpp
	encoder/MayaEncoder.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE ${PRT_INCLUDE_PATH})


### setup compiler flags

set(CMAKE_INCLUDE_CURRENT_DIR ON)
target_compile_definitions(${PROJECT_NAME} PRIVATE
	-DPRT_VERSION_MAJOR=${PRT_VERSION_MAJOR}
	-DPRT_VERSION_MINOR=${PRT_VERSION_MINOR}
)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	# none
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
	target_compile_options(${PROJECT_NAME} PRIVATE "-std=c++14 -stdlib=libc++")
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	target_compile_options(${PROJECT_NAME} PRIVATE "-std=c++11") #TODO: exactly replicate linux compiler flags from prt compiler.cmake
endif()


### setup linker flags

target_link_libraries(${PROJECT_NAME} ${PRT_LINK_LIBRARIES})
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	target_link_libraries(${PROJECT_NAME} IPHlpApi Psapi DbgHelp)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
	# nop
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	target_link_libraries(${PROJECT_NAME} z pthread dl rt)
endif()


### install

install(TARGETS ${PROJECT_NAME} DESTINATION plug-ins/ext)
