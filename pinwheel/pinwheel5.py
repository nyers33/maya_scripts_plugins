# include pinwheel_util
import maya.mel as mel

# geometry - pinwheel pentagon from 6 planes
rad=1.25
mainPlane = cmds.polyDisc( sides=5, subdivisionMode=0, subdivisions=4, r=rad )
cmds.rename( mainPlane, 'planeGEO1' )
side=2.0*rad*math.sqrt(5.0/8.0-math.sqrt(5.0)/8.0)
sidePlain = cmds.polyPlane( w=side, h=3.0, sx=16, sy=24, cuv=0, name="planeGEO2" )
cmds.setAttr( 'planeGEO2.translate', 0.0, 0.0, 1.5+rad*0.25*(1.0+math.sqrt(5.0)), type="double3" )
cmds.setAttr( 'planeGEO2.rotate', 0.0, 0.0, 0.0, type="double3" )
cmds.lattice( dv=(2, 2, 2), oc=True )
cmds.select( 'ffd1Lattice.pt[1][0:1][1]',r=True )
cmds.move( (math.sqrt(5.0-2.0*math.sqrt(5.0))*(3.0+rad*0.25*(1.0+math.sqrt(5.0)))-0.5*side),0,0,r=True )
cmds.select( 'ffd1Lattice.pt[0][0:1][1]',r=True )
cmds.move( -(math.sqrt(5.0-2.0*math.sqrt(5.0))*(3.0+rad*0.25*(1.0+math.sqrt(5.0)))-0.5*side),0,0,r=True )
cmds.delete( sidePlain, constructionHistory=True )
cmds.select( 'planeGEO2' )
cmds.move( 0, 0, 0, 'planeGEO2.scalePivot', 'planeGEO2.rotatePivot', absolute=True )
cmds.makeIdentity( apply=True, t=1, r=1, s=1, n=0 )
cmds.duplicate( 'planeGEO2', rr=True )
cmds.setAttr( 'planeGEO3.rotate', 0.0, 72.0, 0.0, type="double3" )
cmds.makeIdentity( apply=True, t=1, r=1, s=1, n=0 )
cmds.duplicate( 'planeGEO3', rr=True )
cmds.setAttr( 'planeGEO4.rotate', 0.0, 72.0, 0.0, type="double3" )
cmds.makeIdentity( apply=True, t=1, r=1, s=1, n=0 )
cmds.duplicate( 'planeGEO4', rr=True )
cmds.setAttr( 'planeGEO5.rotate', 0.0, 72.0, 0.0, type="double3" )
cmds.makeIdentity( apply=True, t=1, r=1, s=1, n=0 )
cmds.duplicate( 'planeGEO5', rr=True )
cmds.setAttr( 'planeGEO6.rotate', 0.0, 72.0, 0.0, type="double3" )
cmds.makeIdentity( apply=True, t=1, r=1, s=1, n=0 )
pinwheelGEO = cmds.polyUnite( 'planeGEO1', 'planeGEO2', 'planeGEO3', 'planeGEO4', 'planeGEO5', 'planeGEO6', n='pinwheelGEO1' )
cmds.delete( 'pinwheelGEO1', ch=1 )

# geometry - uv planar projection for pinwheel
cmds.polyUVSet( rename=True, newUVSet='uvSet' )
cmds.polyProjection( 'pinwheelGEO1.f[*]', type='Planar', md='y', ibd=True, ch=True )
geomH = 3.0*math.sqrt(5.0)+0.25*(5.0+math.sqrt(5.0))*rad
geomW = 0.25*math.sqrt(10.0-2.0*math.sqrt(5.0))*(12.0+rad+math.sqrt(5.0)*rad)
geomOffsetWH = (-3.0+math.sqrt(5.0)+2.0*math.sqrt(2.0*(5.0+math.sqrt(5.0))))/(4.0*math.sqrt(2.0*(5.0+math.sqrt(5.0))))
cmds.setAttr( 'polyPlanarProj1.projectionHeight', geomW*(100.0/90.0) )
cmds.setAttr( 'polyPlanarProj1.projectionWidth', geomW*(100.0/90.0) )
cmds.setAttr( 'polyPlanarProj1.imageCenterY', 0.5+(0.5-geomOffsetWH)*(90.0/100.0) )
cmds.delete( 'pinwheelGEO1', ch=1 )

# geometry - uv planar projection for pinwheel
cmds.polyUVSet( create=True, uvSet='uvSetWing' )
crossPlaneMid=[]
for iCRV in range(5):
	cmds.select( clear=True )
	cmds.select( 'pinwheelGEO1' )
	angle=2.0/5.0*math.pi
	a=math.sqrt(10.0-2.0*math.sqrt(5.0))
	b=(1.0+math.sqrt(5.0))
	foldPlaneL = [ om.MVector( (-math.cos(-iCRV*angle)), 0 , (-math.sin(-iCRV*angle)) ).normal(), -0.001 ]
	foldPlaneR = [ om.MVector( (math.cos(-(iCRV+1)*angle)), 0 , (math.sin(-(iCRV+1)*angle)) ).normal(), -0.001 ]
	crossPlane = [ om.MVector( 0.25*(-a*math.cos(-iCRV*angle)+b*math.sin(-iCRV*angle)), 0 , 0.25*(-a*math.sin(-iCRV*angle)-b*math.cos(-iCRV*angle)) ).normal(), 0.25*b*rad+0.001 ]
	crossPlaneMid.append( [ om.MVector( -0.25*(-a*math.cos(-iCRV*angle)+b*math.sin(-iCRV*angle)), 0 , -0.25*(-a*math.sin(-iCRV*angle)-b*math.cos(-iCRV*angle)) ).normal(), -0.25*b*rad-0.001 ] )
	getVertsPlane( [ crossPlane, foldPlaneL, foldPlaneR ] )
	faces = cmds.polyListComponentConversion( fv=True, tf=True )
	wingProj = cmds.polyProjection( faces[0], uvs='uvSetWing', type='Planar', md='b', ibd=True, ch=True, kir=False )
	cmds.setAttr( wingProj[0]+'.imageCenterX', 0.75 )
	cmds.setAttr( wingProj[0]+'.imageScaleU', 0.5 )
cmds.select( clear=True )
cmds.select( 'pinwheelGEO1' )
midPentagonVerts = getVertsPlane( crossPlaneMid )
midPentagonFaces = cmds.polyListComponentConversion( fv=True, tf=True )
wingProj = cmds.polyProjection( midPentagonFaces[0], uvs='uvSetWing', type='Planar', md='y', ibd=True, ch=True )
geomH = 0.25*(1.0+math.sqrt(5.0))*rad+rad
geomW = 2*math.sqrt(5.0/8.0+math.sqrt(5.0)/8.0)*rad
geomOffsetWH = (-3.0+math.sqrt(5.0)+2.0*math.sqrt(2.0*(5.0+math.sqrt(5.0))))/(4.0*math.sqrt(2.0*(5.0+math.sqrt(5.0))))
cmds.setAttr( wingProj[0]+'.imageCenterX', 0.25 )
cmds.setAttr( wingProj[0]+'.projectionHeight', geomW*(100.0/25.0) )
cmds.setAttr( wingProj[0]+'.projectionWidth', geomW*(100.0/25.0) )
cmds.setAttr( wingProj[0]+'.imageCenterY', 0.5+(0.5-geomOffsetWH)*(25.0/100.0) )
cmds.delete( 'pinwheelGEO1', ch=1 )

# geometry - uv planar projection for pinwheel
cmds.polyUVSet( create=True, uvSet='uvSetContour' )
cmds.polyUVSet( copy=True, nuv='uvSetContour', uvSet='uvSetWing' )
cmds.polyUVSet( currentUVSet=True,  uvSet='uvSetContour')
for iCRV in range(5):
	angle=2.0/5.0*math.pi
	a=math.sqrt(10.0-2.0*math.sqrt(5.0))
	b=(1.0+math.sqrt(5.0))
	foldPlaneL = [ om.MVector( (-math.cos(-iCRV*angle)), 0 , (-math.sin(-iCRV*angle)) ).normal(), -0.001 ]
	foldPlaneR = [ om.MVector( (math.cos(-(iCRV+1)*angle)), 0 , (math.sin(-(iCRV+1)*angle)) ).normal(), -0.001 ]
	crossPlaneN = om.MVector( 0.25*(-a*math.cos(-iCRV*angle)+b*math.sin(-iCRV*angle)), 0 , 0.25*(-a*math.sin(-iCRV*angle)-b*math.cos(-iCRV*angle)) ).normal()
	# outer line
	cmds.select( clear=True )
	cmds.select( 'pinwheelGEO1' )
	getEdgesPlane( [ [ crossPlaneN, (3.0+rad*0.25*(1.0+math.sqrt(5.0)))-0.001 ] ] )
	verts = cmds.polyListComponentConversion( fe=True, tv=True )
	cmds.select( verts )
	vertA = getVertsEllipsoid( xCtr=(3.0*math.sqrt(5.0)-3.0+rad)*math.sin(-iCRV*angle), yCtr=0.0, zCtr=-(3.0*math.sqrt(5.0)-3.0+rad)*math.cos(-iCRV*angle), a=0.001, b=0.001, c=0.001 )
	vertA = list(set(vertA) - set(midPentagonVerts))
	cmds.select( verts )
	vertB = getVertsEllipsoid( xCtr=(3.0*math.sqrt(5.0)-3.0+rad)*math.sin(-(iCRV+1)*angle), yCtr=0.0, zCtr=-(3.0*math.sqrt(5.0)-3.0+rad)*math.cos(-(iCRV+1)*angle), a=0.001, b=0.001, c=0.001 )
	vertB = list(set(vertB) - set(midPentagonVerts))
	# inner line
	cmds.select( clear=True )
	cmds.select( 'pinwheelGEO1' )
	getEdgesPlane( [ [ crossPlaneN, (rad*0.25*(1.0+math.sqrt(5.0)))-0.001 ], [ -crossPlaneN, -(rad*0.25*(1.0+math.sqrt(5.0)))-0.001 ], foldPlaneL, foldPlaneR ] )
	verts = cmds.polyListComponentConversion( fe=True, tv=True )
	cmds.select( verts )
	vertC = getVertsEllipsoid( xCtr=rad*math.sin(-(iCRV+1)*angle), yCtr=0.0, zCtr=-rad*math.cos(-(iCRV+1)*angle), a=0.001, b=0.001, c=0.001 )
	vertC = list(set(vertC) - set(midPentagonVerts))
	cmds.select( verts )
	vertD = getVertsEllipsoid( xCtr=rad*math.sin(-iCRV*angle), yCtr=0.0, zCtr=-rad*math.cos(-iCRV*angle), a=0.001, b=0.001, c=0.001 )
	vertD = list(set(vertD) - set(midPentagonVerts))
	cmds.select( clear=True )
	cmds.select( 'pinwheelGEO1' )
	getVertsPlane( [ [ crossPlaneN, 0.25*b*rad+0.001 ], foldPlaneL, foldPlaneR ] )
	faces = cmds.polyListComponentConversion( fv=True, tf=True )
	contourProj = cmds.polyContourProjection( faces[0], ch=1, m=0, udc=False )
	cmds.setAttr( contourProj[0]+'.cornerVertices', 4, 'vtx['+str(vertA[0])+']', 'vtx['+str(vertB[0])+']', 'vtx['+str(vertC[0])+']', 'vtx['+str(vertD[0])+']', type='componentList' )
	cmds.setAttr( contourProj[0]+'.userDefinedCorners', 1 )
	cmds.polyEditUV( pivotU=0.5, pivotV=0.5, scaleU=1.0, scaleV=0.5 )
	cmds.polyEditUV( uValue=0.0, vValue=0.25 )
cmds.select( clear=True )
cmds.select( midPentagonFaces[0] )
cmds.polyEditUV( uValue=0.25, vValue=-0.25 )
cmds.select( clear=True )

# texture - albedo for pinwheel (ramp cirlces)
nStripe=4
rColStripe = [1.0,1.0,0.0,1.0,0.0,1.0]
gColStripe = [0.0,1.0,1.0,1.0,0.0,1.0]
bColStripe = [0.0,1.0,0.0,1.0,1.0,1.0]
nCol=6
rampTexture = cmds.shadingNode( 'ramp', name='ramp_circles', asTexture=True )
place2dTexture = cmds.shadingNode( 'place2dTexture', name='p2dtext_circles', asUtility=True )
cmds.connectAttr( place2dTexture+'.outUV', rampTexture+'.uv' )
cmds.connectAttr( place2dTexture+'.outUvFilterSize', rampTexture+'.uvFilterSize' )
cmds.setAttr( rampTexture+'.type', 4 )
cmds.setAttr( rampTexture+'.interpolation', 0 )
for iStripe in range(nStripe):
	for iCol in range(nCol):
		cmds.setAttr( rampTexture+'.colorEntryList['+str(iStripe*nCol+iCol)+'].position', (iStripe*nCol+iCol)*(1.0/math.sqrt(2.0))*(95.0/100.0)/(nStripe*nCol) )
		cmds.setAttr( rampTexture+'.colorEntryList['+str(iStripe*nCol+iCol)+'].color', rColStripe[iCol], gColStripe[iCol], bColStripe[iCol], type='double3' )
cmds.setAttr( rampTexture+'.colorEntryList['+str(nStripe*nCol)+'].position', (1.0/math.sqrt(2.0))*(95.0/100.0) )
cmds.setAttr( rampTexture+'.colorEntryList['+str(nStripe*nCol)+'].color', rColStripe[-1], gColStripe[-1], bColStripe[-1], type='double3' )

# texture - albedo for pinwheel (checker stipes)
checkerTexture = cmds.shadingNode( 'checker', name='checker_stripes', asTexture=True )
place2dTexture = cmds.shadingNode( 'place2dTexture', name='p2dtext_stripes', asUtility=True )
cmds.connectAttr( place2dTexture+'.outUV', checkerTexture+'.uv' )
cmds.connectAttr( place2dTexture+'.outUvFilterSize', checkerTexture+'.uvFilterSize' )
cmds.setAttr( place2dTexture+'.translateFrameU', 0.5 )
cmds.setAttr( place2dTexture+'.wrapU', 0 )
cmds.setAttr( place2dTexture+'.wrapV', 0 )
cmds.setAttr( place2dTexture+'.repeatU', 0.5 )
cmds.setAttr( place2dTexture+'.repeatV', 20.5 )
cmds.setAttr( checkerTexture+'.defaultColor', 1.000, 1.000, 1.000, type='double3' )

# texture - albedo for pinwheel (ramp rainbow)
rampTexture = cmds.shadingNode( 'ramp', name='ramp_rainbow', asTexture=True )
place2dTexture = cmds.shadingNode( 'place2dTexture', name='p2dtext_rainbow', asUtility=True )
cmds.connectAttr( place2dTexture+'.outUV', rampTexture+'.uv' )
cmds.connectAttr( place2dTexture+'.outUvFilterSize', rampTexture+'.uvFilterSize' )
cmds.setAttr( rampTexture+'.type', 0 )
cmds.setAttr( rampTexture+'.interpolation', 4 )
cmds.setAttr( rampTexture+'.colorEntryList[0].position', 0.50 )
cmds.setAttr( rampTexture+'.colorEntryList[1].position', 0.75 )
cmds.setAttr( rampTexture+'.colorEntryList[2].position', 1.00 )
cmds.setAttr( rampTexture+'.colorEntryList[0].color', 1.000, 0.000, 0.000, type='double3' )
cmds.setAttr( rampTexture+'.colorEntryList[1].color', 1.000, 1.000, 1.000, type='double3' )
cmds.setAttr( rampTexture+'.colorEntryList[2].color', 0.000, 0.000, 1.000, type='double3' )

# render - lambert shader
shader = cmds.shadingNode( 'lambert', name='lambert_pinwheel', asShader=True )
cmds.connectAttr( checkerTexture+'.outColor', 'lambert_pinwheel.color', force=True ) 
shading_network = cmds.sets( renderable=True, noSurfaceShader=True, empty=True, name='lambert_pinwheel_SG' )
cmds.connectAttr( 'lambert_pinwheel.outColor', 'lambert_pinwheel_SG.surfaceShader' )
cmds.sets( 'pinwheelGEO1', e=True, forceElement='lambert_pinwheel_SG' )

# geometry - ground and squeeze plane 
cmds.polyPlane( w=10.0, h=10.0, sx=1, sy=1, name="groundPlaneGEO1" )
cmds.setAttr( 'groundPlaneGEO1.translate', 0.0, -0.05, 0.0, type="double3" )
cmds.polyPlane( w=10.0, h=10.0, sx=1, sy=1, name=" flattenPlaneGEO1" )
cmds.setAttr( 'flattenPlaneGEO1.translate', 0.0, 4.0, 0.0, type="double3" )
cmds.setAttr( 'flattenPlaneGEO1.rotate', 180.0, 0.0, 0.0, type="double3" )
cmds.setKeyframe( 'flattenPlaneGEO1', attribute='translateY', v=4.0, t=270 )
cmds.setKeyframe( 'flattenPlaneGEO1', attribute='translateY', v=0.8, t=340 )

groundPlaneCRV1 = cmds.curve( d=1, p=[(-5, -0.05, -5), (5, -0.05, -5), (5, -0.05, 5), (-5, -0.05, 5), (-5, -0.05, -5)], k=[0,1,2,3,4] )
cmds.rename( groundPlaneCRV1, 'groundPlaneCRV1' )
cmds.parent( 'groundPlaneCRV1', 'groundPlaneGEO1' )

flattenPlaneCRV1 = cmds.curve( d=1, p=[(-5, 4, -5), (5, 4, -5), (5, 4, 5), (-5, 4, 5), (-5, 4, -5)], k=[0,1,2,3,4] )
cmds.rename( flattenPlaneCRV1, 'flattenPlaneCRV1' )
cmds.parent( 'flattenPlaneCRV1', 'flattenPlaneGEO1' )

# geometry + animation - cylinder for flattening the mid section of the pinwheel
flattenCylinder = cmds.polyCylinder( n='flattenCylinderGEO1', r=0.55, h=2.0, sx=20, sy=4, sz=6 )
cmds.setAttr( flattenCylinder[0]+'.translate', 0.0, 2.5, 0.0, type="double3" )
cmds.setAttr( flattenCylinder[1]+'.roundCap', 1 )
cmds.setKeyframe( flattenCylinder, attribute='translateY', v=2.5, t=270 )
cmds.setKeyframe( flattenCylinder, attribute='translateY', v=1.7, t=340 )

# animation - folding path visualization
cmds.circle( nr=(1, 0, 0), c=(0, 0, 0), sw=180, r=0.5*(3.0+rad*0.25*(1.0+math.sqrt(5.0)))/(0.25*(1.0+math.sqrt(5.0))), n="foldPathCRV1" )
cmds.setAttr( 'foldPathCRV1.rotate', -90.0, 0.0, 0.0, type="double3" )
cmds.move( 0, 0, 0.5*(3.0+rad*0.25*(1.0+math.sqrt(5.0)))/(0.25*(1.0+math.sqrt(5.0))), 'foldPathCRV1.scalePivot', 'foldPathCRV1.rotatePivot', absolute=True )
cmds.setAttr( 'foldPathCRV1.translate', 0.0, 0.0, -0.5*(3.0+rad*0.25*(1.0+math.sqrt(5.0)))/(0.25*(1.0+math.sqrt(5.0))), type="double3" )
cmds.setAttr( 'foldPathCRV1.scaleZ', 0.66666 )
cmds.makeIdentity( apply=True, t=1, r=1, s=1, n=0 )
cmds.delete( constructionHistory=True )

cmds.duplicate( 'foldPathCRV1', rr=True )
cmds.setAttr( 'foldPathCRV2.rotateY', 72.0 )
cmds.makeIdentity( apply=True, t=1, r=1, s=1, n=0 )
cmds.duplicate( 'foldPathCRV2', rr=True )
cmds.setAttr( 'foldPathCRV3.rotateY', 72.0 )
cmds.makeIdentity( apply=True, t=1, r=1, s=1, n=0 )
cmds.duplicate( 'foldPathCRV3', rr=True )
cmds.setAttr( 'foldPathCRV4.rotateY', 72.0 )
cmds.makeIdentity( apply=True, t=1, r=1, s=1, n=0 )
cmds.duplicate( 'foldPathCRV4', rr=True )
cmds.setAttr( 'foldPathCRV5.rotateY', 72.0 )
cmds.makeIdentity( apply=True, t=1, r=1, s=1, n=0 )

sampleCRV = [0.0, 0.25, 0.5, 0.75, 1.0]
sampleTime = [30, 90, 150, 210, 270]
keyAttributes = ['translateX', 'translateY', 'translateZ', 'rotateX', 'rotateY', 'rotateZ']
crvInfoNode = cmds.pointOnCurve( 'foldPathCRV1', ch=True )
cmds.setAttr( crvInfoNode+'.turnOnPercentage', 1 )

# animation - folding keyframes
foldSize = 1.25
foldOffset = 0.25 * math.sqrt(2.0*foldSize*foldSize)
folderConstarintGuideGeoCollection = []
for iCRV in range(5):
	folderConstarintGuideGeoCollection.append( cmds.polyPlane( w=foldSize, h=foldSize, sx=1, sy=1, name="folderGEO1" ) )
	cmds.connectAttr( 'foldPathCRV'+str(iCRV+1)+'Shape'+'.worldSpace[0]', crvInfoNode+'.inputCurve', force=True )
	for iParam, iFrame in zip(sampleCRV, sampleTime): 
		cmds.setAttr( crvInfoNode+'.parameter', iParam )	
		keyValues = []
		for iNode, iAttr in zip([crvInfoNode], ['.position']):
			keyValues.append( cmds.getAttr( iNode+iAttr )[0][0] )
			keyValues.append( cmds.getAttr( iNode+iAttr )[0][1] )
			keyValues.append( cmds.getAttr( iNode+iAttr )[0][2] )
		# manually set rotation
		keyValues.append( 0 + iParam * 180 )
		keyValues.append( 0 + iCRV * 72 )
		keyValues.append( 0 )
		for iAttr, iVal in zip(keyAttributes, keyValues):
			cmds.setKeyframe( folderConstarintGuideGeoCollection[-1], attribute=iAttr, v=iVal, t=iFrame )
	cmds.setKeyframe( folderConstarintGuideGeoCollection[-1], attribute='translateX', v=foldOffset * math.sin(2.0/5.0*math.pi*iCRV) - 0.0 * foldOffset * math.cos(2.0/5.0*math.pi*iCRV), t=340 )
	cmds.setKeyframe( folderConstarintGuideGeoCollection[-1], attribute='translateY', v=0.05, t=340 )
	cmds.setKeyframe( folderConstarintGuideGeoCollection[-1], attribute='translateZ', v=foldOffset * math.cos(2.0/5.0*math.pi*iCRV) + 0.0 * foldOffset * math.sin(2.0/5.0*math.pi*iCRV), t=340 )
	cmds.setKeyframe( folderConstarintGuideGeoCollection[-1], attribute='rotateX', v=180, t=340 )

# geometry - stitching up the pinwheel
cmds.select( 'pinwheelGEO1' )
getVertsEllipsoid( xCtr=0.0, yCtr=0.0, zCtr=0.0, a=1.3, b=1.3, c=1.3 )
cmds.polyMergeVertex( d=0.01 )
cmds.select( clear=True )

# fx - nucleus & nCloth for pinwheel
nucleus = cmds.createNode( 'nucleus', name="nucleus1" )
ncloth = cmds.createNode( 'nCloth', name="nClothShape1" )
mesh = cmds.createNode( 'mesh', name="outputCloth1" )
cmds.connectAttr( 'pinwheelGEO1' + '.worldMesh[0]', ncloth + '.inputMesh' )
cmds.connectAttr( ncloth + '.outputMesh', mesh + '.inMesh' )
cmds.connectAttr( 'time1.outTime', nucleus + '.currentTime' )
cmds.connectAttr( 'time1.outTime', ncloth + '.currentTime' )
cmds.connectAttr( ncloth + '.currentState', nucleus + '.inputActive[0]' )
cmds.connectAttr( ncloth + '.startState', nucleus + '.inputActiveStart[0]' )
cmds.connectAttr( nucleus + '.outputObjects[0]', ncloth + '.nextState' )
cmds.connectAttr( nucleus + '.startFrame', ncloth + '.startFrame' )
cmds.sets(mesh, addElement='initialShadingGroup' )
	
# fx - colliders (makeCollideNCloth)
for iCol, iColName in enumerate(['groundPlaneGEO1', 'flattenPlaneGEO1', 'flattenCylinderGEO1'], 0):
	cmds.createNode( 'transform', name="nRigid"+str(iCol+1) )
	collider = cmds.createNode( 'nRigid', name="nRigidShape"+str(iCol+1), p="nRigid"+str(iCol+1) )
	cmds.connectAttr( iColName + '.worldMesh[0]', collider + '.inputMesh' )
	cmds.connectAttr( 'time1.outTime', collider + '.currentTime' )
	cmds.connectAttr( nucleus + '.startFrame', collider + '.startFrame' )
	cmds.connectAttr( collider + '.currentState', nucleus + '.inputPassive['+str(iCol)+']' )
	cmds.connectAttr( collider + '.startState', nucleus + '.inputPassiveStart['+str(iCol)+']' )
	cmds.setAttr( collider + '.thickness', 0.042 )

# fx - constraint mid section (createNConstraint)
cmds.select( clear=True )
cmds.select( 'pinwheelGEO1' )
getVertsEllipsoid( xCtr=0.0, yCtr=0.0, zCtr=0.0, a=1.1, b=1.1, c=1.1 )
mel.eval( 'createNConstraint transform 0' )

# fx - constraint tips (createNConstraint)
for iCRV in range(5):
	cmds.select( clear=True )
	cmds.select( 'pinwheelGEO1' )
	foldPlane = [ om.MVector( (-math.cos(-iCRV*2.0/5.0*math.pi)), 0 , (-math.sin(-iCRV*2.0/5.0*math.pi)) ).normal(), -0.001 ]
	crossPlane = [ om.MVector( (math.sin(-iCRV*2.0/5.0*math.pi)), 0 , (-math.cos(-iCRV*2.0/5.0*math.pi)) ).normal(), (3.0+rad*0.25*(1.0+math.sqrt(5.0)))/(0.25*(1.0+math.sqrt(5.0))) - foldSize/2.0 ]
	getVertsPlane( [ crossPlane, foldPlane ] )
	cmds.select( 'folderGEO'+str(iCRV+1), tgl=True )
	mel.eval( 'createNConstraint transform 0' )

# fx - nucleus & clotch properties
cmds.select( clear=True )
cmds.setAttr( 'nucleus1.subSteps', 64 )
cmds.setAttr( 'nucleus1.maxCollisionIterations', 16 )
cmds.setAttr( 'nucleus1.gravity', 0.98 )
cmds.setAttr( 'nClothShape1.selfCollide', 0 )
cmds.setAttr( 'nClothShape1.stretchResistance', 256 )
cmds.setAttr( 'nClothShape1.compressionResistance', 0 )
cmds.setAttr( 'nClothShape1.bendResistance', 256 )
cmds.setAttr( 'nClothShape1.bendAngleDropoff', 0.2 )
cmds.setAttr( 'nClothShape1.shearResistance', 16 )
cmds.setAttr( 'nClothShape1.restitutionAngle', 4 )
cmds.setAttr( 'nClothShape1.restitutionTension', 1024 )
cmds.setAttr( 'nClothShape1.pointMass', 0.024 )
cmds.setAttr( 'nClothShape1.thickness', 0.03 )
cmds.setAttr( 'nClothShape1.selfCollideWidthScale', 3 )

# uv - copy uv data to nCloth mesh
cmds.transferAttributes( 'pinwheelGEO1', 'polySurface1', transferPositions=0, transferNormals=0, transferUVs=2, transferColors=0, sampleSpace=4, searchMethod=3, flipUVs=0, colorBorders=1 )

# camera
cmds.setAttr( 'topShape.backgroundColor', 0.5, 0.5, 0.5, type='double3' )

# animation - playback
cmds.playbackOptions( loop='once' )
cmds.playbackOptions( playbackSpeed=0, maxPlaybackSpeed=0 )
cmds.playbackOptions( maxTime=400 )
