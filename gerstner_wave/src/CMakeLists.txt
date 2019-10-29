set(SOURCE_FILES
	"gerstnerWaveNode.h"
	"gerstnerWaveNode.cpp"
	"gerstnerWaveCmd.h"
	"gerstnerWaveCmd.cpp"
	
	"pluginMain.cpp"
)

find_package(Maya REQUIRED)
find_package(OpenGL REQUIRED)

include_directories(${MAYA_INCLUDE_DIR} ${OPENGL_INCLUDE_DIRS} ${GLUT_INCLUDE_DIRS})
link_directories(${MAYA_LIBRARY_DIR})

IF(WIN32 OR WIN64)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /openmp")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
ENDIF(WIN32 OR WIN64)

IF(UNIX AND NOT APPLE)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
ENDIF(UNIX AND NOT APPLE)

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} ${MAYA_LIBRARIES} ${OPENGL_LIBRARIES} ${GLUT_LIBRARY})

MAYA_PLUGIN(${PROJECT_NAME})
install(TARGETS ${PROJECT_NAME} ${MAYA_TARGET_TYPE} DESTINATION plug-ins)
