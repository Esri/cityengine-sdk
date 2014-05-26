cmake_minimum_required(VERSION 2.8)
cmake_policy(SET CMP0015 NEW)

project(bunny)

# ---

set(PRTX_LOG_PREPEND_PRETTY_FUNCTION_NAME 1)
find_package(prt CONFIG REQUIRED)
include_directories(${PRT_INCLUDE_PATH})

#include_directories(/home/shaegler/git/rjmcmc/install/include)
#link_directories(/home/shaegler/git/rjmcmc/install/lib)

find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIRS})

# ---

add_definitions(-DPRT_INSTALL_PATH="${PRT_INSTALL_PATH}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")
set(CMAKE_CXX_FLAGS_Debug "${CMAKE_CXX_FLAGS_Debug} -ggdb")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	set(CMAKE_INSTALL_RPATH "$ORIGIN")
endif()

# ---

add_executable(bunny main.cpp)
target_link_libraries(bunny ${PRT_LIBRARY} ${Boost_LIBRARIES})

# ---

add_library(voxenc SHARED voxenc.cpp VoxelEncoder.cpp)
target_link_libraries(voxenc ${PRT_LIBRARY})

# ---

install(
	TARGETS bunny voxenc
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
)
install(FILES ${PRT_LIBRARIES} DESTINATION bin OPTIONAL)
install(FILES ${PRT_EXT_LIBRARIES} DESTINATION lib)
