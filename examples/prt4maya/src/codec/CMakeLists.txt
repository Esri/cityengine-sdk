cmake_minimum_required(VERSION 3.2)
cmake_policy(SET CMP0015 NEW)


### project definition

set(PROJECT prt4maya_codec)
project(${PROJECT})


### codec-specific dependencies

find_package(Boost REQUIRED) # TODO: use exact version required by prtx
include_directories(${Boost_INCLUDE_DIR})


### setup build target

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(${PROJECT} SHARED
	codecs_maya.cpp
	encoder/MayaEncoder.cpp
)

target_compile_options(${PROJECT} PUBLIC "-std=c++11") # $<$<CONFIG:RELEASE>:${P4M_COMPILE_FLAGS}>
add_definitions(-DPRT_VERSION_MAJOR=${PRT_VERSION_MAJOR} -DPRT_VERSION_MINOR=${PRT_VERSION_MINOR})

if(PRT_WINDOWS)
	target_link_libraries(${PROJECT} IPHlpApi Psapi DbgHelp)
elseif(PRT_LINUX)
	target_link_libraries(${PROJECT} z pthread dl rt)
endif()

target_link_libraries(${PROJECT} ${PRT_LINK_LIBRARIES})
set_target_properties(${PROJECT} PROPERTIES LINKER_LANGUAGE CXX)


### install target

install(TARGETS ${PROJECT}
	DESTINATION plug-ins/ext
)




