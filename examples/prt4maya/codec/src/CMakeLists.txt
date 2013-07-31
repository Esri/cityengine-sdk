cmake_minimum_required(VERSION 2.8.10)
cmake_policy(SET CMP0015 NEW)


### project definition

set(PROJECT com.esri.prt.codecs.maya)
project(${PROJECT})

include(${PROJECT_SOURCE_DIR}/../../com.esri.prt.core/cmake/common.cmake)


### external dependencies

find_package(Boost NO_MODULE)
include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIR})

find_package(gdal NO_MODULE)
include_directories(${GDAL_INCLUDE_DIR})
link_directories(${GDAL_LIBRARY_DIR})

### internal dependencies

find_package(com.esri.prt.util CONFIG)
include_directories(${PRT_UTIL_INCLUDE_PATH})

find_package(com.esri.prt.core CONFIG)
include_directories(${PRT_CORE_INCLUDE_PATH})


### build target

list(APPEND SOURCES codecs_maya.cpp)
list(APPEND SOURCES codecs_maya.h)				    # msvc sln
SOURCE_GROUP(src REGULAR_EXPRESSION ".*.[h|cpp]")	# msvc sln

add_subdirectory_source_group(encoder)

add_library(${PROJECT} SHARED ${SOURCES})

if(PRT_WINDOWS)
	target_link_libraries(${PROJECT} IPHlpApi Psapi DbgHelp)
elseif(PRT_LINUX)
    target_link_libraries(${PROJECT} z pthread dl rt)
endif()

target_link_libraries(${PROJECT} ${PRT_UTIL_LIBRARY} ${PRT_CORE_LIBRARY} ${Boost_LIBRARIES} ${GDAL_LIBRARIES})

#set_target_properties(${PROJECT} PROPERTIES VERSION 1.0.0 SOVERSION 1)
set_target_properties(${PROJECT} PROPERTIES LINKER_LANGUAGE CXX)


### install target

install(CODE "MESSAGE(\"wiping installation dir\")")
install(CODE "file(REMOVE_RECURSE ${CMAKE_INSTALL_PREFIX})")
install(TARGETS ${PROJECT} DESTINATION lib EXPORT "${PROJECT}Targets")
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/encoder/IMayaOutputHandler.h DESTINATION include)


### install config file and export target

install(EXPORT "${PROJECT}Targets" DESTINATION "cmake")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/exportConfig.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT}Config.cmake COPYONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT}Config.cmake DESTINATION cmake)


### print final configuration

prt_print_build_flags()
