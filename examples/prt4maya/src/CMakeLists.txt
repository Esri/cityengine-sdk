cmake_minimum_required(VERSION 3.2)

project(prt4maya)
include(${CMAKE_CURRENT_LIST_DIR}/common.cmake)

add_subdirectory(codec)
add_subdirectory(client)

add_custom_target(install_${PROJECT_NAME}
	make install
	DEPENDS ${PROJECT_NAME}
	COMMENT "Installing ${PROJECT_NAME}"
)
 