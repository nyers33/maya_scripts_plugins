#ifndef TKPARAMCURVEEQSAMPLER_H
#define TKPARAMCURVEEQSAMPLER_H

#include "paramCurveEqSampler.h"

class tkParamCurveEqSampler : public paramCurveEqSampler {
public:
	tkParamCurveEqSampler() {}
	virtual	~tkParamCurveEqSampler();
	virtual MStatus computeTranslation(double param, const MPlug& plug, MDataBlock& data);
	virtual MStatus computeForward(double param, const MPlug& plug, MDataBlock& data);
	virtual MStatus computeUp(double param, const MPlug& plug, MDataBlock& data);
 
	static void* creator();
	static MStatus initialize();
	
	static MTypeId id;
	static MObject inP;
	static MObject inQ;
};
#endif