#include "gerstnerWaveNode.h"
#include "gerstnerWaveCmd.h"

#include <maya/MFnPlugin.h>

MStatus initializePlugin( MObject obj )
{
    MStatus status;

	MFnPlugin plugin(obj, "NyersCorp", "1.0", "Any");

	status = plugin.registerNode("gerstnerWaveMesh", gerstnerWaveNode::id, gerstnerWaveNode::creator, gerstnerWaveNode::initialize);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	status = plugin.registerCommand("gerstnerWaveMesh", gerstnerWaveCmd::creator, gerstnerWaveCmd::newSyntax);
	CHECK_MSTATUS_AND_RETURN_IT(status);

    return MS::kSuccess;
}

MStatus uninitializePlugin( MObject obj )
{
    MStatus status;

	MFnPlugin plugin(obj);

	status = plugin.deregisterNode(gerstnerWaveNode::id);
    CHECK_MSTATUS_AND_RETURN_IT(status);

	status = plugin.deregisterCommand("gerstnerWaveMesh");
	CHECK_MSTATUS_AND_RETURN_IT(status);

    return MS::kSuccess;
}