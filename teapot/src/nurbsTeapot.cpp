#include "nurbsTeapot.h"

#include <maya/MGlobal.h>
#include <maya/MDagModifier.h>

#include <maya/MPointArray.h>
#include <maya/MDoubleArray.h>

#include <maya/MFnNurbsSurface.h>
#include <maya/MFnSet.h>
#include <maya/MSelectionList.h>

#include "teapot.h"

nurbsTeapot::nurbsTeapot() {}

nurbsTeapot::~nurbsTeapot() {}

void* nurbsTeapot::creator()
{
	return new nurbsTeapot;
}

MStatus nurbsTeapot::doIt(const MArgList& argList)
{
	MStatus stat;
	MDagModifier dagMod;
	MObject newNode = dagMod.MDagModifier::createNode("transform", MObject::kNullObj, &stat);
	dagMod.doIt();
	
	// set up knots
	MDoubleArray knotArray;
	knotArray.append(0.0); knotArray.append(0.0); knotArray.append(0.0);
	knotArray.append(1.0); knotArray.append(1.0); knotArray.append(1.0);

	// set up CVs
	MPointArray cvArray;

	// create the surface
	MFnNurbsSurface mfnNurbsSurf;

	for (unsigned int iSurf = 0; iSurf < TEAPOT_NB_PATCHES; ++iSurf)
	{
		MPoint cv;
		for (unsigned int iCV = 0; iCV < 16; ++iCV)
		{
			cv.x = teapotVertices[3 * (teapotIndices[iSurf * 16 + iCV] - 1) + 0];
			cv.z = teapotVertices[3 * (teapotIndices[iSurf * 16 + iCV] - 1) + 1];
			cv.y = teapotVertices[3 * (teapotIndices[iSurf * 16 + iCV] - 1) + 2];
			cvArray.append(cv);
		}

		mfnNurbsSurf.create(cvArray, knotArray, knotArray, 3, 3,
			MFnNurbsSurface::kOpen, MFnNurbsSurface::kOpen,
			true, newNode, &stat);

		cvArray.clear();
	}

	MFnDependencyNode nodeFn(newNode);
	nodeFn.setName("nurbsTeapot#");

	MObject initialSG;
	MSelectionList selection;

	selection.clear();
	MGlobal::getSelectionListByName("initialShadingGroup", selection);
	selection.getDependNode(0, initialSG);

	MFnSet fnSG(initialSG, &stat);
	fnSG.addMember(newNode);
	
	if (MS::kSuccess != stat)
		cerr << "surfaceCreate failed: status " << stat << endl;

	return stat;
}