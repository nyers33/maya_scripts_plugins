#include "gerstnerWaveNode.h"

#include <maya/MGlobal.h>
#include <maya/MTime.h>
#include <maya/MMatrix.h>
#include <maya/MFloatMatrix.h>

#include <maya/MPointArray.h>
#include <maya/MFloatVectorArray.h>

#include <maya/MFnMesh.h>
#include <maya/MFnMeshData.h>

#include <maya/MFnTypedAttribute.h>
#include <maya/MFnMatrixAttribute.h>
#include <maya/MFnUnitAttribute.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnEnumAttribute.h>

#include <time.h>
#include <random>

#ifndef M_PI
	#define M_PI 3.14159265358979323846
#endif

MTypeId			gerstnerWaveNode::id(0x0007ffef);

bool gerstnerWaveNode::dirtyWaveParamWavelength;
bool gerstnerWaveNode::dirtyWaveParamAmplitude;
bool gerstnerWaveNode::dirtyWaveParamSteepness;
bool gerstnerWaveNode::dirtyWaveParamWindAngle;

MObject gerstnerWaveNode::aEnable;
MObject gerstnerWaveNode::aTime;
MObject gerstnerWaveNode::aEnvelope;
MObject gerstnerWaveNode::aInMesh;
MObject gerstnerWaveNode::aInMatrix;
MObject gerstnerWaveNode::aOutMesh;
	    
MObject gerstnerWaveNode::aNWave;
MObject gerstnerWaveNode::aWavelengthMedian;
MObject gerstnerWaveNode::aAmplitudeMedian;
MObject gerstnerWaveNode::aSteepnessParam;
MObject gerstnerWaveNode::aWindAngleMedian;

#define MAKE_INPUT_WITH_PARAM(attr, k, s, r, w)					\
    CHECK_MSTATUS (attr.setKeyable((k)));	\
	CHECK_MSTATUS (attr.setStorable((s)));	\
    CHECK_MSTATUS (attr.setReadable((r)));	\
	CHECK_MSTATUS (attr.setWritable((w)));

#define MAKE_INPUT(attr)					\
    CHECK_MSTATUS (attr.setKeyable(true));	\
	CHECK_MSTATUS (attr.setStorable(true));	\
    CHECK_MSTATUS (attr.setReadable(true));	\
	CHECK_MSTATUS (attr.setWritable(true));

#define MAKE_OUTPUT(attr)						\
    CHECK_MSTATUS (attr.setKeyable(false));		\
	CHECK_MSTATUS (attr.setStorable(false));	\
    CHECK_MSTATUS (attr.setReadable(true));		\
	CHECK_MSTATUS (attr.setWritable(false));

inline void getVertexIndexs(MIntArray& vlist)
{
	for (unsigned int i = 0; i<vlist.length(); ++i)
		vlist[i] = i;
}

double randMToN(double M, double N)
{
	return M + (rand() / (RAND_MAX / (N - M)));
}

gerstnerWaveNode::gerstnerWaveNode() {}

gerstnerWaveNode::~gerstnerWaveNode()
{
	delete[] wavelength;
	delete[] frequency;
	delete[] phi;
	delete[] phaseVelocity;
	delete[] amplitude;
	delete[] steepness;
	delete[] windAngle;
	delete[] directionX;
	delete[] directionZ;
}

void* gerstnerWaveNode::creator()
{
	return new gerstnerWaveNode;
}

MStatus gerstnerWaveNode::compute(const MPlug& plug, MDataBlock& data)
{
	// when input attributes are dirty this method will be called to
	// recompute the output attributes
	MStatus stat;

	if (dirtyWaveParamWavelength)
	{
		wavelengthMedian = data.inputValue(aWavelengthMedian).asDouble();
		for (unsigned int i = 0; i < nWave; ++i)
		{
			wavelength[i] = randMToN(0.5*wavelengthMedian, 2.0*wavelengthMedian);
			frequency[i] = 2.0 * M_PI / wavelength[i];
			phaseVelocity[i] = sqrt(9.81 * wavelength[i] / (2.0 * M_PI));
			phi[i] = phaseVelocity[i] * 2.0 * M_PI / wavelength[i];
		}
	}
	if (dirtyWaveParamAmplitude || dirtyWaveParamWavelength)
	{
		amplitudeMedian = data.inputValue(aAmplitudeMedian).asDouble();
		for (unsigned int i = 0; i < nWave; ++i)
			amplitude[i] = amplitudeMedian * wavelengthMedian / wavelength[i];
	}
	if (dirtyWaveParamSteepness || dirtyWaveParamWavelength)
	{
		steepnessParam = data.inputValue(aSteepnessParam).asDouble();
		for (unsigned int i = 0; i < nWave; ++i)
			steepness[i] = steepnessParam * (frequency[i] * amplitude[i] * nWave);
	}
	if (dirtyWaveParamWindAngle)
	{
		windAngleMedian = data.inputValue(aWindAngleMedian).asDouble();
		for (unsigned int i = 0; i < nWave; ++i)
		{
			windAngle[i] = randMToN(windAngleMedian - M_PI / 2.5, windAngleMedian + M_PI / 2.5);
			directionX[i] = cos(windAngle[i]);
			directionZ[i] = sin(windAngle[i]);
		}
	}

	dirtyWaveParamWavelength = false;
	dirtyWaveParamAmplitude = false;
	dirtyWaveParamSteepness = false;
	dirtyWaveParamWindAngle = false;

	if (plug == aOutMesh)
	{
		bool isEnabled = data.inputValue(aEnable).asBool();
		MObject inMesh = data.inputValue(aInMesh).asMeshTransformed();
		MMatrix mat = data.inputValue(aInMatrix).asMatrix();
		MTime time = data.inputValue(aTime).asTime();
		float envelope = data.inputValue(aEnvelope).asFloat();
		MDataHandle outMesh_hdl = data.outputValue(aOutMesh);

		if (inMesh.isNull()) {
			MString nodeName = MFnDependencyNode(thisMObject()).name();
			MGlobal::displayWarning(nodeName + " has no inMesh.");
			return MS::kFailure;
		}

		MFnMesh inMeshFn(inMesh);
		MObject outMesh;
		MFnMeshData meshDataFn;
		MFnMesh outMeshFn;
		MObject outMeshData = meshDataFn.create();
		outMesh = inMeshFn.copy(inMesh, outMeshData);
		outMeshFn.setObject(outMesh);

		// same as has no effect
		if (isEnabled == false) {
			outMesh_hdl.set(outMeshData);
			data.setClean(plug);
			return MS::kSuccess;
		}

		int frame = static_cast<int>(time.value());
		double second = time.as(MTime::kSeconds);

		// deform meshes here
		execGW(inMesh,inMeshFn,outMeshFn,mat,envelope,frame,second);

		outMesh_hdl.set(outMeshData);
		data.setClean(plug);
	}
	else
	{
		return MS::kUnknownParameter;
	}

	return MS::kSuccess;
}

MStatus gerstnerWaveNode::initialize()
{
	MStatus stat;
	srand(static_cast<unsigned int>(time(NULL)));

	MFnTypedAttribute tAttr;
	MFnMatrixAttribute mAttr;
	MFnUnitAttribute uAttr;
	MFnEnumAttribute eAttr;
	MFnNumericAttribute nAttr;

	// inputs
	aEnable = nAttr.create("enable", "en", MFnNumericData::kBoolean);
	nAttr.setDefault(true);
	MAKE_INPUT(nAttr);

	aTime = uAttr.create("time", "t", MFnUnitAttribute::kTime, 0.0);
	MAKE_INPUT(uAttr);

	aInMesh = tAttr.create("inMesh", "im", MFnData::kMesh);
	MAKE_INPUT(tAttr);

	aInMatrix = mAttr.create("inMatrix", "imat", MFnMatrixAttribute::kDouble);
	MAKE_INPUT_WITH_PARAM(tAttr, true, false, true, true);

	aEnvelope = nAttr.create("envelope", "e", MFnNumericData::kFloat, 1.f);
	MAKE_INPUT(nAttr);
	nAttr.setSoftMin(0.f);
	nAttr.setSoftMax(1.f);

	aNWave = nAttr.create("nWave", "nW", MFnNumericData::kInt, 4);
	MAKE_INPUT(nAttr);
	aWavelengthMedian = nAttr.create("wavelength", "wavelength", MFnNumericData::kDouble, 500.0f);
	MAKE_INPUT(nAttr);
	aAmplitudeMedian = nAttr.create("amplitude", "amplitude", MFnNumericData::kDouble, 20.0f);
	MAKE_INPUT(nAttr);
	aSteepnessParam = nAttr.create("steepness", "steepness", MFnNumericData::kDouble, 0.75f);
	MAKE_INPUT(nAttr);
	aWindAngleMedian = nAttr.create("windangle", "windangle", MFnNumericData::kDouble, M_PI / 2.0);
	MAKE_INPUT(nAttr);

	// outputs
	aOutMesh = tAttr.create("outMesh", "om", MFnData::kMesh);
	MAKE_OUTPUT(tAttr);

	CHECK_MSTATUS(addAttribute(aEnable));
	CHECK_MSTATUS(addAttribute(aInMesh));
	CHECK_MSTATUS(addAttribute(aOutMesh));
	CHECK_MSTATUS(addAttribute(aInMatrix));
	CHECK_MSTATUS(addAttribute(aTime));
	CHECK_MSTATUS(addAttribute(aEnvelope));

	//CHECK_MSTATUS(addAttribute(aNWave));
	CHECK_MSTATUS(addAttribute(aWavelengthMedian));
	CHECK_MSTATUS(addAttribute(aAmplitudeMedian));
	CHECK_MSTATUS(addAttribute(aSteepnessParam));
	CHECK_MSTATUS(addAttribute(aWindAngleMedian));

	CHECK_MSTATUS(attributeAffects(aEnable, aOutMesh));
	CHECK_MSTATUS(attributeAffects(aInMesh, aOutMesh));
	CHECK_MSTATUS(attributeAffects(aInMatrix, aOutMesh));
	CHECK_MSTATUS(attributeAffects(aTime, aOutMesh));
	CHECK_MSTATUS(attributeAffects(aEnvelope, aOutMesh));

	//CHECK_MSTATUS(attributeAffects(aNWave, aOutMesh));
	CHECK_MSTATUS(attributeAffects(aWavelengthMedian, aOutMesh));
	CHECK_MSTATUS(attributeAffects(aAmplitudeMedian, aOutMesh));
	CHECK_MSTATUS(attributeAffects(aSteepnessParam, aOutMesh));
	CHECK_MSTATUS(attributeAffects(aWindAngleMedian, aOutMesh));

	return MS::kSuccess;
}

void gerstnerWaveNode::postConstructor()
{
	dirtyWaveParamWavelength = dirtyWaveParamAmplitude = dirtyWaveParamSteepness = dirtyWaveParamWindAngle = true;
	
	nWave = 4;
	wavelength = new double[nWave];
	frequency = new double[nWave];
	phi = new double[nWave];
	phaseVelocity = new double[nWave];
	amplitude = new double[nWave];
	steepness = new double[nWave];
	windAngle = new double[nWave];
	directionX = new double[nWave];
	directionZ = new double[nWave];
}

MStatus gerstnerWaveNode::execGW(MObject& inMesh, const MFnMesh& inMeshFn, MFnMesh& outMeshFn, const MMatrix& mat, const float envelope, const int frame, const double second)
{
	// make inverse matrix for return to current space
	const MMatrix imat = mat.inverse();

	// make MFloatMatrix for normal transforming
	float matElems[4][4];
	mat.get(matElems);
	const MFloatMatrix ifmat = MFloatMatrix(matElems).inverse();

	MPointArray inPs, outPs;
	MFloatVectorArray inNs;
	MVectorArray outNs;

	// get necessary values for eval from the mesh
	inMeshFn.getPoints(inPs);
	unsigned int pNums = inPs.length();

	outPs.setLength(pNums);
	outNs.setLength(pNums);

	for (unsigned int i = 0; i < pNums; ++i)
	{
		MPoint vertex;
		evalPositionGW(inPs[i].x, inPs[i].z, second, vertex);
		outPs.set(vertex * imat, i);

		MVector normal;
		evalNormalGW(vertex.x, vertex.z, second, normal);
		outNs.set(normal * imat, i);
	}

	outMeshFn.setPoints(outPs);
	MIntArray vlist(inMeshFn.numVertices());
	getVertexIndexs(vlist);
	outMeshFn.setVertexNormals(outNs, vlist, MSpace::kObject);

	return MS::kSuccess;
}

void gerstnerWaveNode::evalPositionGW(double x, double z, double t, MPoint& point)
{
	point = MPoint(x, 0.0, z);
	for (unsigned int i = 0; i < gerstnerWaveNode::nWave; ++i)
	{
		point.x += gerstnerWaveNode::steepness[i] * gerstnerWaveNode::amplitude[i] * gerstnerWaveNode::directionX[i] *
			cos(gerstnerWaveNode::frequency[i] * (gerstnerWaveNode::directionX[i] * x + gerstnerWaveNode::directionZ[i] * z) + gerstnerWaveNode::phi[i] * t);
		point.z += gerstnerWaveNode::steepness[i] * gerstnerWaveNode::amplitude[i] * gerstnerWaveNode::directionZ[i] *
			cos(gerstnerWaveNode::frequency[i] * (gerstnerWaveNode::directionX[i] * x + gerstnerWaveNode::directionZ[i] * z) + gerstnerWaveNode::phi[i] * t);
		point.y += gerstnerWaveNode::amplitude[i] *
			sin(gerstnerWaveNode::frequency[i] * (gerstnerWaveNode::directionX[i] * x + gerstnerWaveNode::directionZ[i] * z) + gerstnerWaveNode::phi[i] * t);
	}
}

void gerstnerWaveNode::evalNormalGW(double x, double z, double t, MVector& normal)
{
	normal = MVector(0.0, 1.0, 0.0);
	for (unsigned int i = 0; i < gerstnerWaveNode::nWave; ++i)
	{
		double WA = gerstnerWaveNode::frequency[i] * gerstnerWaveNode::amplitude[i];
		double S = sin(gerstnerWaveNode::frequency[i] * (gerstnerWaveNode::directionX[i] * x + gerstnerWaveNode::directionZ[i] * z) + gerstnerWaveNode::phi[i] * t);
		double C = cos(gerstnerWaveNode::frequency[i] * (gerstnerWaveNode::directionX[i] * x + gerstnerWaveNode::directionZ[i] * z) + gerstnerWaveNode::phi[i] * t);
		
		normal.x -= gerstnerWaveNode::directionX[i] * WA * C;
		normal.z -= gerstnerWaveNode::directionZ[i] * WA * C; 
		normal.y -= gerstnerWaveNode::steepness[i] * WA * S;
		normal.normalize();
	}
}

MStatus gerstnerWaveNode::setDependentsDirty(const MPlug& plugBeingDirtied, MPlugArray& affectedPlugs)
{
	if (plugBeingDirtied == aWavelengthMedian)
		gerstnerWaveNode::dirtyWaveParamWavelength = true;
	else if (plugBeingDirtied == aAmplitudeMedian)
		gerstnerWaveNode::dirtyWaveParamAmplitude = true;
	else if (plugBeingDirtied == aSteepnessParam)
		gerstnerWaveNode::dirtyWaveParamSteepness = true;
	else if (plugBeingDirtied == aWindAngleMedian)
		gerstnerWaveNode::dirtyWaveParamWindAngle = true;

	return MS::kSuccess;
}