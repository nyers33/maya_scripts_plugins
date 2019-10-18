#include "polyTeapot.h"

#include <maya/MGlobal.h>
#include <maya/MDagModifier.h>

#include <maya/MPoint.h>
#include <maya/MPointArray.h>
#include <maya/MIntArray.h>
#include <maya/MFloatArray.h>
#include <maya/MFloatVector.h>
#include <maya/MFloatVectorArray.h>

#include <maya/MVector.h>
#include <maya/MMatrix.h>
#include <maya/MMatrixArray.h>

#include <maya/MFnMesh.h>
#include <maya/MFnSet.h>
#include <maya/MSelectionList.h>

#include "teapot.h"

inline MPoint bezierCurve(const double& t, const MPoint& p0, const MPoint& p1, const MPoint& p2, const MPoint& p3)
{
	return pow(1.0 - t, 3.0) * p0 + 3.0 * pow(1.0 - t, 2.0) * t * p1 +
		3.0 * (1.0 - t) * pow(t, 2.0) * p2 + pow(t, 3.0) * p3;
}

inline MPoint bezierSurface(const double& u, const double& v, const MPointArray& cvs)
{
	MPoint surfPt;
	double dotProduct[3];

	MMatrix bezierBaseMat(bezierBase);
	MPoint uVec, vVec;
	uVec[0] = pow(u, 3.); uVec[1] = pow(u, 2.); uVec[2] = u; uVec[3] = 1;
	vVec[0] = pow(v, 3.); vVec[1] = pow(v, 2.); vVec[2] = v; vVec[3] = 1;

	MMatrix geomMat[3];
	for (int iCV = 0; iCV < 16; ++iCV)
	{
		for (int iDim = 0; iDim < 3; ++iDim)
		{
			double cv[4];
			cvs[iCV].get(cv);
	
			geomMat[iDim][iCV / 4][iCV % 4] = cv[iDim];
		}
	}

	surfPt = uVec * bezierBaseMat * geomMat[0] * bezierBaseMat.transpose();
	dotProduct[0] = surfPt.x * vVec.x + surfPt.y * vVec.y + surfPt.z * vVec.z + surfPt.w * vVec.w;

	surfPt = uVec * bezierBaseMat * geomMat[1] * bezierBaseMat.transpose();
	dotProduct[1] = surfPt.x * vVec.x + surfPt.y * vVec.y + surfPt.z * vVec.z + surfPt.w * vVec.w;

	surfPt = uVec * bezierBaseMat * geomMat[2] * bezierBaseMat.transpose();
	dotProduct[2] = surfPt.x * vVec.x + surfPt.y * vVec.y + surfPt.z * vVec.z + surfPt.w * vVec.w;

	surfPt.x = dotProduct[0]; surfPt.z = dotProduct[1]; surfPt.y = dotProduct[2]; surfPt.w = 1;
	return surfPt;
}

inline MVector bezierSurfaceNormals(const double& u, const double& v, const MPointArray& cvs)
{
	MVector uTangent, vTangent, normalVec;
	MPoint aux;
	double dotProduct[3];

	MMatrix bezierBaseMat(bezierBase);
	MPoint uVec, vVec;
	uVec[0] = pow(u, 3.); uVec[1] = pow(u, 2.); uVec[2] = u; uVec[3] = 1;
	vVec[0] = pow(v, 3.); vVec[1] = pow(v, 2.); vVec[2] = v; vVec[3] = 1;
	
	MPoint duVec, dvVec;
	duVec[0] = 3.*pow(u, 2); duVec[1] = 2.*u; duVec[2] = 1; duVec[3] = 0;
	dvVec[0] = 3.*pow(v, 2); dvVec[1] = 2.*v; dvVec[2] = 1; dvVec[3] = 0;

	MMatrix geomMat[3];
	for (int iCV = 0; iCV < 16; ++iCV)
	{
		for (int iDim = 0; iDim < 3; ++iDim)
		{
			double cv[4];
			cvs[iCV].get(cv);

			geomMat[iDim][iCV / 4][iCV % 4] = cv[iDim];
		}
	}

	aux = duVec * bezierBaseMat * geomMat[0] * bezierBaseMat.transpose();
	dotProduct[0] = aux.x * vVec.x + aux.y * vVec.y + aux.z * vVec.z + aux.w * vVec.w;

	aux = duVec * bezierBaseMat * geomMat[1] * bezierBaseMat.transpose();
	dotProduct[1] = aux.x * vVec.x + aux.y * vVec.y + aux.z * vVec.z + aux.w * vVec.w;

	aux = duVec * bezierBaseMat * geomMat[2] * bezierBaseMat.transpose();
	dotProduct[2] = aux.x * vVec.x + aux.y * vVec.y + aux.z * vVec.z + aux.w * vVec.w;

	uTangent.x = dotProduct[0]; uTangent.y = dotProduct[1]; uTangent.z = dotProduct[2];

	aux = uVec * bezierBaseMat * geomMat[0] * bezierBaseMat.transpose();
	dotProduct[0] = aux.x * dvVec.x + aux.y * dvVec.y + aux.z * dvVec.z + aux.w * dvVec.w;

	aux = uVec * bezierBaseMat * geomMat[1] * bezierBaseMat.transpose();
	dotProduct[1] = aux.x * dvVec.x + aux.y * dvVec.y + aux.z * dvVec.z + aux.w * dvVec.w;

	aux = uVec * bezierBaseMat * geomMat[2] * bezierBaseMat.transpose();
	dotProduct[2] = aux.x * dvVec.x + aux.y * dvVec.y + aux.z * dvVec.z + aux.w * dvVec.w;

	vTangent.x = dotProduct[0]; vTangent.y = dotProduct[1]; vTangent.z = dotProduct[2];

	uTangent.normalize();
	vTangent.normalize();

	normalVec = (uTangent ^ vTangent).normal();
	return MVector(-normalVec.x, -normalVec.z, -normalVec.y);;
}

inline void getVertexIndexs(MIntArray& vlist)
{
	for (unsigned int i = 0; i<vlist.length(); ++i)
		vlist[i] = i;
}

polyTeapot::polyTeapot() {}

polyTeapot::~polyTeapot() {}

void* polyTeapot::creator()
{
	return new polyTeapot;
}

MStatus polyTeapot::doIt(const MArgList& argList)
{
	MStatus stat;
	MDagModifier dagMod;
	MObject newNode = dagMod.MDagModifier::createNode("transform", MObject::kNullObj, &stat);
	dagMod.doIt();

	MPointArray vertexArray;
	MVectorArray normals;

	MPoint cv;
	MPointArray cvs;
	MMatrix cvMatrix;

	MFloatArray vArray;
	MFloatArray uArray;

	unsigned int tessellation;
	tessellation = 8;
	unsigned int nVert, nPoly;
	nVert = nPoly = 0;

	for (unsigned int iSurf = 0; iSurf < TEAPOT_NB_PATCHES; ++iSurf)
	{
		for (unsigned int iCV = 0; iCV < 16; ++iCV)
		{
			cv.x = teapotVertices[3 * (teapotIndices[iSurf * 16 + iCV] - 1) + 0];
			cv.y = teapotVertices[3 * (teapotIndices[iSurf * 16 + iCV] - 1) + 1];
			cv.z = teapotVertices[3 * (teapotIndices[iSurf * 16 + iCV] - 1) + 2];
			cvs.append(cv);
		}
		for (unsigned int v = 0; v <= tessellation; ++v)
		{
			for (unsigned int u = 0; u <= tessellation; ++u)
			{
				vertexArray.append(bezierSurface(u / static_cast<double>(tessellation), v / static_cast<double>(tessellation), cvs));
				normals.append(bezierSurfaceNormals(u / static_cast<double>(tessellation), v / static_cast<double>(tessellation), cvs));
				++nVert;
			}
		}
		cvs.clear();
	}

	// correct normals at pinched cvs - top and bottom of mesh
	for (unsigned int i = 0; i < 4 * (tessellation + 1); ++i)
	{
		normals[20 * (tessellation + 1)*(tessellation + 1) + i*(tessellation + 1)] = MFloatVector(0.0, 1.0, 0.0);
	}
	for (unsigned int i = 0; i < 4 * (tessellation + 1); ++i)
	{
		normals[28 * (tessellation + 1)*(tessellation + 1) + i*(tessellation + 1)] = MFloatVector(0.0, -1.0, 0.0);
	}

	// set up indices
	MIntArray polygonCounts;
	for (unsigned int iPoly = 0; iPoly < tessellation*tessellation*TEAPOT_NB_PATCHES; ++iPoly)
	{
		polygonCounts.append(4);
		++nPoly;
	}

	MIntArray polygonConnects;
	MIntArray uvIds;
	for (unsigned int iSurf = 0; iSurf < TEAPOT_NB_PATCHES; ++iSurf)
	{
		for (unsigned int v = 0; v < tessellation; ++v)
		{
			for (unsigned int u = 0; u < tessellation; ++u)
			{
				polygonConnects.append((tessellation + 1)*(tessellation + 1)*iSurf + u + (tessellation + 1)*(v + 0));
				polygonConnects.append((tessellation + 1)*(tessellation + 1)*iSurf + u + (tessellation + 1)*(v + 0) + 1);
				polygonConnects.append((tessellation + 1)*(tessellation + 1)*iSurf + u + (tessellation + 1)*(v + 1) + 1);
				polygonConnects.append((tessellation + 1)*(tessellation + 1)*iSurf + u + (tessellation + 1)*(v + 1));

				uvIds.append((tessellation + 1)*(tessellation + 1) + u + (tessellation + 1)*(v + 0));
				uvIds.append((tessellation + 1)*(tessellation + 1) + u + (tessellation + 1)*(v + 0) + 1);
				uvIds.append((tessellation + 1)*(tessellation + 1) + u + (tessellation + 1)*(v + 1) + 1);
				uvIds.append((tessellation + 1)*(tessellation + 1) + u + (tessellation + 1)*(v + 1));
			}
		}
	}

	for (unsigned int j = 0; j < tessellation; ++j)
	{
		for (unsigned int i = 0; i < tessellation; ++i)
		{
			uArray.append(static_cast<float>(i) / tessellation);
			vArray.append(static_cast<float>(j) / tessellation);
		}
	}

	// create the surface
	MFnMesh mfnMesh;
	mfnMesh.create(nVert, nPoly, vertexArray, polygonCounts, polygonConnects, newNode, &stat);
	
	// set normals
	MIntArray vlist(mfnMesh.numVertices());
	getVertexIndexs(vlist);
	mfnMesh.setVertexNormals(normals, vlist, MSpace::kObject);
	
	// set normals one-by-one
	//int counter = 0;
	//for (unsigned int iSurf = 0; iSurf < TEAPOT_NB_PATCHES; ++iSurf)
	//{
	//	for (unsigned int v = 0; v <= tessellation; ++v)
	//	{
	//		for (unsigned int u = 0; u <= tessellation; ++u)
	//		{
	//			stat = mfnMesh.setVertexNormal(normals[counter], counter, MSpace::kObject);
	//			counter++;
	//		}
	//	}
	//}

	//mfnMesh.clearUVs();
	//mfnMesh.setUVs(uArray,vArray);
	//stat = mfnMesh.assignUVs(polygonCounts, uvIds);

	MFnDependencyNode nodeFn(newNode);
	nodeFn.setName("polyTeapot#");

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