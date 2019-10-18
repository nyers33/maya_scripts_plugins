#include "nurbsTeapot.h"
#include "polyTeapot.h"

#include <maya/MFnPlugin.h>

MStatus initializePlugin( MObject obj )
{
    MStatus status;

    MFnPlugin fnPlugin( obj, "NyersCorp", "1.0", "Any" );

	status = fnPlugin.registerCommand("nurbsTeapot", nurbsTeapot::creator);
    CHECK_MSTATUS_AND_RETURN_IT( status );

	status = fnPlugin.registerCommand("polyTeapot", polyTeapot::creator);
	CHECK_MSTATUS_AND_RETURN_IT(status);

    return MS::kSuccess;
}

MStatus uninitializePlugin( MObject obj )
{
    MStatus status;

    MFnPlugin fnPlugin( obj );

    status = fnPlugin.deregisterCommand("nurbsTeapot");
    CHECK_MSTATUS_AND_RETURN_IT( status );

	status = fnPlugin.deregisterCommand("polyTeapot");
	CHECK_MSTATUS_AND_RETURN_IT(status);

    return MS::kSuccess;
}