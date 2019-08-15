cmake_minimum_required(VERSION 3.3)
cmake_policy(SET CMP0015 NEW)


### project definition

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose one of: Debug Release RelWithDebInfo MinSizeRel")
endif()

project(prt_stlenc)


### dependencies

include(FetchContent)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    set(P4C_WINDOWS 1)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(P4C_LINUX 1)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    set(P4C_MACOS 1)
endif()

# if prt_DIR is not provided, download PRT from its github home
if(NOT prt_DIR)
	if(P4C_WINDOWS)
		set(PRT_OS "win10")
		set(PRT_TC "vc141")
	elseif(P4C_LINUX)
		set(PRT_OS "rhel7")
		set(PRT_TC "gcc63")
	elseif(P4C_MACOS)
		set(PRT_OS "osx12")
		set(PRT_TC "ac81")
	endif()

	set(PRT_VERSION "2.1.XXXX")
	set(PRT_CLS     "${PRT_OS}-${PRT_TC}-x86_64-rel-opt")
	set(PRT_URL     "https://github.com/esri/esri-cityengine-sdk/releases/download/${PRT_VERSION}/esri_ce_sdk-${PRT_VERSION}-${PRT_CLS}.zip")

	FetchContent_Declare(prt URL ${PRT_URL})
	FetchContent_GetProperties(prt)
	if(NOT prt_POPULATED)
		message(STATUS "Fetching PRT from ${PRT_URL}...")
		FetchContent_Populate(prt)
	endif()

	set(prt_DIR "${prt_SOURCE_DIR}/cmake")
endif()

find_package(prt CONFIG REQUIRED)
set(CESDK_VERSION "cesdk_${PRT_VERSION_MAJOR}_${PRT_VERSION_MINOR}_${PRT_VERSION_MICRO}")
message(STATUS "Using prt_DIR = ${prt_DIR} with version ${PRT_VERSION_MAJOR}.${PRT_VERSION_MINOR}.${PRT_VERSION_MICRO}")


### build target

add_library(${PROJECT_NAME} SHARED main.cpp STLEncoder.cpp)
add_definitions(-DPRT_VERSION_MAJOR=${PRT_VERSION_MAJOR} -DPRT_VERSION_MINOR=${PRT_VERSION_MINOR})

target_include_directories(${PROJECT_NAME} PRIVATE ${PRT_INCLUDE_PATH})
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	target_compile_options(${PROJECT_NAME} PRIVATE -std=c++11 -march=nocona -fvisibility=hidden -fvisibility-inlines-hidden -Wl,--exclude-libs,ALL -Wall -Wextra -Wunused-parameter -Wl,-rpath,'$ORIGIN')
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
	target_compile_options(${PROJECT_NAME} PRIVATE -std=c++14 -stdlib=libc++ -fvisibility=hidden -fvisibility-inlines-hidden -Wl,--exclude-libs,ALL -Wall -Wextra -Wunused-parameter)
endif()
target_link_libraries(${PROJECT_NAME} ${PRT_CORE_LIBRARY})


### install target

set(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/../install" CACHE PATH "default install prefix" FORCE)

install(CODE "file(REMOVE_RECURSE ${CMAKE_INSTALL_PREFIX}/*)")
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION lib LIBRARY DESTINATION lib)
install(FILES ${PROJECT_SOURCE_DIR}/../../../LICENSE DESTINATION .)
if(WIN32)
	install(FILES ${PROJECT_SOURCE_DIR}/../README_windows.md DESTINATION .)
else()
	install(FILES ${PROJECT_SOURCE_DIR}/../README_linux_osx.md DESTINATION .)
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

