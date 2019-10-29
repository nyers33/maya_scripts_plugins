#ifndef GERSTNERWAVECMD_H
#define GERSTNERWAVECMD_H

#include <maya/MPxCommand.h>
#include <maya/MSyntax.h>
#include <maya/MArgList.h>
#include <maya/MDGModifier.h>

class gerstnerWaveCmd : public MPxCommand
{
public:
	virtual MStatus	doIt(const MArgList&);
	virtual MStatus	undoIt();
	virtual MStatus redoIt();
	virtual bool isUndoable() const { return true; };
	static void* creator();
	static MSyntax newSyntax();

private:
	MDGModifier dgMod;
};

#endif