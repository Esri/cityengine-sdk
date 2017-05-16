cmake_minimum_required(VERSION 3.2)

project(prt4maya)

add_subdirectory(codec)

set(PRT4MAYA_CODEC_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/codec)
add_subdirectory(client)

add_custom_target(install_${PROJECT_NAME}
	make install
	DEPENDS ${PROJECT_NAME}
	COMMENT "Installing ${PROJECT_NAME}"
)
 
