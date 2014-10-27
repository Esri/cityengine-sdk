cmake_minimum_required(VERSION 2.8.10)
cmake_policy(SET CMP0015 NEW)


### project definition

set(PROJECT prt_stlenc)
project(${PROJECT})


### dependencies

find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIRS})
add_definitions(-DBOOST_MINOR_VERSION=${Boost_MINOR_VERSION})

if (NOT prt_DIR)
	set(prt_DIR ${PROJECT_SOURCE_DIR}/../../../prt/cmake)
endif()
find_package(prt CONFIG REQUIRED)
set(CESDK_VERSION "cesdk_${PRT_VERSION_MAJOR}_${PRT_VERSION_MINOR}_${PRT_VERSION_MICRO}")


### build target

add_library(${PROJECT} SHARED main.cpp STLEncoder.cpp)

include_directories(${PRT_INCLUDE_PATH})
target_link_libraries(${PROJECT} ${PRT_LIBRARY})


### install target

set(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/../install" CACHE PATH "default install prefix" FORCE) 

install(CODE "file(REMOVE_RECURSE ${CMAKE_INSTALL_PREFIX}/*)")
install(TARGETS ${PROJECT} DESTINATION lib)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/../../../license DESTINATION .)
if(WIN32)
	install(FILES ${PROJECT_SOURCE_DIR}/../README.windows DESTINATION .)
else()
	install(FILES ${PROJECT_SOURCE_DIR}/../README.linux_osx DESTINATION .)
endif()


### package target

if(NOT STLENC_VERSION_MICRO)
	set(STLENC_VERSION_MICRO 0)
endif()

string(TOLOWER ${CMAKE_SYSTEM_NAME} PACKAGE_SYSTEM)
if(CMAKE_CXX_SIZEOF_DATA_PTR EQUAL 8)
	set(PACKAGE_ARCH "x86_64")
else()
	set(PACKAGE_ARCH "x86")
endif()

set(CPACK_PACKAGE_NAME					"esri_stlenc")
set(CPACK_PACKAGE_VENDOR				"Esri R&D Zurich Inc.")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY	"Custom Encoder Example for CityEngine SDK")
set(CPACK_PACKAGE_VERSION_MAJOR			0)
set(CPACK_PACKAGE_VERSION_MINOR			0)
set(CPACK_PACKAGE_VERSION_PATCH			${STLENC_VERSION_MICRO})
set(CPACK_PACKAGE_INSTALL_DIRECTORY		"${CPACK_PACKAGE_NAME}")
set(CPACK_PACKAGE_FILE_NAME				"${CPACK_PACKAGE_INSTALL_DIRECTORY}")

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	set(CPACK_GENERATOR					ZIP)
	set(CPACK_SOURCE_GENERATOR			ZIP)
	set(CPACK_ZIP_COMPONENT_INSTALL		1)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
	set(CPACK_GENERATOR					ZIP)
	set(CPACK_SOURCE_GENERATOR			ZIP)
	set(CPACK_TGZ_COMPONENT_INSTALL		1)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	set(CPACK_GENERATOR					ZIP)
	set(CPACK_SOURCE_GENERATOR			ZIP)
	set(CPACK_TGZ_COMPONENT_INSTALL		1)
endif()

include(CPack)

