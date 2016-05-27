cmake_minimum_required(VERSION 3.2)
cmake_policy(SET CMP0015 NEW)


### project definition

set(PROJECT prt4maya_codec)
project(${PROJECT})
include(${CMAKE_CURRENT_LIST_DIR}/../common.cmake)


### codec-specific dependencies

### boost location

#if (WIN32)
#	set(ENV_BOOST_INC "$ENV{Boost_INCLUDE_DIR}")
#	set(ENV_BOOST_LIB "$ENV{Boost_LIBRARY_DIR}")
#	if(ENV_BOOST_INC)
#		set(Boost_INCLUDE_DIR ${ENV_BOOST_INC})
#	else()
#		message(FATAL_ERROR "Boost_INCLUDE_DIR environment variable has not been found")
#	endif()
#	if(ENV_BOOST_LIB)
#		set(Boost_LIBRARY_DIR ${ENV_BOOST_LIB})
#	else()
#		message(FATAL_ERROR "Boost_LIBRARY_DIR environment variable has not been found")
#	endif()
#endif()

#find_package(Boost 1.59.0 EXACT REQUIRED)
#include_directories(${Boost_INCLUDE_DIR})


### setup build target

add_library(${PROJECT} SHARED
	codecs_maya.cpp
	encoder/MayaEncoder.cpp
)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	# nop
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
	target_compile_options(${PROJECT} PUBLIC "-std=c++14 -stdlib=libc++") # $<$<CONFIG:RELEASE>:${P4M_COMPILE_FLAGS}>
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	target_compile_options(${PROJECT} PUBLIC "-std=c++11") # $<$<CONFIG:RELEASE>:${P4M_COMPILE_FLAGS}>
endif()

add_definitions(-DPRT_VERSION_MAJOR=${PRT_VERSION_MAJOR} -DPRT_VERSION_MINOR=${PRT_VERSION_MINOR})

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	target_link_libraries(${PROJECT} IPHlpApi Psapi DbgHelp)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
	# nop
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	target_link_libraries(${PROJECT} z pthread dl rt)
endif()

target_link_libraries(${PROJECT} ${PRT_LINK_LIBRARIES})
set_target_properties(${PROJECT} PROPERTIES LINKER_LANGUAGE CXX)
