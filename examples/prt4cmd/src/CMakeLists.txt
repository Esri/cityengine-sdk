cmake_minimum_required(VERSION 2.8.10)
cmake_policy(SET CMP0015 NEW)


### project definition

set(PROJECT prt4cmd)
project(${PROJECT})


### dependencies

find_package(Boost COMPONENTS program_options filesystem)
include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIRS})
add_definitions(-DBOOST_MINOR_VERSION=${Boost_MINOR_VERSION})

if(NOT prt_DIR)
	set(prt_DIR "../../../prt/cmake")
endif()
find_package(prt CONFIG)


### build target

add_executable(${PROJECT} prt4cmd.cpp)

SET(CMAKE_INSTALL_RPATH "")
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)

include_directories(${PRT_INCLUDE_PATH})
target_link_libraries(${PROJECT} ${PRT_LIBRARY} ${Boost_LIBRARIES})

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath,'$ORIGIN'")

elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
	add_custom_command(TARGET ${PROJECT} POST_BUILD COMMAND ${CMAKE_INSTALL_NAME_TOOL}
		-change @rpath/libcom.esri.prt.core.dylib ${PRT_CORE_LIBRARY}
		${PROJECT})

endif()


# install target

set(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/../install" CACHE PATH "default install prefix" FORCE) 

install(CODE "file(REMOVE_RECURSE ${CMAKE_INSTALL_PREFIX}/*)")
install(PROGRAMS ${CMAKE_BINARY_DIR}/prt4cmd${CMAKE_EXECUTABLE_SUFFIX} DESTINATION bin)
install(FILES ${PRT_LIBRARIES} DESTINATION bin OPTIONAL)
install(FILES ${PRT_EXT_LIBRARIES} DESTINATION lib)
install(DIRECTORY ../data DESTINATION .)
install(CODE "FILE(MAKE_DIRECTORY ${CMAKE_INSTALL_PREFIX}/output)")
