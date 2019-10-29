#include "CurveInfo.h"
#include "PolygonInfo.h"
#include "SurfaceInfo.h"

#include <maya/MFnPlugin.h>

MStatus initializePlugin( MObject obj )
{
    MStatus status;
    MFnPlugin fnPlugin( obj, "NyersCorp", "1.0", "Any" );

	status = fnPlugin.registerCommand("curveInfo", CurveInfo::creator);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	status = fnPlugin.registerCommand("polygonInfo", PolygonInfo::creator);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	status = fnPlugin.registerCommand("surfaceInfo", SurfaceInfo::creator);
    CHECK_MSTATUS_AND_RETURN_IT( status );

    return MS::kSuccess;
}

MStatus uninitializePlugin( MObject obj )
{
    MStatus status;
    MFnPlugin fnPlugin( obj );

	status = fnPlugin.deregisterCommand("curveInfo");
	CHECK_MSTATUS_AND_RETURN_IT(status);

	status = fnPlugin.deregisterCommand("polygonInfo");
	CHECK_MSTATUS_AND_RETURN_IT(status);

	status = fnPlugin.deregisterCommand("surfaceInfo");
	CHECK_MSTATUS_AND_RETURN_IT(status);

    return MS::kSuccess;
}