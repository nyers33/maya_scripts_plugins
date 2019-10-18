#ifndef NURBSTEAPOT_H
#define NURBSTEAPOT_H

#include <maya/MPxCommand.h>
#include <maya/MGlobal.h>
#include <maya/MObject.h>

class nurbsTeapot : public MPxCommand
{
public:
	nurbsTeapot();
	virtual	~nurbsTeapot();
    virtual MStatus doIt( const MArgList& argList );
    static void* creator();
};


#endif