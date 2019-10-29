#include "PolygonInfo.h"
#include "util.h"

#include <maya/MPointArray.h>
#include <maya/MDoubleArray.h>
#include <maya/MFloatArray.h>
#include <maya/MFloatVectorArray.h>
#include <maya/MObjectArray.h>

#include <maya/MFnMesh.h>

#include <sstream>

PolygonInfo::PolygonInfo()
{
}

void* PolygonInfo::creator()
{
	return new PolygonInfo;
}

MStatus PolygonInfo::doIt(const MArgList& argList)
{
	MStatus stat;

    MGlobal::displayInfo( "Querying poly information!" );

	MObjectArray objects;
	getSelected(objects, MFn::kMesh);

	// iterate through the list of items returned
	for (unsigned int i = 0; i<objects.length(); ++i)
	{
		// attach a function set to the selected object
		MFnDependencyNode fn(objects[i]);

		// write the object name to the script editor
		MGlobal::displayInfo(fn.name().asChar());

		MFnMesh meshFn(objects[i], &stat);
		CHECK_MSTATUS_AND_RETURN_IT(stat);

		int faceCount = meshFn.numPolygons();
		int edgeCount = meshFn.numEdges();
		int vertexCount = meshFn.numVertices();
		int UVCount = meshFn.numUVs();

		MPointArray verts;
		MFloatVectorArray normals;
		meshFn.getPoints(verts);
		meshFn.getNormals(normals, MSpace::kWorld);

		MFloatArray uvU, uvV;
		meshFn.getUVs(uvU, uvV);

		std::stringstream ssVerts;
		ssVerts << "vertices" << std::endl;
		for (unsigned int iVert = 0; iVert < verts.length(); ++iVert)
		{
			ssVerts << iVert << " | " << chop(static_cast<float>(verts[iVert].x)) << ' ' << chop(static_cast<float>(verts[iVert].y)) << ' ' << chop(static_cast<float>(verts[iVert].z)) << std::endl;
		}
		ssVerts << "end";

		std::stringstream ssNormals;
		ssNormals << "normals" << std::endl;
		for (unsigned int iNorm = 0; iNorm < normals.length(); ++iNorm)
		{
			ssNormals << iNorm << " | " << chop(normals[iNorm].x) << ' ' << chop(normals[iNorm].y) << ' ' << chop(normals[iNorm].z) << std::endl;
		}
		ssNormals << "end";

		MGlobal::displayInfo(ssVerts.str().c_str());
		MGlobal::displayInfo(ssNormals.str().c_str());
	}

	if (objects.length() == 0)
		MGlobal::displayInfo("No poly selected!");

    return MS::kSuccess;
}