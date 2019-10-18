set(SOURCE_FILES
	"pluginMain.cpp"
	"inclMFnPluginClass.h"
	"paramCurveEqSampler.h"
	"paramCurveEqSampler.cpp"
	"tkParamCurveEqSampler.h"
	"tkParamCurveEqSampler.cpp"
	"tkBezierCurve.h"
	"tkBezierCurve.cpp"
)

find_package(Maya REQUIRED)
find_package(OpenGL REQUIRED)

include_directories(${MAYA_INCLUDE_DIR} ${OPENGL_INCLUDE_DIRS} ${GLUT_INCLUDE_DIRS})
link_directories(${MAYA_LIBRARY_DIR})

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} ${MAYA_LIBRARIES} ${OPENGL_LIBRARIES} ${GLUT_LIBRARY})

MAYA_PLUGIN(${PROJECT_NAME})
install(TARGETS ${PROJECT_NAME} ${MAYA_TARGET_TYPE} DESTINATION plug-ins)
