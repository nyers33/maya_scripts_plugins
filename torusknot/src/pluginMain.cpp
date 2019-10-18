#include "paramCurveEqSampler.h"
#include "tkParamCurveEqSampler.h"
#include "tkBezierCurve.h"

#include <maya/MFnPlugin.h>

//---------------------------------------------------------------------------------------------------------------------
// initializePlugin / uninitializePlugin
//---------------------------------------------------------------------------------------------------------------------

MStatus initializePlugin(MObject obj)
{
	MStatus   status;
	MFnPlugin plugin(obj, "NyersCorp", "3.0", "Any");

	// pure virtual base node
	//status = plugin.registerNode("paramCurveEqSampler", paramCurveEqSampler::id, paramCurveEqSampler::creator, paramCurveEqSampler::initialize);
	//if (!status)
	//{
	//	status.perror("registerNode paramCurveEqSampler");
	//	return status;
	//}

	status = plugin.registerNode("tkParamCurveEqSampler", tkParamCurveEqSampler::id, tkParamCurveEqSampler::creator, tkParamCurveEqSampler::initialize);
	if (!status)
	{
		status.perror("registerNode tkParamCurveEqSampler");
		return status;
	}

	status = plugin.registerNode("tkBezierCurve", tkBezierCurve::id, tkBezierCurve::creator, tkBezierCurve::initialize);
	if (!status) {
		status.perror("registerNode tkBezierCurve");
		return status;
	}

	return MS::kSuccess;
}

MStatus uninitializePlugin(MObject obj)
{
	MStatus   status;
	MFnPlugin plugin(obj);

	// pure virtual base node
	//status = plugin.deregisterNode(paramCurveEqSampler::id);
	//if (!status)
	//{
	//	status.perror("deregisterNode paramCurveEqSampler");
	//	return status;
	//}

	status = plugin.deregisterNode(tkParamCurveEqSampler::id);
	if (!status)
	{
		status.perror("deregisterNode tkParamCurveEqSampler");
		return status;
	}
	
	status = plugin.deregisterNode(tkBezierCurve::id);
	if (!status)
	{
		status.perror("deregisterNode tkBezierCurve");
		return status;
	}

	return status;
}