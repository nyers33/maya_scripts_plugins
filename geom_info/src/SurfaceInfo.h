#ifndef SURFACEINFO_H
#define SURFACEINFO_H

#include <maya/MPxCommand.h>
#include <maya/MGlobal.h>
#include <maya/MObject.h>

class SurfaceInfo : public MPxCommand
{
public:
	SurfaceInfo();
    virtual MStatus doIt( const MArgList& argList );
    static void* creator();
};


#endif