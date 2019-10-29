#include "gerstnerWaveCmd.h"
#include "gerstnerWaveNode.h"

#include <maya/MDagPath.h>
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <maya/MSelectionList.h>
#include <maya/MSyntax.h>
#include <maya/MArgDatabase.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MFnMesh.h>
#include <maya/MGlobal.h>

#include <iostream>

const char *envFlag = "-en", *envFlagLong = "-envelope";

void getTimeNode(MObject& timeNode)
{
	MSelectionList sel;
	sel.clear();
	sel.add(MString("time1"));
	sel.getDependNode(0, timeNode);
}

void* gerstnerWaveCmd::creator()
{
	return new gerstnerWaveCmd;
}

MSyntax gerstnerWaveCmd::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag(envFlag, envFlagLong, MSyntax::kDouble);
	syntax.useSelectionAsDefault(true);
	syntax.setObjectType(MSyntax::kSelectionList, 0, 1);
	return syntax;
}

MStatus gerstnerWaveCmd::doIt(const MArgList &args)
{
	double env = 1.0;
	MSelectionList tgtObj;

	MArgDatabase argData(syntax(), args);
	if (argData.isFlagSet(envFlag))
		argData.getFlagArgument(envFlag, 0, env);

	argData.getCommandArgument(0, tgtObj);

	// get target object
	if (tgtObj.isEmpty()) {
		argData.getObjects(tgtObj);
		if (tgtObj.isEmpty()) {
			MGlobal::displayError("No object given. Please select a mesh object.");
			return MS::kFailure;
		}
	}

	// get MDagPath from string
	MDagPath dagMesh;
	tgtObj.getDagPath(0, dagMesh);

	// extend to shape when transform was given 
	if (dagMesh.apiType() == MFn::kTransform)
		dagMesh.extendToShape();

	// occuring error when invalid object was given
	if (dagMesh.apiType() != MFn::kMesh) {
		MGlobal::displayError("No mesh object given. Please select a mesh object.");
		return MS::kFailure;
	}

	// insert seExprMesh node to current network
	MObject origMesh = dagMesh.node();
	MFnDependencyNode meshFn(origMesh);

	MPlug inMeshPlug = meshFn.findPlug("inMesh");
	MPlug outMeshPlug;

	MPlugArray conns;
	inMeshPlug.connectedTo(conns, true, false); // find dst-side only
	if (conns.length() > 0) { // has connection
		outMeshPlug = conns[0]; // set first connection
		dgMod.disconnect(outMeshPlug, inMeshPlug);
		dgMod.doIt();
	}
	else { // has no connection
		// get transform
		// it is needed to generate construction history on the mesh
		MDagPath transform(dagMesh);
		// from mesh hierarchy to transform hierarchy 
		transform.pop();

		// copy geometry as intermediateObject
		MFnMesh fnMesh;
		MObject intermediate = fnMesh.copy(origMesh, transform.node());

		// set to intermediate object
		meshFn.setObject(intermediate);
		MPlug intermediatePlug = fnMesh.findPlug("intermediateObject");
		intermediatePlug.setValue(true);

		outMeshPlug = fnMesh.findPlug("outMesh");
	}

	// create and connect seExprNode
	MObject gwNode = dgMod.createNode(gerstnerWaveNode::id);
	MFnDependencyNode gwMeshNodeFn(gwNode);
	MString nodeName = gwMeshNodeFn.name();

	dgMod.connect(outMeshPlug, gwMeshNodeFn.findPlug("inMesh"));
	dgMod.connect(gwMeshNodeFn.findPlug("outMesh"), inMeshPlug);

	// set initial value
	gwMeshNodeFn.findPlug("envelope").setValue(env);

	// connect to time
	MObject timeNode;
	getTimeNode(timeNode);
	MFnDependencyNode fnTime(timeNode);
	dgMod.connect(fnTime.findPlug("outTime"), gwMeshNodeFn.findPlug("time"));

	// set result (node name)
	clearResult();
	setResult(gwMeshNodeFn.name());

	return redoIt();
}

MStatus gerstnerWaveCmd::undoIt()
{
	return dgMod.undoIt();
}

MStatus gerstnerWaveCmd::redoIt()
{
	return dgMod.doIt();
}
