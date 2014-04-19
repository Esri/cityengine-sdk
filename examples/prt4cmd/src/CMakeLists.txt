cmake_minimum_required(VERSION 2.8.10)
cmake_policy(SET CMP0015 NEW)


### project definition

set(PROJECT prt4cmd)
project(${PROJECT})


### dependencies

find_package(Boost COMPONENTS system program_options filesystem)
include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIRS})

if(NOT prt_DIR)
	set(prt_DIR "${PROJECT_SOURCE_DIR}/../../../prt/cmake")
endif()
find_package(prt CONFIG)


### build target

add_executable(${PROJECT} prt4cmd.cpp)

SET(CMAKE_INSTALL_RPATH "")
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)

include_directories(${PRT_INCLUDE_PATH})
target_link_libraries(${PROJECT} ${Boost_LIBRARIES} ${PRT_LIBRARY})

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath,'$ORIGIN'")
endif()


# install target

set(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/../install" CACHE PATH "default install prefix" FORCE) 

install(CODE "file(REMOVE_RECURSE ${CMAKE_INSTALL_PREFIX}/*)")
install(PROGRAMS ${CMAKE_BINARY_DIR}/prt4cmd${CMAKE_EXECUTABLE_SUFFIX} DESTINATION bin)
install(FILES ${PRT_LIBRARIES} DESTINATION bin OPTIONAL)
install(FILES ${PRT_EXT_LIBRARIES} DESTINATION lib)
install(CODE "FILE(MAKE_DIRECTORY ${CMAKE_INSTALL_PREFIX}/output)")
