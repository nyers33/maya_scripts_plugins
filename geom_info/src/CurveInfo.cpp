#include "CurveInfo.h"
#include "util.h"

#include <maya/MPointArray.h>
#include <maya/MDoubleArray.h>
#include <maya/MObjectArray.h>

#include <maya/MFnNurbsCurve.h>

#include <sstream>

CurveInfo::CurveInfo()
{
}

void* CurveInfo::creator()
{
	return new CurveInfo;
}

MStatus CurveInfo::doIt(const MArgList& argList)
{
	MStatus stat;

    MGlobal::displayInfo( "Querying curve information!" );

	MObjectArray objects;
	getSelected(objects, MFn::kNurbsCurve);

	// iterate through the list of items returned
	for (unsigned int i = 0; i<objects.length(); ++i)
	{
		// attach a function set to the selected object
		MFnDependencyNode fn(objects[i]);

		// write the object name to the script editor
		MGlobal::displayInfo(fn.name().asChar());

		MFnNurbsCurve curveFn(objects[i], &stat);
		CHECK_MSTATUS_AND_RETURN_IT(stat);

		MString curveType = curveFn.typeName();
		MGlobal::displayInfo(curveType);

		MFnNurbsCurve::Form curveForm = curveFn.form();
		std::stringstream ssForm;
		ssForm << "curve form: " << curveForm;
		MGlobal::displayInfo(ssForm.str().c_str());

		int curveDegree = curveFn.degree();
		std::stringstream ssDegree;
		ssDegree << "curve degree: " << curveDegree;
		MGlobal::displayInfo(ssDegree.str().c_str());

		MPointArray cvs;
		MDoubleArray knots;

		curveFn.getCVs(cvs, MSpace::kWorld);
		std::stringstream ssCV;
		ssCV << "control vertices" << std::endl;
		for (unsigned int iCV = 0; iCV < cvs.length(); ++iCV)
		{
			ssCV << cvs[iCV].x << ' ' << cvs[iCV].y << ' ' << cvs[iCV].z << std::endl;
		}
		ssCV << "end";

		curveFn.getKnots(knots);
		std::stringstream ssKnots;
		ssKnots << "knots" << std::endl;
		for (unsigned int iK = 0; iK < knots.length(); ++iK)
		{
			ssKnots << knots[iK] << std::endl;
		}
		ssKnots << "end";

		MGlobal::displayInfo(ssCV.str().c_str());
		MGlobal::displayInfo(ssKnots.str().c_str());
	}

	if (objects.length() == 0)
		MGlobal::displayInfo("No curve selected!");

    return MS::kSuccess;
}