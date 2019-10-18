#include "paramCurveEqSampler.h"
#include "inclMFnPluginClass.h"

#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnCompoundAttribute.h>

MTypeId	    paramCurveEqSampler::id(0x0007ffff);
MObject		paramCurveEqSampler::inParam;
MObject		paramCurveEqSampler::outTranslation;
MObject		paramCurveEqSampler::outForward;
MObject		paramCurveEqSampler::outUp;

paramCurveEqSampler::~paramCurveEqSampler() {}

MStatus paramCurveEqSampler::initialize()
{
	MStatus				stat;
	
	MFnNumericAttribute		numericAttr;
	MFnCompoundAttribute	compAttr;
	MObject					floatAttr;

	inParam = numericAttr.create("inParam", "prm", MFnNumericData::kDouble, 0.0, &stat);
	if (!stat) {
		stat.perror("ERROR creating paramCurveEqSampler param attribute");
		return stat;
	}
	numericAttr.setStorable(false);

	outTranslation = compAttr.create("outTranslation", "trs", &stat);
	if (!stat) {
		stat.perror("ERROR creating paramCurveEqSampler translation attribute");
		return stat;
	}
	compAttr.setWritable(false);
	compAttr.setStorable(false);

	floatAttr = numericAttr.create("X Coordinate", "tx", MFnNumericData::kFloat);
	addAttribute(floatAttr);
	compAttr.addChild(floatAttr);

	floatAttr = numericAttr.create("Y Coordinate", "ty", MFnNumericData::kFloat);
	addAttribute(floatAttr);
	compAttr.addChild(floatAttr);
	
	floatAttr = numericAttr.create("Z Coordinate", "tz", MFnNumericData::kFloat);
	addAttribute(floatAttr);
	compAttr.addChild(floatAttr);

	outForward = compAttr.create("outForward", "fwd", &stat);
	if (!stat) {
		stat.perror("ERROR creating paramCurveEqSampler forward attribute");
		return stat;
	}
	compAttr.setWritable(false);
	compAttr.setStorable(false);

	floatAttr = numericAttr.create("X Forward", "fx", MFnNumericData::kFloat);
	addAttribute(floatAttr);
	compAttr.addChild(floatAttr);

	floatAttr = numericAttr.create("Y Forward", "fy", MFnNumericData::kFloat);
	addAttribute(floatAttr);
	compAttr.addChild(floatAttr);

	floatAttr = numericAttr.create("Z Forward", "fz", MFnNumericData::kFloat);
	addAttribute(floatAttr);
	compAttr.addChild(floatAttr);

	outUp = compAttr.create("outUp", "up", &stat);
	if (!stat) {
		stat.perror("ERROR creating paramCurveEqSampler up attribute");
		return stat;
	}
	compAttr.setWritable(false);
	compAttr.setStorable(false);

	floatAttr = numericAttr.create("X Up", "ux", MFnNumericData::kFloat);
	addAttribute(floatAttr);
	compAttr.addChild(floatAttr);

	floatAttr = numericAttr.create("Y Up", "uy", MFnNumericData::kFloat);
	addAttribute(floatAttr);
	compAttr.addChild(floatAttr);

	floatAttr = numericAttr.create("Z Up", "uz", MFnNumericData::kFloat);
	addAttribute(floatAttr);
	compAttr.addChild(floatAttr);

	stat = addAttribute(inParam);
	if (!stat) { stat.perror("addAttribute inParam"); return stat; }

	stat = addAttribute(outTranslation);
	if (!stat) { stat.perror("addAttribute outTranslation"); return stat; }

	stat = addAttribute(outForward);
	if (!stat) { stat.perror("addAttribute outForward"); return stat; }

	stat = addAttribute(outUp);
	if (!stat) { stat.perror("addAttribute outUp"); return stat; }

	stat = attributeAffects(inParam, outTranslation);
	if (!stat) { stat.perror("attributeAffects inParam --> outTranslation"); return stat; }
	stat = attributeAffects(inParam, outForward);
	if (!stat) { stat.perror("attributeAffects inParam --> outForward"); return stat; }
	stat = attributeAffects(inParam, outUp);
	if (!stat) { stat.perror("attributeAffects inParam --> outUp"); return stat; }

	return MS::kSuccess;
}

MStatus paramCurveEqSampler::compute(const MPlug& plug, MDataBlock& data)
{
	MStatus stat;
	
	MDataHandle inputData;
	inputData = data.inputValue(inParam, &stat);
	if (stat != MS::kSuccess)
	{
		cerr << "ERROR getting data: " << stat << endl;
		return stat;
	}
	double param = inputData.asDouble();

	if (plug == outTranslation)
	{
		stat = computeTranslation(param, plug, data);
		
		stat = data.setClean(plug);
		if (stat != MS::kSuccess)
		{
			cerr << "Error in cleaning outTranslation plug: "
				<< stat << endl;
			return stat;
		}
	}
	else if(plug == outForward)
	{
		stat = computeForward(param, plug, data);
		
		stat = data.setClean(plug);
		if (stat != MS::kSuccess)
		{
			cerr << "Error in cleaning outForward plug: "
				<< stat << endl;
			return stat;
		}
	}
	else if(plug == outUp)
	{
		stat = computeUp(param, plug, data);
		
		stat = data.setClean(plug);
		if (stat != MS::kSuccess)
		{
			cerr << "Error in cleaning outForward plug: "
				<< stat << endl;
			return stat;
		}
	}
	else
	{
		return MS::kUnknownParameter;
	}

	return MS::kSuccess;
}
