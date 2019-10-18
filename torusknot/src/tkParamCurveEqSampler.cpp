#include "tkParamCurveEqSampler.h"
#include "inclMFnPluginClass.h"

#include <maya/MFnNumericAttribute.h>
#include <maya/MFloatVector.h>

#include <maya/MString.h>
#include <maya/MTypeId.h>
#include <maya/MPlug.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>

#ifndef M_PI
	#define M_PI 3.14159265358979323846
#endif

MTypeId	    tkParamCurveEqSampler::id(0x0007fffe);
MObject		tkParamCurveEqSampler::inP;
MObject		tkParamCurveEqSampler::inQ;

inline double mod(double a, double b) {
	int n = int(a / b);
	a -= n*b;
	if (a < 0) a += b;
	return a;
}

tkParamCurveEqSampler::~tkParamCurveEqSampler() {}

void* tkParamCurveEqSampler::creator()
{
	return new tkParamCurveEqSampler();
}

MStatus tkParamCurveEqSampler::initialize()
{
	MStatus stat = MS::kSuccess;
	
	stat = paramCurveEqSampler::initialize();
	
	MFnNumericAttribute		numericAttr;

	inP = numericAttr.create("inP", "p", MFnNumericData::kInt, 2, &stat);
	if (!stat) {
		stat.perror("ERROR creating tkParamCurveEqSampler inP attribute");
		return stat;
	}
	numericAttr.setStorable(true);

	inQ = numericAttr.create("inQ", "q", MFnNumericData::kInt, 7, &stat);
	if (!stat) {
		stat.perror("ERROR creating tkParamCurveEqSampler inQ attribute");
		return stat;
	}
	numericAttr.setStorable(true);
	
	stat = addAttribute(inP);
	if (!stat) { stat.perror("addAttribute inP"); return stat; }
	stat = addAttribute(inQ);
	if (!stat) { stat.perror("addAttribute inQ"); return stat; }

	stat = attributeAffects(inP, outTranslation);
	if (!stat) { stat.perror("attributeAffects inP --> outTranslation"); return stat; }
	stat = attributeAffects(inP, outForward);
	if (!stat) { stat.perror("attributeAffects inP --> outForward"); return stat; }
	stat = attributeAffects(inP, outUp);
	if (!stat) { stat.perror("attributeAffects inP --> outUp"); return stat; }
	
	stat = attributeAffects(inQ, outTranslation);
	if (!stat) { stat.perror("attributeAffects inQ --> outTranslation"); return stat; }
	stat = attributeAffects(inQ, outForward);
	if (!stat) { stat.perror("attributeAffects inQ --> outForward"); return stat; }
	stat = attributeAffects(inQ, outUp);
	if (!stat) { stat.perror("attributeAffects inQ --> outUp"); return stat; }

	return MS::kSuccess;
}

MStatus tkParamCurveEqSampler::computeTranslation(double param, const MPlug& plug, MDataBlock& data)
{
	MStatus stat;

	MDataHandle inputData;

	inputData = data.inputValue(inP, &stat);
	if (stat != MS::kSuccess)
	{
		cerr << "ERROR getting data: " << stat << endl;
		return stat;
	}
	int p = inputData.asInt();

	inputData = data.inputValue(inQ, &stat);
	if (stat != MS::kSuccess)
	{
		cerr << "ERROR getting data: " << stat << endl;
		return stat;
	}
	int q = inputData.asInt();

	MDataHandle dataHandle = data.outputValue(tkParamCurveEqSampler::outTranslation, &stat);
	if (stat != MS::kSuccess)
	{
		cerr << "Error in getting data handle: " << stat << endl;
		return stat;
	}

	MFloatVector translationVec;
	param = mod(param, 2 * M_PI);
	double r = cos(q*param) + 2.0;
	translationVec.x = static_cast<float>(r*cos(p*param));
	translationVec.y = static_cast<float>(r*sin(p*param));
	translationVec.z = static_cast<float>(-sin(q*param));

	dataHandle.set3Float(translationVec.x, translationVec.y, translationVec.z);
	dataHandle.setClean();

	return MS::kSuccess;
}

MStatus tkParamCurveEqSampler::computeForward(double param, const MPlug& plug, MDataBlock& data)
{
	MStatus stat;

	MDataHandle inputData;

	inputData = data.inputValue(inP, &stat);
	if (stat != MS::kSuccess)
	{
		cerr << "ERROR getting data: " << stat << endl;
		return stat;
	}
	int p = inputData.asInt();

	inputData = data.inputValue(inQ, &stat);
	if (stat != MS::kSuccess)
	{
		cerr << "ERROR getting data: " << stat << endl;
		return stat;
	}
	int q = inputData.asInt();

	MDataHandle dataHandle = data.outputValue(tkParamCurveEqSampler::outForward, &stat);
	if (stat != MS::kSuccess)
	{
		cerr << "Error in getting data handle: " << stat << endl;
		return stat;
	}

	MFloatVector forwardVec;
	const double s_pparam = sin(p*param);
	const double s_qparam = sin(q*param);
	const double c_pparam = cos(p*param);
	const double c_qparam = cos(q*param);
	const double c_2qparam = cos(2*q*param);
	const double c_3qparam = cos(3*q*param);
	const double c_4qparam = cos(4*q*param);
	const double tDiv = sqrt((9 * pow(p, 2)) / 2. + (9 * c_qparam * pow(p, 2)) / 2. + pow(q, 2));

	// tangent
	forwardVec.x = static_cast<float>((-((2 + c_qparam)*p*s_pparam) - c_pparam * q*s_qparam) / tDiv);
	forwardVec.y = static_cast<float>((c_pparam * (2 + c_qparam)*p - q*s_pparam * s_qparam) / tDiv);
	forwardVec.z = static_cast<float>(-((c_qparam * q) / tDiv));
	forwardVec.normalize();

	dataHandle.set3Float(forwardVec.x, forwardVec.y, forwardVec.z);
	dataHandle.setClean();

	return MS::kSuccess;
}

MStatus tkParamCurveEqSampler::computeUp(double param, const MPlug& plug, MDataBlock& data)
{
	MStatus stat;

	MDataHandle inputData;

	inputData = data.inputValue(inP, &stat);
	if (stat != MS::kSuccess)
	{
		cerr << "ERROR getting data: " << stat << endl;
		return stat;
	}
	int p = inputData.asInt();

	inputData = data.inputValue(inQ, &stat);
	if (stat != MS::kSuccess)
	{
		cerr << "ERROR getting data: " << stat << endl;
		return stat;
	}
	int q = inputData.asInt();

	MDataHandle dataHandle = data.outputValue(tkParamCurveEqSampler::outUp, &stat);
	if (stat != MS::kSuccess)
	{
		cerr << "Error in getting data handle: " << stat << endl;
		return stat;
	}

	MFloatVector upVec;
	const double s_pparam = sin(p*param);
	const double s_qparam = sin(q*param);
	const double c_pparam = cos(p*param);
	const double c_qparam = cos(q*param);
	const double c_2qparam = cos(2 * q*param);
	const double c_3qparam = cos(3 * q*param);
	const double c_4qparam = cos(4 * q*param);

	double nDiv1 = 9 * pow(p, 2) + c_2qparam * pow(p, 2) + 8 * c_qparam * pow(p, 2) + 2 * pow(q, 2);
	double nDiv2 = 227 * pow(p, 6) + 16 * c_3qparam * pow(p, 6) + c_4qparam * pow(p, 6) + 189 * pow(p, 4)*pow(q, 2) - c_4qparam * pow(p, 4)*pow(q, 2) + 60 * pow(p, 2)*pow(q, 4) + 8 * pow(q, 6) +
		4 * c_2qparam * pow(p, 2)*(25 * pow(p, 4) + 15 * pow(p, 2)*pow(q, 2) - pow(q, 4)) + 16 * c_qparam * pow(p, 2)*(19 * pow(p, 4) + 16 * pow(p, 2)*pow(q, 2) + 4 * pow(q, 4));
	double nNum1 = c_qparam * (51 * pow(p, 4) + 24 * pow(p, 2)*pow(q, 2) + 4 * pow(q, 4)) + pow(p, 2)*(44 * pow(p, 2) + c_3qparam * pow(p, 2) + 20 * pow(q, 2) + 4 * c_2qparam * (3 * pow(p, 2) + pow(q, 2)));
	double nNum2 = 9 * pow(p, 2) + c_2qparam * pow(p, 2) + 8 * c_qparam * pow(p, 2) + 4 * pow(q, 2);

	// normal
	upVec.x = static_cast<float>((-(c_pparam * nNum1) + 2 * nNum2*p*q*s_pparam * s_qparam) / sqrt(nDiv1*nDiv2));
	upVec.y = static_cast<float>((-(nNum1*s_pparam) - 2 * c_pparam * nNum2*p*q*s_qparam) / sqrt(nDiv1*nDiv2));
	upVec.z = static_cast<float>((4 * pow(q, 2)*(4 * pow(p, 2) + 2 * c_qparam * pow(p, 2) + pow(q, 2))*s_qparam) / sqrt(nDiv1*nDiv2));
	upVec.normalize();

	dataHandle.set3Float(upVec.x, upVec.y, upVec.z);
	dataHandle.setClean();

	return MS::kSuccess;
}
