#ifndef GERSTNERWAVENODE_H
#define GERSTNERWAVENODE_H

#include <maya/MPxNode.h>

class MPoint;
class MVector;
class MFnMesh;

class gerstnerWaveNode : public MPxNode
{
public:
	gerstnerWaveNode();
	virtual	~gerstnerWaveNode();
	void postConstructor();

	virtual MStatus compute(const MPlug& plug, MDataBlock& data);

	static void* creator();
	static MStatus initialize();
	virtual MStatus	setDependentsDirty(const MPlug&, MPlugArray&);

	MStatus execGW(MObject& inMesh, const MFnMesh& inMeshFn, MFnMesh& outMeshFn, const MMatrix& mat, const float envelope, const int frame, const double second);
	void evalPositionGW(double x, double z, double t, MPoint& point);
	void evalNormalGW(double x, double z, double t, MVector& normal);

	unsigned int	nWave;
	double			wavelengthMedian;
	double			amplitudeMedian;
	double			steepnessParam;
	double			windAngleMedian;
	double*			wavelength;
	double*			frequency;
	double*			phaseVelocity;
	double*			phi;
	double*			amplitude;
	double*			steepness;
	double*			windAngle;
	double*			directionX;
	double*			directionZ;

	static MTypeId	id;
	static MObject	aEnable;
	static MObject	aTime;
	static MObject	aEnvelope;
	static MObject	aInMesh;
	static MObject	aInMatrix;
	static MObject	aOutMesh;

	static MObject	aNWave;
	static MObject	aWavelengthMedian;
	static MObject	aAmplitudeMedian;
	static MObject	aSteepnessParam;
	static MObject	aWindAngleMedian;

private:
	static bool dirtyWaveParamWavelength;
	static bool dirtyWaveParamAmplitude;
	static bool dirtyWaveParamSteepness;
	static bool dirtyWaveParamWindAngle;
};

#endif