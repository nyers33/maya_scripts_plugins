#include "SurfaceInfo.h"
#include "util.h"

#include <maya/MPointArray.h>
#include <maya/MDoubleArray.h>
#include <maya/MObjectArray.h>

#include <maya/MFnNurbsSurface.h>

#include <sstream>

SurfaceInfo::SurfaceInfo()
{
}

void* SurfaceInfo::creator()
{
	return new SurfaceInfo;
}

MStatus SurfaceInfo::doIt(const MArgList& argList)
{
	MStatus stat;

    MGlobal::displayInfo( "Querying nurbs surface information!" );

	MObjectArray objects;
	getSelected(objects, MFn::kNurbsSurface);

	// iterate through the list of items returned
	for (unsigned int i = 0; i<objects.length(); ++i)
	{
		// attach a function set to the selected object
		MFnDependencyNode fn(objects[i]);

		// write the object name to the script editor
		MGlobal::displayInfo(fn.name().asChar());

		MFnNurbsSurface surfaceFn(objects[i], &stat);
		CHECK_MSTATUS_AND_RETURN_IT(stat);

		MString surfaceType = surfaceFn.typeName();
		MGlobal::displayInfo(surfaceType);

		MFnNurbsSurface::Form surfaceFormInU = surfaceFn.formInU();
		MFnNurbsSurface::Form surfaceFormInV = surfaceFn.formInV();
		std::stringstream ssForm;
		ssForm << "surface form: " << surfaceFormInU << " - " << surfaceFormInV;
		MGlobal::displayInfo(ssForm.str().c_str());

		int curveDegreeU = surfaceFn.degreeU();
		int curveDegreeV = surfaceFn.degreeV();
		std::stringstream ssDegree;
		ssDegree << "surface degree: " << curveDegreeU << " - " << curveDegreeV;
		MGlobal::displayInfo(ssDegree.str().c_str());

		MPointArray cvs;
		MDoubleArray knotsU, knotsV;

		surfaceFn.getCVs(cvs, MSpace::kWorld);
		std::stringstream ssCV;
		ssCV << "control vertices" << std::endl;
		for (unsigned int iCV = 0; iCV < cvs.length(); ++iCV)
		{
			ssCV << chop(cvs[iCV].x) << ' ' << chop(cvs[iCV].y) << ' ' << chop(cvs[iCV].z) << std::endl;
		}
		ssCV << "end";

		surfaceFn.getKnotsInU(knotsU);
		surfaceFn.getKnotsInV(knotsV);
		std::stringstream ssKnotsU, ssKnotsV;
		
		ssKnotsU << "knotsU" << std::endl;
		for (unsigned int iK = 0; iK < knotsU.length(); ++iK)
		{
			ssKnotsU << knotsU[iK] << std::endl;
		}
		ssKnotsU << "end";
		
		ssKnotsV << "knotsV" << std::endl;
		for (unsigned int iK = 0; iK < knotsV.length(); ++iK)
		{
			ssKnotsV << knotsV[iK] << std::endl;
		}
		ssKnotsV << "end";

		MGlobal::displayInfo(ssCV.str().c_str());
		MGlobal::displayInfo(ssKnotsU.str().c_str());
		MGlobal::displayInfo(ssKnotsV.str().c_str());
	}

	if (objects.length() == 0)
		MGlobal::displayInfo("No nurbs surface selected!");

    return MS::kSuccess;
}