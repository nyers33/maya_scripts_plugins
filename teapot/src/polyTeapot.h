#ifndef POLYTEAPOT_H
#define POLYTEAPOT_H

#include <maya/MPxCommand.h>
#include <maya/MGlobal.h>
#include <maya/MObject.h>

class polyTeapot : public MPxCommand
{
public:
	polyTeapot();
	virtual	~polyTeapot();
    virtual MStatus doIt( const MArgList& argList );
    static void* creator();
};


#endif