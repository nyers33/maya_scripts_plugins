# turn a poly torus into a torus knot
import maya.cmds as cmds
import maya.api.OpenMaya as om
import math
cmds.createNode( 'tkParamCurveEqSampler' )
cmds.setAttr('tkParamCurveEqSampler1.inP', 5)
cmds.setAttr('tkParamCurveEqSampler1.inQ', 3)
pTorus = cmds.getAttr('tkParamCurveEqSampler1.inP')
qTorus = cmds.getAttr('tkParamCurveEqSampler1.inQ')
mainRad = 2.0
secRad = 0.4
resArc = 256
resHeight = 16
cmds.polyTorus( sx=resArc, sy=resHeight, r=mainRad, sr=secRad )

for j in range(resArc):
	for i in range(resHeight):
		cmds.setAttr( 'tkParamCurveEqSampler1.inParam', 2.0 * math.pi * float(-j) / resArc )
		rotTorus = (j + 1.0) * 2.0 * math.pi / resArc
		translTorus = mainRad * om.MVector( math.cos(rotTorus), 0.0, -math.sin(rotTorus) )
		cmds.move( -translTorus.x, -translTorus.y, -translTorus.z, 'pTorus1.vtx['+str(j+i*resArc)+']', relative=True )
		cmds.rotate( 0, str(-(j + 1.0) * 360.0 / resArc) + 'deg', 0, 'pTorus1.vtx['+str(j+i*resArc)+']', pivot=(0, 0, 0) )
		vecTrns = om.MVector( cmds.getAttr( 'tkParamCurveEqSampler1.outTranslation' )[0] )
		vecFwd =  om.MVector( cmds.getAttr( 'tkParamCurveEqSampler1.outForward' )[0] )
		vecUp =  om.MVector( cmds.getAttr( 'tkParamCurveEqSampler1.outUp' )[0] )
		vecCrs = vecFwd ^ vecUp
		matTransformation = om.MMatrix( [[vecFwd.x, vecUp.x, vecCrs.x, vecTrns.x],[vecFwd.y, vecUp.y, vecCrs.y, vecTrns.y],[vecFwd.z, vecUp.z, vecCrs.z, vecTrns.z],[0.0, 0.0, 0.0, 1.0]] )
		angle = math.pi * 0.5
		matRotTransformation = om.MMatrix( [[1.0, 0.0, 0.0, 0.0],[0.0, math.cos(angle), -math.sin(angle), 0.0],[0.0, math.sin(angle), math.cos(angle), 0.0],[0.0, 0.0, 0.0, 1.0]] )
		matRotTransformation = om.MMatrix( [[math.cos(angle), 0.0, math.sin(angle), 0.0],[0.0, 1.0, 0.0, 0.0],[-math.sin(angle), 0.0, math.cos(angle), 0.0],[0.0, 0.0, 0.0, 1.0]] )
		ptTorus = om.MPoint( cmds.xform('pTorus1.vtx['+str(j+i*resArc)+']', q=True, worldSpace=True, t=True) )
		ptTK = matTransformation * matRotTransformation * ptTorus
		cmds.move( ptTK.x, ptTK.y, ptTK.z, 'pTorus1.vtx['+str(j+i*resArc)+']', absolute=True )
