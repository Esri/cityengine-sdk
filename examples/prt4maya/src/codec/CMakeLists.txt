cmake_minimum_required(VERSION 3.2)
cmake_policy(SET CMP0015 NEW)


### project definition

set(PROJECT prt4maya_codec)
project(${PROJECT})
include(${CMAKE_CURRENT_LIST_DIR}/../common.cmake)


### dependencies

include_directories(${PRT_INCLUDE_PATH})


### setup build target

add_library(${PROJECT} SHARED
	codecs_maya.cpp
	encoder/MayaEncoder.cpp
)


### setup compiler flags

set(CMAKE_INCLUDE_CURRENT_DIR ON)
add_definitions(-DPRT_VERSION_MAJOR=${PRT_VERSION_MAJOR} -DPRT_VERSION_MINOR=${PRT_VERSION_MINOR})

# TODO: separate into prt/prtx-specific compiler flags and prt4maya specific compiler flags

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	# none
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
	target_compile_options(${PROJECT} PUBLIC "-std=c++14 -stdlib=libc++") # $<$<CONFIG:RELEASE>:${P4M_COMPILE_FLAGS}>
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	target_compile_options(${PROJECT} PUBLIC "-std=c++11") # $<$<CONFIG:RELEASE>:${P4M_COMPILE_FLAGS}>
endif()


### setup linker flags

target_link_libraries(${PROJECT} ${PRT_LINK_LIBRARIES})
set_target_properties(${PROJECT} PROPERTIES LINKER_LANGUAGE CXX)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	target_link_libraries(${PROJECT} IPHlpApi Psapi DbgHelp)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
	# nop
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	target_link_libraries(${PROJECT} z pthread dl rt)
endif()
