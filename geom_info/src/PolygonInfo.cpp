#include "PolygonInfo.h"
#include "util.h"

#include <maya/MPointArray.h>
#include <maya/MDoubleArray.h>
#include <maya/MFloatArray.h>
#include <maya/MFloatVectorArray.h>
#include <maya/MObjectArray.h>

#include <maya/MFnMesh.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MItMeshEdge.h >
#include <maya/MItMeshVertex.h>

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

		std::stringstream ssFaces;
		ssFaces << "faces with vertexIDs" << std::endl;

		MItMeshPolygon faceIter(objects[i], &stat);
		CHECK_MSTATUS_AND_RETURN_IT(stat);
		
		for (; !faceIter.isDone(); faceIter.next())
		{
			unsigned int faceID = faceIter.index();

			MItMeshEdge edgeIter(objects[i], faceIter.currentItem());
			ssFaces << faceID;
			for (; !edgeIter.isDone(); edgeIter.next())
			{
				ssFaces << " | " << edgeIter.index(0) << " - " << edgeIter.index(1);
			}
			ssFaces << " with MItMeshEdge"<< std::endl;

			MIntArray faceEdges;
			faceIter.getEdges(faceEdges);
			ssFaces << faceID;
			for (unsigned int iVert = 0; iVert < faceEdges.length(); ++iVert)
			{
				int2 edgeVerts;
				meshFn.getEdgeVertices(faceEdges[iVert], edgeVerts);
				ssFaces << " | " << edgeVerts[0] << " - " << edgeVerts[1];
			}
			ssFaces << " with MItMeshPolygon.getEdges()" << std::endl;

			// with winding order
			MIntArray faceVerts;
			faceIter.getVertices(faceVerts);
			ssFaces << faceID << " |";
			for (unsigned int iVert = 0; iVert < faceVerts.length(); ++iVert)
			{
				ssFaces << " " << faceVerts[iVert];
			}
			ssFaces << " with MItMeshPolygon.getVertices()" << std::endl;
		}

		MGlobal::displayInfo(ssVerts.str().c_str());
		MGlobal::displayInfo(ssNormals.str().c_str());
		MGlobal::displayInfo(ssFaces.str().c_str());
	}

	if (objects.length() == 0)
		MGlobal::displayInfo("No poly selected!");

    return MS::kSuccess;
}