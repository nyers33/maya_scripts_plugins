#ifndef POLYGONINFO_H
#define POLYGONINFO_H

#include <maya/MPxCommand.h>
#include <maya/MGlobal.h>
#include <maya/MObject.h>

class PolygonInfo : public MPxCommand
{
public:
	PolygonInfo();
    virtual MStatus doIt( const MArgList& argList );
    static void* creator();
};

#endif