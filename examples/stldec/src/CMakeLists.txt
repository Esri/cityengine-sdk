cmake_minimum_required(VERSION 3.3)
cmake_policy(SET CMP0015 NEW)


### project definition

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose one of: Debug Release RelWithDebInfo MinSizeRel")
endif()

project(prt_stldec)


### setup build target

add_library(${PROJECT_NAME} SHARED main.cpp STLDecoder.cpp)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	target_compile_options(${PROJECT_NAME} PRIVATE -std=c++11 -march=nocona -fvisibility=hidden -fvisibility-inlines-hidden -Wl,--exclude-libs,ALL -Wall -Wextra -Wunused-parameter -Wl,-rpath,'$ORIGIN')
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
	target_compile_options(${PROJECT_NAME} PRIVATE -std=c++14 -stdlib=libc++ -fvisibility=hidden -fvisibility-inlines-hidden -Wl,--exclude-libs,ALL -Wall -Wextra -Wunused-parameter)
endif()


### dependencies

if (NOT prt_DIR)
	set(prt_DIR ${PROJECT_SOURCE_DIR}/../../../prt/cmake)
endif()
find_package(prt CONFIG REQUIRED)
set(CESDK_VERSION "cesdk_${PRT_VERSION_MAJOR}_${PRT_VERSION_MINOR}_${PRT_VERSION_MICRO}")
target_include_directories(${PROJECT_NAME} PRIVATE ${PRT_INCLUDE_PATH})
target_link_libraries(${PROJECT_NAME} PRIVATE ${PRT_CORE_LIBRARY})
target_compile_definitions(${PROJECT_NAME} PRIVATE
		-DPRT_VERSION_MAJOR=${PRT_VERSION_MAJOR}
		-DPRT_VERSION_MINOR=${PRT_VERSION_MINOR})


### install target

set(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/../install" CACHE PATH "default install prefix" FORCE)

install(CODE "file(REMOVE_RECURSE ${CMAKE_INSTALL_PREFIX}/*)")
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION lib LIBRARY DESTINATION lib)
install(FILES ${PROJECT_SOURCE_DIR}/../../../LICENSE DESTINATION .)
if(WIN32)
	install(FILES ${PROJECT_SOURCE_DIR}/../README_windows.md OPTIONAL DESTINATION .)
else()
	install(FILES ${PROJECT_SOURCE_DIR}/../README_linux_osx.md OPTIONAL DESTINATION .)
endif()


### package target

if(NOT STLDEC_VERSION_MICRO)
	set(STLDEC_VERSION_MICRO 0)
endif()

string(TOLOWER ${CMAKE_SYSTEM_NAME} PACKAGE_SYSTEM)
if(CMAKE_CXX_SIZEOF_DATA_PTR EQUAL 8)
	set(PACKAGE_ARCH "x86_64")
else()
	set(PACKAGE_ARCH "x86")
endif()

set(CPACK_PACKAGE_NAME					"esri_stldec")
set(CPACK_PACKAGE_VENDOR				"Esri R&D Zurich Inc.")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY	"Custom Decoder Example for CityEngine SDK")
set(CPACK_PACKAGE_VERSION_MAJOR			0)
set(CPACK_PACKAGE_VERSION_MINOR			0)
set(CPACK_PACKAGE_VERSION_PATCH			${STLDEC_VERSION_MICRO})
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
