#ifndef PARAMCURVEEQSAMPLER_H
#define PARAMCURVEEQSAMPLER_H

#include <maya/MPxNode.h>

class paramCurveEqSampler : public MPxNode {
public:
	paramCurveEqSampler() {}
	virtual	~paramCurveEqSampler();
	
	virtual MStatus compute(const MPlug& plug, MDataBlock& data);
	virtual MStatus computeTranslation(double param, const MPlug& plug, MDataBlock& data) = 0;
	virtual MStatus computeForward(double param, const MPlug& plug, MDataBlock& data) = 0;
	virtual MStatus computeUp(double param, const MPlug& plug, MDataBlock& data) = 0;
	
	static MStatus initialize();
 
	static MTypeId id;
	static MObject inParam;
	static MObject outTranslation;
	static MObject outForward;
	static MObject outUp;
};
#endif