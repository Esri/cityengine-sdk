cmake_minimum_required(VERSION 3.3)
cmake_policy(SET CMP0015 NEW)


### project definition

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose one of: Debug Release RelWithDebInfo MinSizeRel")
endif()

set(PROJECT prt4cmd)
project(${PROJECT})


### dependencies

if(NOT prt_DIR)
	message(STATUS "prt_DIR not set, trying fallback ${PROJECT_SOURCE_DIR}/../../../prt/cmake")
	set(prt_DIR "${PROJECT_SOURCE_DIR}/../../../prt/cmake") # fallback location
endif()
find_package(prt REQUIRED CONFIG)
include_directories(${PRT_INCLUDE_PATH})
set(CESDK_VERSION "cesdk_${PRT_VERSION_MAJOR}_${PRT_VERSION_MINOR}_${PRT_VERSION_MICRO}")


### build target

if(NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)
	set(CMAKE_SKIP_RPATH FALSE)
	set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
	if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
		set(CMAKE_INSTALL_RPATH "\$ORIGIN")
	elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
		set(CMAKE_INSTALL_RPATH "@loader_path")
	endif()
endif()

add_executable(${PROJECT} prt4cmd.cpp utils.cpp)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	target_compile_definitions(${PROJECT} PRIVATE _CRT_SECURE_NO_WARNINGS)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	target_compile_options(${PROJECT} PRIVATE
		-std=c++11 -march=nocona -fvisibility=hidden -fvisibility-inlines-hidden
		-Wl,--exclude-libs,ALL -Wall -Wextra -Wunused-parameter -Wl,-rpath,'\$ORIGIN'
	)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
	target_compile_options(${PROJECT} PRIVATE -std=c++14 -stdlib=libc++)
endif()

target_link_libraries(${PROJECT} ${PRT_LINK_LIBRARIES})


### install target

set(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/../install" CACHE PATH "default install prefix" FORCE)

install(CODE "file(REMOVE_RECURSE ${CMAKE_INSTALL_PREFIX}/*)")
install(TARGETS ${PROJECT} RUNTIME DESTINATION bin)
install(FILES ${PRT_LIBRARIES} DESTINATION bin OPTIONAL)
install(FILES ${PRT_EXT_LIBRARIES} DESTINATION lib)
install(CODE "FILE(MAKE_DIRECTORY ${CMAKE_INSTALL_PREFIX}/output)")
install(FILES ${PROJECT_SOURCE_DIR}/../../../LICENSE DESTINATION .)
if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	install(FILES ${PROJECT_SOURCE_DIR}/../README_windows.md DESTINATION .)
else()
	install(FILES ${PROJECT_SOURCE_DIR}/../README_linux_osx.md DESTINATION .)
endif()


### package target

string(TOLOWER ${CMAKE_SYSTEM_NAME} PACKAGE_SYSTEM)
if(CMAKE_CXX_SIZEOF_DATA_PTR EQUAL 8)
	set(PACKAGE_ARCH "x86_64")
else()
	set(PACKAGE_ARCH "x86")
endif()

set(CPACK_PACKAGE_NAME					"esri_prt4cmd")
set(CPACK_PACKAGE_VENDOR				"Esri R&D Zurich Inc.")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY	"Command Line Example for CityEngine SDK")
set(CPACK_PACKAGE_VERSION_MAJOR			${PRT4CMD_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR			${PRT4CMD_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH			${PRT4CMD_VERSION_MICRO})
set(CPACK_PACKAGE_INSTALL_DIRECTORY		"${CPACK_PACKAGE_NAME}_${PACKAGE_SYSTEM}_${PACKAGE_ARCH}")
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
