set(PRT_CODECS_MAYA_PROJECT com.esri.prt.codecs.maya)

# in order to find the directory for this project we need to list all possible relative paths from the various
# start CMakeLists to the project directory
find_path(	PRT_CODECS_MAYA_PREFIX "cmake/Find${PRT_CODECS_MAYA_PROJECT}.cmake" 
			${CMAKE_CURRENT_SOURCE_DIR}/../../../${PRT_CODECS_MAYA_PROJECT}	# for tests sources
			${CMAKE_CURRENT_SOURCE_DIR}/../../${PRT_CODECS_MAYA_PROJECT}		# for normal sources
)

set(${PRT_CODECS_MAYA_PROJECT}_INCLUDE_PATH ${PRT_CODECS_MAYA_PREFIX}/install/include)

if(WIN32)
	set(LIBSUF ${CMAKE_IMPORT_LIBRARY_SUFFIX})
else()
	set(LIBSUF ${CMAKE_SHARED_LIBRARY_SUFFIX})
endif()

find_library(PRT_CODECS_MAYA_LIBRARY NAMES ${CMAKE_SHARED_LIBRARY_PREFIX}${PRT_CODECS_MAYA_PROJECT}${LIBSUF} HINTS ${PRT_CODECS_MAYA_PREFIX}/install/lib)
set(PRT_CODECS_MAYA_LIBRARIES ${PRT_CODECS_MAYA_LIBRARY} )
list(APPEND PRT_CODECS_MAYA_LIBRARIES ${PRT_CODECS_MAYA_PREFIX}/install/lib/${CMAKE_SHARED_LIBRARY_PREFIX}${PRT_CODECS_MAYA_PROJECT}${CMAKE_SHARED_LIBRARY_SUFFIX})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(${PRT_CODECS_MAYA_PROJECT} DEFAULT_MSG PRT_CODECS_MAYA_LIBRARY ${PRT_CODECS_MAYA_PROJECT}_INCLUDE_PATH)