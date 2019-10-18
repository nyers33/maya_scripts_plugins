#ifndef TKBEZIERCURVE_H
#define TKBEZIERCURVE_H

#include <maya/MPxNode.h>

class tkBezierCurve : public MPxNode {
public:
	tkBezierCurve() {}
	virtual ~tkBezierCurve();
	
	virtual MStatus compute(const MPlug& plug, MDataBlock& data);
	MObject createCurve(int &p, int &q, int &divisions, float &offset, MObject &newCurveData, MStatus &stat);
	
	static void* creator();
	static MStatus initialize();
 
	static MTypeId id;
	static MObject inP;
	static MObject inQ;
	static MObject inDivisions;
	static MObject inOffset;
	static MObject outCurve;
};
#endif