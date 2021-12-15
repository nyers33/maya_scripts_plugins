#include "tkBezierCurve.h"
#include "inclMFnPluginClass.h"

#include <math.h>
#include <string.h>
#include <sstream>

#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>

#include <maya/MFnNurbsCurve.h>
#include <maya/MFnNurbsCurveData.h>
#include <maya/MVectorArray.h>

#include <maya/MPointArray.h>
#include <maya/MDoubleArray.h>

#include <maya/MGlobal.h>

#include "tridiagLinAlg.h"

#ifndef M_PI
	#define M_PI 3.14159265358979323846
#endif

//#define DEBUG_PRINT;

MTypeId	    tkBezierCurve::id(0x0007fffd);

MObject	    tkBezierCurve::inP;
MObject	    tkBezierCurve::inQ;
MObject	    tkBezierCurve::inDivisions;
MObject	    tkBezierCurve::inOffset;
MObject	    tkBezierCurve::outCurve;

tkBezierCurve::~tkBezierCurve() {}

void* tkBezierCurve::creator()
{
	return new tkBezierCurve();
}

MStatus tkBezierCurve::initialize()
{
	MStatus				stat;
	
	MFnNumericAttribute		numericAttr;
	MFnTypedAttribute		typedAttr;

	inP = numericAttr.create("inP", "p", MFnNumericData::kInt, 2, &stat);
	if (!stat) {
		stat.perror("ERROR creating tkBezierCurve inP attribute");
		return stat;
	}
	numericAttr.setStorable(true);

	inQ = numericAttr.create("inQ", "q", MFnNumericData::kInt, 7, &stat);
	if (!stat) {
		stat.perror("ERROR creating tkBezierCurve inQ attribute");
		return stat;
	}
	numericAttr.setStorable(true);
	
	inDivisions = numericAttr.create("inDivisions", "div", MFnNumericData::kInt, 24, &stat);
	if ( !stat ) {
		stat.perror("ERROR creating tkBezierCurve divisions attribute");
		return stat;
	}
	numericAttr.setStorable(true);
	
	inOffset = numericAttr.create("inOffset", "off", MFnNumericData::kFloat, 0.0, &stat);
	if ( !stat ) {
		stat.perror("ERROR creating tkBezierCurve offset attribute");
		return stat;
	}
	numericAttr.setStorable(true);
	
	MObject defaultData = MObject::kNullObj;
	outCurve = typedAttr.create("outCurve", "out", MFnNurbsCurveData::kNurbsCurve, defaultData, &stat);
	if ( !stat ) {
		stat.perror("ERROR creating tkBezierCurve curve attribute");
		return stat;
	}
	typedAttr.setWritable(false);
	typedAttr.setStorable(false);
	
	stat = addAttribute(inP);
	if (!stat) { stat.perror("addAttribute inP"); return stat; }
	stat = addAttribute(inQ);
	if (!stat) { stat.perror("addAttribute inQ"); return stat; }
	stat = addAttribute(inDivisions);
	if (!stat) { stat.perror("addAttribute inDivisions"); return stat; }
	stat = addAttribute(inOffset);
	if (!stat) { stat.perror("addAttribute inOffset"); return stat; }

	stat = addAttribute(outCurve);
	if (!stat) { stat.perror("addAttribute outCurve"); return stat; }

	stat = attributeAffects(inP, outCurve);
	if (!stat) { stat.perror("attributeAffects inP --> outCurve"); return stat; }
	stat = attributeAffects(inQ, outCurve);
	if (!stat) { stat.perror("attributeAffects inQ --> outCurve"); return stat; }
	stat = attributeAffects(inDivisions, outCurve);
	if (!stat) { stat.perror("attributeAffects inDivisions --> outCurve"); return stat; }
	stat = attributeAffects(inOffset, outCurve);
	if (!stat) { stat.perror("attributeAffects inOffset --> outCurve"); return stat; }

	return MS::kSuccess;
}

MObject tkBezierCurve::createCurve(int &p, int &q, int &divisions, float &offset, MObject &newCurveData, MStatus &stat)
{
	MFnNurbsCurve curveFn;
	const int degree = 3;

	MPointArray cvs;
	MDoubleArray knots;

	MPointArray curvePoints;
	MVectorArray curveTangents;

	MObject curve;

	float step = 2.0f * static_cast<float>(M_PI) / static_cast<float>(divisions);

	if (degree == 1)
	{
		for (int iDiv = 0; iDiv <= divisions; ++iDiv)
		{
			float phi = iDiv * step;
			float r = cos(q*phi) + 2.0f;

			cvs.append(MPoint(r*cos(p*phi), r*sin(p*phi), -sin(q*phi)));
			knots.append(static_cast<float>(iDiv));
		}

		// N - curve of degree - linear (1) = polyline
		// double-check copy
		cvs.set(cvs[0], divisions);

		curve = curveFn.create(cvs, knots, degree, MFnNurbsCurve::kPeriodic, false, false, newCurveData, &stat);
	}
	else
	{
		for (int iDiv = 0; iDiv < divisions; ++iDiv)
		{
			float phi = iDiv * step;
			float r = cos(q*phi) + 2.0f;

			curvePoints.append(MPoint(r*cos(p*phi), r*sin(p*phi), -sin(q*phi)));
			MVector tangent = MVector(-p*(2.0f + cos(q*phi))*sin(p*phi) - q*cos(p*phi)*sin(q*phi), p*cos(p*phi)*(2.0f + cos(q*phi)) - q*sin(p*phi)*sin(q*phi), -q*cos(q*phi));
			tangent.normalize();
			curveTangents.append(tangent);
		}

		double * param = new double[divisions+1];
		for (int i = 0; i <= divisions; ++i)
			param[i] = i * (2.0f * M_PI / static_cast<double>(divisions));

		float* mainDiag = new float[divisions];
		float* subDiag = new float[divisions];
		float* superDiag = new float[divisions];
		float* dRHS = new float[divisions];
		float* dRHS_upper = new float[divisions];
		float* dRHS_lower = new float[divisions];

		// tridiagonal matrix algorithm (Thomas algorithm) for periodic boundary conditions ---> Sherman-Morrison formula

		for (int i = 0; i < divisions - 1; ++i)
		{
			superDiag[i] = -9.0f / 70.0f * static_cast<float>(
				curveTangents[i].x * curveTangents[i + 1].x +
				curveTangents[i].y * curveTangents[i + 1].y +
				curveTangents[i].z * curveTangents[i + 1].z);
		}
		superDiag[divisions - 1] = -9.0f / 70.0f * static_cast<float>(
			curveTangents[0].x * curveTangents[divisions - 1].x +
			curveTangents[0].y * curveTangents[divisions - 1].y +
			curveTangents[0].z * curveTangents[divisions - 1].z);

		for (int i = 1; i < divisions; ++i)
		{
			subDiag[i] = -9.0f / 70.0f * static_cast<float>(
				curveTangents[i - 1].x * curveTangents[i].x +
				curveTangents[i - 1].y * curveTangents[i].y +
				curveTangents[i - 1].z * curveTangents[i].z);
		}
		subDiag[0] = -9.0f / 70.0f * static_cast<float>(
			curveTangents[divisions - 1].x * curveTangents[0].x +
			curveTangents[divisions - 1].y * curveTangents[0].y +
			curveTangents[divisions - 1].z * curveTangents[0].z);

		for (int i = 0; i < divisions; ++i)
		{
			mainDiag[i] = 2.0f * 6.0f / 35.0f;
		}
		mainDiag[divisions - 1] += superDiag[divisions - 1] * subDiag[0] / mainDiag[0];
		mainDiag[0] *= 2.0f;

		#ifdef DEBUG_PRINT
			std::stringstream ssSuperDiag;
			ssSuperDiag << "super diag" << std::endl;
			for (unsigned int i = 0; i < static_cast<unsigned int>(divisions); ++i)
			{
				ssSuperDiag << superDiag[i] << std::endl;
			}
			ssSuperDiag << "end";
			MGlobal::displayInfo(ssSuperDiag.str().c_str());

			std::stringstream ssSubDiag;
			ssSubDiag << "sub diag" << std::endl;
			for (unsigned int i = 0; i < static_cast<unsigned int>(divisions); ++i)
			{
				ssSubDiag << subDiag[i] << std::endl;
			}
			ssSubDiag << "end";
			MGlobal::displayInfo(ssSubDiag.str().c_str());

			std::stringstream ssMainDiag;
			ssMainDiag << "main diag" << std::endl;
			for (unsigned int i = 0; i < static_cast<unsigned int>(divisions); ++i)
			{
				ssMainDiag << mainDiag[i] << std::endl;
			}
			ssMainDiag << "end";
			MGlobal::displayInfo(ssMainDiag.str().c_str());
		#endif

		for (int i = 0; i < divisions; ++i)
		{			
			const double p0x = curvePoints[i].x;
			const double p0y = curvePoints[i].y;
			const double p0z = curvePoints[i].z;
			const double t0x = curveTangents[i].x;
			const double t0y = curveTangents[i].y;
			const double t0z = curveTangents[i].z;

			const double p3x = curvePoints[(i + 1) % divisions].x;
			const double p3y = curvePoints[(i + 1) % divisions].y;
			const double p3z = curvePoints[(i + 1) % divisions].z;
			const double t3x = curveTangents[(i + 1) % divisions].x;
			const double t3y = curveTangents[(i + 1) % divisions].y;
			const double t3z = curveTangents[(i + 1) % divisions].z;

			const double paramStart = param[i];
			const double paramEnd = param[(i + 1)];

			dRHS_upper[i] = static_cast<float>((11 * p0x*t0x) / 35. + (13 * p3x*t0x) / 70. + (11 * p0y*t0y) / 35. + (13 * p3y*t0y) / 70 + (11 * p0z*t0z) / 35. +
				(13 * p3z*t0z) / 70. - (12 * t0x*(cos(p*paramEnd) - cos(p*paramStart) +
				p*(paramEnd - paramStart)*sin(p*paramEnd))) / (pow(p, 2)*pow(paramEnd - paramStart, 2)) -
				(12 * t0x*(3 * (-2 + pow(p, 2)*pow(paramEnd - paramStart, 2))*cos(p*paramEnd) + 6 * cos(p*paramStart) +
				p*(-6 + pow(p, 2)*pow(paramEnd - paramStart, 2))*(paramEnd - paramStart)*sin(p*paramEnd))) /
				(pow(p, 4)*pow(paramEnd - paramStart, 4)) -
				(24 * t0y*((-2 + pow(p, 2)*pow(paramEnd - paramStart, 2))*cos(p*paramEnd) +
				2 * (cos(p*paramStart) + p*(-paramEnd + paramStart)*sin(p*paramEnd)))) /
				(pow(p, 3)*pow(paramEnd - paramStart, 3)) +
				(12 * t0y*(p*(-6 + pow(p, 2)*pow(paramEnd - paramStart, 2))*(paramEnd - paramStart)*
				cos(p*paramEnd) - 3 * (-2 + pow(p, 2)*pow(paramEnd - paramStart, 2))*sin(p*paramEnd) -
				6 * sin(p*paramStart))) / (pow(p, 4)*pow(paramEnd - paramStart, 4)) +
				(12 * t0y*(p*(paramEnd - paramStart)*cos(p*paramEnd) - sin(p*paramEnd) + sin(p*paramStart))) /
				(pow(p, 2)*pow(paramEnd - paramStart, 2)) +
				(24 * t0x*(2 * p*(paramEnd - paramStart)*cos(p*paramEnd) +
				(-2 + pow(p, 2)*pow(paramEnd - paramStart, 2))*sin(p*paramEnd) + 2 * sin(p*paramStart))) /
				(pow(p, 3)*pow(paramEnd - paramStart, 3)) -
				(6 * t0x*(3 * pow(p + q, 4)*cos(paramStart*(p - q)) + 3 * pow(p - q, 4)*cos(paramStart*(p + q)) +
				cos(paramEnd*q)*(3 * (pow(p, 4)*(-2 + pow(p, 2)*pow(paramEnd - paramStart, 2)) -
				pow(p, 2)*(12 + pow(p, 2)*pow(paramEnd - paramStart, 2))*pow(q, 2) -
				(2 + pow(p, 2)*pow(paramEnd - paramStart, 2))*pow(q, 4) +
				pow(paramEnd - paramStart, 2)*pow(q, 6))*cos(p*paramEnd) +
				p*(paramEnd - paramStart)*(p - q)*(p + q)*
				(pow(p, 2)*(-6 + pow(p, 2)*pow(paramEnd - paramStart, 2)) -
				2 * (9 + pow(p, 2)*pow(paramEnd - paramStart, 2))*pow(q, 2) +
				pow(paramEnd - paramStart, 2)*pow(q, 4))*sin(p*paramEnd)) +
				q*((-paramEnd + paramStart)*(p - q)*(p + q)*
				(pow(p, 4)*pow(paramEnd - paramStart, 2) - 6 * pow(q, 2) +
				pow(paramEnd - paramStart, 2)*pow(q, 4) +
				2 * pow(p, 2)*(-9 - pow(paramEnd - paramStart, 2)*pow(q, 2)))*cos(p*paramEnd) +
				6 * p*(pow(p, 4)*pow(paramEnd - paramStart, 2) - 4 * pow(q, 2) +
				pow(paramEnd - paramStart, 2)*pow(q, 4) +
				2 * pow(p, 2)*(-2 - pow(paramEnd - paramStart, 2)*pow(q, 2)))*sin(p*paramEnd))*
				sin(paramEnd*q))) / (pow(paramEnd - paramStart, 4)*pow(p - q, 4)*pow(p + q, 4)) +
				(12 * t0z*((-2 + pow(paramEnd - paramStart, 2)*pow(q, 2))*cos(paramEnd*q) +
				2 * (cos(paramStart*q) + (-paramEnd + paramStart)*q*sin(paramEnd*q)))) /
				(pow(paramEnd - paramStart, 3)*pow(q, 3)) -
				(12 * t0y*(pow(p + q, 3)*cos(paramStart*(p - q)) + pow(p - q, 3)*cos(paramStart*(p + q)) +
				2 * (-paramEnd + paramStart)*(pow(p, 4) - pow(q, 4))*cos(paramEnd*q)*sin(p*paramEnd) +
				q*(pow(p, 2)*(-6 + pow(p, 2)*pow(paramEnd - paramStart, 2)) -
				2 * (1 + pow(p, 2)*pow(paramEnd - paramStart, 2))*pow(q, 2) +
				pow(paramEnd - paramStart, 2)*pow(q, 4))*sin(p*paramEnd)*sin(paramEnd*q) +
				cos(p*paramEnd)*(p*(pow(p, 2)*(-2 + pow(p, 2)*pow(paramEnd - paramStart, 2)) -
				2 * (3 + pow(p, 2)*pow(paramEnd - paramStart, 2))*pow(q, 2) +
				pow(paramEnd - paramStart, 2)*pow(q, 4))*cos(paramEnd*q) +
				4 * p*(paramEnd - paramStart)*(p - q)*q*(p + q)*sin(paramEnd*q)))) /
				(pow(paramEnd - paramStart, 3)*pow(pow(p, 2) - pow(q, 2), 3)) -
				(6 * t0z*((paramEnd - paramStart)*q*(-6 + pow(paramEnd - paramStart, 2)*pow(q, 2))*cos(paramEnd*q) -
				3 * (-2 + pow(paramEnd - paramStart, 2)*pow(q, 2))*sin(paramEnd*q) - 6 * sin(paramStart*q))) /
				(pow(paramEnd - paramStart, 4)*pow(q, 4)) +
				(6 * t0z*((-paramEnd + paramStart)*q*cos(paramEnd*q) + sin(paramEnd*q) - sin(paramStart*q))) /
				(pow(paramEnd - paramStart, 2)*pow(q, 2)) +
				(6 * t0y*((pow(p, 2) + pow(q, 2))*cos(paramStart*q)*sin(p*paramStart) +
				p*cos(p*paramEnd)*((paramEnd - paramStart)*(p - q)*(p + q)*cos(paramEnd*q) +
				2 * q*sin(paramEnd*q)) + sin(p*paramEnd)*
				((-pow(p, 2) - pow(q, 2))*cos(paramEnd*q) +
				(paramEnd - paramStart)*(p - q)*q*(p + q)*sin(paramEnd*q)) -
				2 * p*q*cos(p*paramStart)*sin(paramStart*q))) /
				(pow(paramEnd - paramStart, 2)*pow(pow(p, 2) - pow(q, 2), 2)) +
				(12 * t0x*(2 * p*(pow(p, 2) + 3 * pow(q, 2))*cos(paramStart*q)*sin(p*paramStart) +
				sin(p*paramEnd)*(p*(pow(p, 2)*(-2 + pow(p, 2)*pow(paramEnd - paramStart, 2)) -
				2 * (3 + pow(p, 2)*pow(paramEnd - paramStart, 2))*pow(q, 2) +
				pow(paramEnd - paramStart, 2)*pow(q, 4))*cos(paramEnd*q) +
				4 * p*(paramEnd - paramStart)*(p - q)*q*(p + q)*sin(paramEnd*q)) +
				cos(p*paramEnd)*(2 * (paramEnd - paramStart)*(pow(p, 4) - pow(q, 4))*cos(paramEnd*q) -
				q*(pow(p, 2)*(-6 + pow(p, 2)*pow(paramEnd - paramStart, 2)) -
				2 * (1 + pow(p, 2)*pow(paramEnd - paramStart, 2))*pow(q, 2) +
				pow(paramEnd - paramStart, 2)*pow(q, 4))*sin(paramEnd*q)) -
				2 * q*(3 * pow(p, 2) + pow(q, 2))*cos(p*paramStart)*sin(paramStart*q))) /
				(pow(paramEnd - paramStart, 3)*pow(pow(p, 2) - pow(q, 2), 3)) -
				(6 * t0x*((-pow(p, 2) - pow(q, 2))*cos(p*paramStart)*cos(paramStart*q) +
				p*sin(p*paramEnd)*((paramEnd - paramStart)*(p - q)*(p + q)*cos(paramEnd*q) +
				2 * q*sin(paramEnd*q)) + cos(p*paramEnd)*
				((pow(p, 2) + pow(q, 2))*cos(paramEnd*q) +
				(paramEnd - paramStart)*q*(-pow(p, 2) + pow(q, 2))*sin(paramEnd*q)) -
				2 * p*q*sin(p*paramStart)*sin(paramStart*q))) /
				(pow(paramEnd - paramStart, 2)*pow(pow(p, 2) - pow(q, 2), 2)) +
				(6 * t0y*(3 * (-(pow(p, 6)*pow(paramEnd - paramStart, 2)) + 2 * pow(q, 4) -
				pow(paramEnd - paramStart, 2)*pow(q, 6) +
				pow(p, 4)*(2 + pow(paramEnd - paramStart, 2)*pow(q, 2)) +
				pow(p, 2)*pow(q, 2)*(12 + pow(paramEnd - paramStart, 2)*pow(q, 2)))*cos(paramEnd*q)*
				sin(p*paramEnd) - 3 * pow(p, 4)*sin(paramStart*(p - q)) -
				12 * pow(p, 3)*q*sin(paramStart*(p - q)) - 18 * pow(p, 2)*pow(q, 2)*sin(paramStart*(p - q)) -
				12 * p*pow(q, 3)*sin(paramStart*(p - q)) - 3 * pow(q, 4)*sin(paramStart*(p - q)) +
				(paramEnd - paramStart)*(p - q)*q*(p + q)*
				(pow(p, 4)*pow(paramEnd - paramStart, 2) - 6 * pow(q, 2) +
				pow(paramEnd - paramStart, 2)*pow(q, 4) +
				2 * pow(p, 2)*(-9 - pow(paramEnd - paramStart, 2)*pow(q, 2)))*sin(p*paramEnd)*
				sin(paramEnd*q) + p*cos(p*paramEnd)*
				((paramEnd - paramStart)*(p - q)*(p + q)*
				(pow(p, 2)*(-6 + pow(p, 2)*pow(paramEnd - paramStart, 2)) -
				2 * (9 + pow(p, 2)*pow(paramEnd - paramStart, 2))*pow(q, 2) +
				pow(paramEnd - paramStart, 2)*pow(q, 4))*cos(paramEnd*q) +
				6 * q*(pow(p, 4)*pow(paramEnd - paramStart, 2) - 4 * pow(q, 2) +
				pow(paramEnd - paramStart, 2)*pow(q, 4) +
				2 * pow(p, 2)*(-2 - pow(paramEnd - paramStart, 2)*pow(q, 2)))*sin(paramEnd*q)) -
				3 * pow(p - q, 4)*sin(paramStart*(p + q)))) /
				(pow(paramEnd - paramStart, 4)*pow(p - q, 4)*pow(p + q, 4)));

			dRHS_lower[(i + 1) % divisions] = static_cast<float>((13 * p0x*t3x) / 70. + (11 * p3x*t3x) / 35. + (13 * p0y*t3y) / 70. + (11 * p3y*t3y) / 35. + (13 * p0z*t3z) / 70. +
				(11 * p3z*t3z) / 35. + (12 * t3x*(3 * (-2 + pow(p, 2)*pow(paramEnd - paramStart, 2))*cos(p*paramEnd) +
				6 * cos(p*paramStart) + p*(-6 + pow(p, 2)*pow(paramEnd - paramStart, 2))*
				(paramEnd - paramStart)*sin(p*paramEnd))) / (pow(p, 4)*pow(paramEnd - paramStart, 4)) +
				(12 * t3y*((-2 + pow(p, 2)*pow(paramEnd - paramStart, 2))*cos(p*paramEnd) +
				2 * (cos(p*paramStart) + p*(-paramEnd + paramStart)*sin(p*paramEnd)))) /
				(pow(p, 3)*pow(paramEnd - paramStart, 3)) -
				(12 * t3y*(p*(-6 + pow(p, 2)*pow(paramEnd - paramStart, 2))*(paramEnd - paramStart)*
				cos(p*paramEnd) - 3 * (-2 + pow(p, 2)*pow(paramEnd - paramStart, 2))*sin(p*paramEnd) -
				6 * sin(p*paramStart))) / (pow(p, 4)*pow(paramEnd - paramStart, 4)) -
				(12 * t3x*(2 * p*(paramEnd - paramStart)*cos(p*paramEnd) +
				(-2 + pow(p, 2)*pow(paramEnd - paramStart, 2))*sin(p*paramEnd) + 2 * sin(p*paramStart))) /
				(pow(p, 3)*pow(paramEnd - paramStart, 3)) +
				(6 * t3z*(-((-2 + pow(paramEnd - paramStart, 2)*pow(q, 2))*cos(paramEnd*q)) - 2 * cos(paramStart*q) +
				2 * (paramEnd - paramStart)*q*sin(paramEnd*q))) / (pow(paramEnd - paramStart, 3)*pow(q, 3)) +
				(6 * t3x*(3 * pow(p + q, 4)*cos(paramStart*(p - q)) + 3 * pow(p - q, 4)*cos(paramStart*(p + q)) +
				cos(paramEnd*q)*(3 * (pow(p, 4)*(-2 + pow(p, 2)*pow(paramEnd - paramStart, 2)) -
				pow(p, 2)*(12 + pow(p, 2)*pow(paramEnd - paramStart, 2))*pow(q, 2) -
				(2 + pow(p, 2)*pow(paramEnd - paramStart, 2))*pow(q, 4) +
				pow(paramEnd - paramStart, 2)*pow(q, 6))*cos(p*paramEnd) +
				p*(paramEnd - paramStart)*(p - q)*(p + q)*
				(pow(p, 2)*(-6 + pow(p, 2)*pow(paramEnd - paramStart, 2)) -
				2 * (9 + pow(p, 2)*pow(paramEnd - paramStart, 2))*pow(q, 2) +
				pow(paramEnd - paramStart, 2)*pow(q, 4))*sin(p*paramEnd)) +
				q*(-((paramEnd - paramStart)*(p - q)*(p + q)*
				(pow(p, 4)*pow(paramEnd - paramStart, 2) - 6 * pow(q, 2) +
				pow(paramEnd - paramStart, 2)*pow(q, 4) +
				2 * pow(p, 2)*(-9 - pow(paramEnd - paramStart, 2)*pow(q, 2)))*cos(p*paramEnd)) +
				6 * p*(pow(p, 4)*pow(paramEnd - paramStart, 2) - 4 * pow(q, 2) +
				pow(paramEnd - paramStart, 2)*pow(q, 4) +
				2 * pow(p, 2)*(-2 - pow(paramEnd - paramStart, 2)*pow(q, 2)))*sin(p*paramEnd))*
				sin(paramEnd*q))) / (pow(paramEnd - paramStart, 4)*pow(p - q, 4)*pow(p + q, 4)) +
				(6 * t3y*(pow(p + q, 3)*cos(paramStart*(p - q)) + pow(p - q, 3)*cos(paramStart*(p + q)) +
				2 * (-paramEnd + paramStart)*(pow(p, 4) - pow(q, 4))*cos(paramEnd*q)*sin(p*paramEnd) +
				q*(pow(p, 2)*(-6 + pow(p, 2)*pow(paramEnd - paramStart, 2)) -
				2 * (1 + pow(p, 2)*pow(paramEnd - paramStart, 2))*pow(q, 2) +
				pow(paramEnd - paramStart, 2)*pow(q, 4))*sin(p*paramEnd)*sin(paramEnd*q) +
				cos(p*paramEnd)*(p*(pow(p, 2)*(-2 + pow(p, 2)*pow(paramEnd - paramStart, 2)) -
				2 * (3 + pow(p, 2)*pow(paramEnd - paramStart, 2))*pow(q, 2) +
				pow(paramEnd - paramStart, 2)*pow(q, 4))*cos(paramEnd*q) +
				4 * p*(paramEnd - paramStart)*(p - q)*q*(p + q)*sin(paramEnd*q)))) /
				(pow(paramEnd - paramStart, 3)*pow(pow(p, 2) - pow(q, 2), 3)) +
				(6 * t3z*((paramEnd - paramStart)*q*(-6 + pow(paramEnd - paramStart, 2)*pow(q, 2))*cos(paramEnd*q) -
				3 * (-2 + pow(paramEnd - paramStart, 2)*pow(q, 2))*sin(paramEnd*q) - 6 * sin(paramStart*q))) /
				(pow(paramEnd - paramStart, 4)*pow(q, 4)) -
				(6 * t3x*(2 * p*(pow(p, 2) + 3 * pow(q, 2))*cos(paramStart*q)*sin(p*paramStart) +
				sin(p*paramEnd)*(p*(pow(p, 2)*(-2 + pow(p, 2)*pow(paramEnd - paramStart, 2)) -
				2 * (3 + pow(p, 2)*pow(paramEnd - paramStart, 2))*pow(q, 2) +
				pow(paramEnd - paramStart, 2)*pow(q, 4))*cos(paramEnd*q) +
				4 * p*(paramEnd - paramStart)*(p - q)*q*(p + q)*sin(paramEnd*q)) +
				cos(p*paramEnd)*(2 * (paramEnd - paramStart)*(pow(p, 4) - pow(q, 4))*cos(paramEnd*q) -
				q*(pow(p, 2)*(-6 + pow(p, 2)*pow(paramEnd - paramStart, 2)) -
				2 * (1 + pow(p, 2)*pow(paramEnd - paramStart, 2))*pow(q, 2) +
				pow(paramEnd - paramStart, 2)*pow(q, 4))*sin(paramEnd*q)) -
				2 * q*(3 * pow(p, 2) + pow(q, 2))*cos(p*paramStart)*sin(paramStart*q))) /
				(pow(paramEnd - paramStart, 3)*pow(pow(p, 2) - pow(q, 2), 3)) -
				(6 * t3y*(3 * (-(pow(p, 6)*pow(paramEnd - paramStart, 2)) + 2 * pow(q, 4) -
				pow(paramEnd - paramStart, 2)*pow(q, 6) +
				pow(p, 4)*(2 + pow(paramEnd - paramStart, 2)*pow(q, 2)) +
				pow(p, 2)*pow(q, 2)*(12 + pow(paramEnd - paramStart, 2)*pow(q, 2)))*cos(paramEnd*q)*
				sin(p*paramEnd) - 3 * pow(p, 4)*sin(paramStart*(p - q)) -
				12 * pow(p, 3)*q*sin(paramStart*(p - q)) - 18 * pow(p, 2)*pow(q, 2)*sin(paramStart*(p - q)) -
				12 * p*pow(q, 3)*sin(paramStart*(p - q)) - 3 * pow(q, 4)*sin(paramStart*(p - q)) +
				(paramEnd - paramStart)*(p - q)*q*(p + q)*
				(pow(p, 4)*pow(paramEnd - paramStart, 2) - 6 * pow(q, 2) +
				pow(paramEnd - paramStart, 2)*pow(q, 4) +
				2 * pow(p, 2)*(-9 - pow(paramEnd - paramStart, 2)*pow(q, 2)))*sin(p*paramEnd)*
				sin(paramEnd*q) + p*cos(p*paramEnd)*
				((paramEnd - paramStart)*(p - q)*(p + q)*
				(pow(p, 2)*(-6 + pow(p, 2)*pow(paramEnd - paramStart, 2)) -
				2 * (9 + pow(p, 2)*pow(paramEnd - paramStart, 2))*pow(q, 2) +
				pow(paramEnd - paramStart, 2)*pow(q, 4))*cos(paramEnd*q) +
				6 * q*(pow(p, 4)*pow(paramEnd - paramStart, 2) - 4 * pow(q, 2) +
				pow(paramEnd - paramStart, 2)*pow(q, 4) +
				2 * pow(p, 2)*(-2 - pow(paramEnd - paramStart, 2)*pow(q, 2)))*sin(paramEnd*q)) -
				3 * pow(p - q, 4)*sin(paramStart*(p + q)))) /
				(pow(paramEnd - paramStart, 4)*pow(p - q, 4)*pow(p + q, 4)));
		}

		for (int i = 0; i < divisions; ++i)
			dRHS[i] = -(dRHS_upper[i] - dRHS_lower[i]);

		#ifdef DEBUG_PRINT
			std::stringstream ssDRHS;
			ssDRHS << "dRHS" << std::endl;
			for (unsigned int i = 0; i < static_cast<unsigned int>(divisions); ++i)
			{
				ssDRHS << dRHS[i] << std::endl;
			}
			ssDRHS << "end";
			MGlobal::displayInfo(ssDRHS.str().c_str());
		#endif

		float * tangentWeights = new float[divisions];

		float * linSolveA = new float[divisions];
		float * linSolveB = new float[divisions];
		float * linSolveC = new float[divisions];
		float * linSolveD = new float[divisions];
		float * uSM = new float[divisions];
		float * vSM = new float[divisions];
		float * ySM = new float[divisions];
		float * qSM = new float[divisions];

		memset(uSM, 0, sizeof(float) * divisions);
		memset(vSM, 0, sizeof(float) * divisions);

		uSM[0] = -2.0f * 6.0f / 35.0f;
		uSM[divisions-1] = superDiag[divisions-1];

		vSM[0] = 1.0f;
		vSM[divisions - 1] = -subDiag[0] / (2.0f * 6.0f / 35.0f);

		memcpy(linSolveA, subDiag, sizeof(float) * divisions);
		memcpy(linSolveB, mainDiag, sizeof(float) * divisions);
		memcpy(linSolveC, superDiag, sizeof(float) * divisions);
		memcpy(linSolveD, dRHS, sizeof(float) * divisions);
		solve_tridiagonal_in_place_destructive(linSolveD, divisions, linSolveA, linSolveB, linSolveC);
		memcpy(ySM, linSolveD, sizeof(float) * divisions);

		memcpy(linSolveA, subDiag, sizeof(float) * divisions);
		memcpy(linSolveB, mainDiag, sizeof(float) * divisions);
		memcpy(linSolveC, superDiag, sizeof(float) * divisions);
		memcpy(linSolveD, uSM, sizeof(float) * divisions);
		solve_tridiagonal_in_place_destructive(linSolveD, divisions, linSolveA, linSolveB, linSolveC);
		memcpy(qSM, linSolveD, sizeof(float) * divisions);

		float dot_v_y = 0.0;
		float dot_v_q = 0.0;
		for (int i = 0; i < divisions; ++i)
		{
			dot_v_y += (vSM[i] * ySM[i]);
			dot_v_q += (vSM[i] * qSM[i]);
		}

		for (int i = 0; i < divisions; ++i)
			tangentWeights[i] = ySM[i] - (dot_v_y / (1.0f + dot_v_q)) * qSM[i];

		#ifdef DEBUG_PRINT
			std::stringstream ssTW;
			ssTW << "tW" << std::endl;
			for (unsigned int i = 0; i < static_cast<unsigned int>(divisions); ++i)
			{
				ssTW << tangentWeights[i] << std::endl;
			}
			ssTW << "end";
			MGlobal::displayInfo(ssTW.str().c_str());
		#endif

		for (int iDiv = 0; iDiv < divisions; ++iDiv)
		{
			float phi = iDiv * step;
			float r = cos(q*phi) + 2.0f;

			cvs.append(curvePoints[iDiv]);
			cvs.append(curvePoints[iDiv] + tangentWeights[iDiv] * curveTangents[iDiv]);
			cvs.append(curvePoints[(iDiv + 1) % divisions] - tangentWeights[(iDiv + 1) % divisions] * curveTangents[(iDiv + 1) % divisions]);
		}
		// a periodic curve is a special case of a closed curve
		// the last N CVs in the curve must overlap the first N CVs
		// results in a curve with no tangent break at the seam where the ends meet
		for (int iDiv = 0; iDiv < degree; ++iDiv)
			cvs.append(cvs[iDiv]);

		#ifdef DEBUG_PRINT
			std::stringstream ssCV;
			ssCV << "control vertices" << std::endl;
			for (unsigned int iCV = 0; iCV < cvs.length(); ++iCV)
			{
				ssCV << cvs[iCV].x << ' ' << cvs[iCV].y << ' ' << cvs[iCV].z << std::endl;
			}
			ssCV << "end";
			MGlobal::displayInfo(ssCV.str().c_str());
		#endif

		for (int iKnot = -degree+1; iKnot < (divisions+1)*degree; ++iKnot)
			knots.append(iKnot);

		curve = curveFn.create(cvs, knots, degree, MFnNurbsCurve::kPeriodic, false, false, newCurveData, &stat);

		delete[] tangentWeights;

		delete[] linSolveA;
		delete[] linSolveB;
		delete[] linSolveC;
		delete[] linSolveD;
		delete[] uSM;
		delete[] vSM;
		delete[] ySM;
		delete[] qSM;

		delete[] param;
		delete[] mainDiag;
		delete[] subDiag;
		delete[] superDiag;
		delete[] dRHS;
		delete[] dRHS_upper;
		delete[] dRHS_lower;
	}

	if ( stat != MS::kSuccess )
	{
		cerr << "Error in creating curve: " << stat << endl;
		return MObject::kNullObj;
	}

	// stat = MGlobal::addToModel( curve );
	return curve;
}

MStatus tkBezierCurve::compute(const MPlug& plug, MDataBlock& data)
{
	MStatus stat;

	if ( plug == outCurve )	// create TorusKnot curve
	{
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
		
		inputData = data.inputValue( inDivisions, &stat );
		if( stat != MS::kSuccess )
		{
			cerr << "ERROR getting data: " << stat << endl;
			return stat;
		}
		int divisions = inputData.asInt();
		
		inputData = data.inputValue( inOffset, &stat );
		if( stat != MS::kSuccess )
		{
			cerr << "ERROR getting data: " << stat << endl;
			return stat;
		}
		float offset = inputData.asFloat();

		MDataHandle curveHandle = data.outputValue( tkBezierCurve::outCurve, &stat );
		if( stat != MS::kSuccess )
		{
		  cerr << "Error in getting data handle: " << stat << endl;
		  return stat;
		}
		
		MFnNurbsCurveData dataCreator;
		MObject newCurveData = dataCreator.create( &stat );
		if ( stat != MS::kSuccess ) {
		  cerr << "Error creating new nurbs curve data block: "
			   << stat << endl;
		  return stat;
		}

		/* MObject newCurve = */ createCurve(p, q, divisions, offset, newCurveData, stat);
		if( stat != MS::kSuccess )
		{
		  cerr << "Error in creating curve: " << stat << endl;
		  return stat;
		}
		
		// newCurve is the new curve object, but it has been packed
		// into the datablock we created for it, and the data block
		// is what we must put onto the plug
		curveHandle.set( newCurveData );
		
		stat = data.setClean( plug );
		if( stat != MS::kSuccess )
		{
		  cerr << "Error in cleaning outCurve plug: "
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
