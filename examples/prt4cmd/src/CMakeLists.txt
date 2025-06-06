cmake_minimum_required(VERSION 3.19)
cmake_policy(SET CMP0015 NEW)
if(POLICY CMP0135)
	cmake_policy(SET CMP0135 NEW)
endif()


### project definition

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose one of: Debug Release RelWithDebInfo MinSizeRel")
endif()

project(prt4cmd CXX)


### environment

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    set(P4C_WINDOWS 1)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(P4C_LINUX 1)
endif()


### dependencies

# if prt_DIR is not provided, download PRT from its github home
if(NOT prt_DIR)
	include(FetchContent)

	if(P4C_WINDOWS)
		set(PRT_OS "win10")
		set(PRT_TC "vc1437")
	elseif(P4C_LINUX)
		set(PRT_OS "rhel8")
		set(PRT_TC "gcc112")
	endif()

	set(PRT_VERSION "3.3.11172")
	set(PRT_CLS     "${PRT_OS}-${PRT_TC}-x86_64-rel-opt")
	set(PRT_URL     "https://github.com/esri/cityengine-sdk/releases/download/${PRT_VERSION}/esri_ce_sdk-${PRT_VERSION}-${PRT_CLS}.zip")

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

add_executable(${PROJECT_NAME} prt4cmd.cpp utils.cpp)

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 20 CXX_EXTENSIONS OFF CXX_STANDARD_REQUIRED ON)

if(P4C_WINDOWS)
	target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)

elseif(P4C_LINUX)
	target_compile_options(${PROJECT_NAME} PRIVATE
		-march=nocona -fvisibility=hidden -fvisibility-inlines-hidden
		-Wl,--exclude-libs,ALL -Wall -Wextra -Wunused-parameter -Wl,-rpath,'\$ORIGIN')
	target_link_libraries(${PROJECT_NAME} PRIVATE stdc++fs)

endif()

target_include_directories(${PROJECT_NAME} PRIVATE ${PRT_INCLUDE_PATH})
target_link_libraries(${PROJECT_NAME} PRIVATE ${PRT_LINK_LIBRARIES})

if(NOT P4C_WINDOWS)
	set_target_properties(${PROJECT_NAME} PROPERTIES
		INSTALL_RPATH_USE_LINK_PATH FALSE
		SKIP_RPATH FALSE
		BUILD_WITH_INSTALL_RPATH TRUE)
	if(P4C_LINUX)
		set_target_properties(${PROJECT_NAME} PROPERTIES INSTALL_RPATH "\$ORIGIN")
	endif()
endif()


### install target

set(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/../install" CACHE PATH "default install prefix" FORCE)

install(CODE "file(REMOVE_RECURSE ${CMAKE_INSTALL_PREFIX}/*)")
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
install(FILES ${PRT_LIBRARIES} DESTINATION bin OPTIONAL)
install(FILES ${PRT_EXT_LIBRARIES} DESTINATION lib)
install(DIRECTORY ${PRT_EXTENSION_PATH}/usd DESTINATION lib MESSAGE_NEVER)
install(CODE "FILE(MAKE_DIRECTORY ${CMAKE_INSTALL_PREFIX}/output)")
install(FILES ${PROJECT_SOURCE_DIR}/../../../LICENSE DESTINATION .)
if(P4C_WINDOWS)
	install(FILES ${PROJECT_SOURCE_DIR}/../README_windows.md DESTINATION .)
else()
	install(FILES ${PROJECT_SOURCE_DIR}/../README_linux.md DESTINATION .)
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

if(P4C_WINDOWS)
	set(CPACK_GENERATOR					ZIP)
	set(CPACK_SOURCE_GENERATOR			ZIP)
	set(CPACK_ZIP_COMPONENT_INSTALL		1)
elseif(P4C_LINUX)
	set(CPACK_GENERATOR					ZIP)
	set(CPACK_SOURCE_GENERATOR			ZIP)
	set(CPACK_TGZ_COMPONENT_INSTALL		1)
endif()

include(CPack)
