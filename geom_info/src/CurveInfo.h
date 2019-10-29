#ifndef CURVEINFO_H
#define CURVEINFO_H

#include <maya/MPxCommand.h>
#include <maya/MGlobal.h>
#include <maya/MObject.h>

class CurveInfo : public MPxCommand
{
public:
	CurveInfo();
    virtual MStatus doIt( const MArgList& argList );
    static void* creator();
};


#endif