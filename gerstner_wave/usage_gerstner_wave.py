import maya.cmds as cmds

cmds.polyPlane( w=32, h=32, sx=64, sy=64 )
cmds.gerstnerWaveMesh()
cmds.setAttr( 'gerstnerWaveMesh1.wavelength', 10.0 )
cmds.setAttr( 'gerstnerWaveMesh1.amplitude', 0.25 )
cmds.setAttr( 'gerstnerWaveMesh1.steepness', 0.75 )
cmds.setAttr( 'gerstnerWaveMesh1.windangle', 1.571 )
