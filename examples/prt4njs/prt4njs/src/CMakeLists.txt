cmake_minimum_required(VERSION 2.8.10)


### project definition

set(PROJECT prt4njs)
project(${PROJECT})


### dependencies

find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIRS})

if(NOT prt_DIR)
	set(prt_DIR "${PROJECT_SOURCE_DIR}/../../../../prt/cmake")
endif()
find_package(prt CONFIG REQUIRED)
include_directories(${PRT_INCLUDE_PATH})

set(NODEJS_BASE "$ENV{HOME}/.node-gyp/0.10.21")
include_directories("${NODEJS_BASE}/src"})
include_directories("${NODEJS_BASE}/deps/v8/include"})
include_directories("${NODEJS_BASE}/deps/uv/include"})


### build target

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -ggdb")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -ggdb")

add_library(${PROJECT} SHARED prt4njs.cc)
target_link_libraries(${PROJECT} ${PRT_LIBRARY} ${Boost_LIBRARIES})

add_custom_command(
	TARGET ${PROJECT} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E rename $<TARGET_FILE:${PROJECT}> prt4njs.node
)


### install target

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT) 
    set(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/../install" CACHE PATH "prt4njs install prefix" FORCE) 
endif() 

install(FILES ${CMAKE_BINARY_DIR}/prt4njs.node DESTINATION .)

