import maya.cmds as cmds
import maya.OpenMaya as om
import maya.OpenMayaMPx as omMPx
import math

# various util functions for selection

# point-plane distance
def distPointPlane(point, planeND):
	planeN, planeD = planeND
	return planeN.x * point.x + planeN.y * point.y + planeN.z * point.z  - planeD

# get vertices that are in front of the provided planes
# additinally, the polygon center have to pass the test as well
def getVertsPlane( planes ):
	selection_list = om.MSelectionList()
	dag_path = om.MDagPath()
	component = om.MObject()
	om.MGlobal.getActiveSelectionList(selection_list)
	selection_list.getDagPath(0, dag_path, component)
	mItVtx = om.MItMeshVertex( dag_path )
	if not component.isNull():
		if (component.apiType() == om.MFn.kMeshVertComponent):
			mItVtx = om.MItMeshVertex( dag_path, component )
			if mItVtx.count() == 0:
				mItVtx = om.MItMeshVertex( dag_path )
	cmds.select( clear=True )
	compVtx = om.MObject()
	for iPlane in planes:
		vtxList = om.MIntArray()
		while not mItVtx.isDone():
			vtxId = mItVtx.index()
			point = mItVtx.position(om.MSpace.kWorld)
			faceList = om.MIntArray()
			mItVtx.getConnectedFaces(faceList)
			mFnCompFace = om.MFnSingleIndexedComponent()
			compFace = mFnCompFace.create(om.MFn.kMeshPolygonComponent)
			map(mFnCompFace.addElement, faceList)
			mItPoly = om.MItMeshPolygon( dag_path, compFace )
			mItVtx.next()
			dist = distPointPlane( point, iPlane )
			if (dist < 0):
				continue
			while not mItPoly.isDone():
				point = mItPoly.center()
				dist = distPointPlane( point, iPlane )
				if (dist > 0):
					vtxList.append(vtxId)
					break
				mItPoly.next()
		mFnCompVtx = om.MFnSingleIndexedComponent()
		compVtx = mFnCompVtx.create(om.MFn.kMeshVertComponent)
		map(mFnCompVtx.addElement, vtxList)
		mItVtx.reset( dag_path, compVtx )
	add_sel_vtx = om.MSelectionList()
	add_sel_vtx.add( dag_path, compVtx )
	om.MGlobal.setActiveSelectionList(add_sel_vtx)
	return vtxList

# get edges that are in front of the provided planes (edge centers)
def getEdgesPlane( planes ):
	selection_list = om.MSelectionList()
	dag_path = om.MDagPath()
	component = om.MObject()
	om.MGlobal.getActiveSelectionList(selection_list)
	selection_list.getDagPath(0, dag_path, component)
	mItEdge = om.MItMeshEdge( dag_path )
	if not component.isNull():
		print("not NULL")
		if (component.apiType() == om.MFn.kMeshEdgeComponent):
			mItEdge = om.MItMeshEdge( dag_path, component )
			print("OK")
	cmds.select( clear=True )
	compEdge = om.MObject()
	for iPlane in planes:
		edgeList = om.MIntArray()
		while not mItEdge.isDone():
			edgeId = mItEdge.index()
			point = mItEdge.center(om.MSpace.kWorld)
			mItEdge.next()
			dist = distPointPlane( point, iPlane )
			if (dist < 0):
				continue
			edgeList.append(edgeId)
		mFnCompEdge = om.MFnSingleIndexedComponent()
		compEdge = mFnCompEdge.create(om.MFn.kMeshEdgeComponent)
		map(mFnCompEdge.addElement, edgeList)
		mItEdge.reset( dag_path, compEdge )
	add_sel_edge = om.MSelectionList()
	add_sel_edge.add( dag_path, compEdge )
	om.MGlobal.setActiveSelectionList(add_sel_edge)
	return edgeList

# get vertices inside bounding box
def getVertsBB( xMin=-1.0, xMax=1.0, yMin=-1.0, yMax=1.0, zMin=-1.0, zMax=1.0 ):
	selection_list = om.MSelectionList()
	dag_path = om.MDagPath()
	component = om.MObject()
	om.MGlobal.getActiveSelectionList(selection_list)
	selection_list.getDagPath(0, dag_path, component)
	mItVtx = om.MItMeshVertex( dag_path )
	if not component.isNull():
		if (component.apiType() == om.MFn.kMeshVertComponent):
			mItVtx = om.MItMeshVertex( dag_path, component )
			if mItVtx.count() == 0:
				mItVtx = om.MItMeshVertex( dag_path )
	cmds.select( clear=True )
	compVtx = om.MObject()
	vtxList = om.MIntArray()
	while not mItVtx.isDone():
		vtxId = mItVtx.index()
		point = mItVtx.position(om.MSpace.kWorld)
		mItVtx.next()
		if (point.x < xMin):
			continue
		if (point.x > xMax):
			continue
		if (point.y < yMin):
			continue
		if (point.y > yMax):
			continue
		if (point.z < zMin):
			continue
		if (point.z > zMax):
			continue
		vtxList.append(vtxId)
	mFnCompVtx = om.MFnSingleIndexedComponent()
	compVtx = mFnCompVtx.create(om.MFn.kMeshVertComponent)
	map(mFnCompVtx.addElement, vtxList)
	add_sel_vtx = om.MSelectionList()
	add_sel_vtx.add( dag_path, compVtx )
	om.MGlobal.setActiveSelectionList(add_sel_vtx)
	return vtxList

# get vertices inside ellipsoid
def getVertsEllipsoid( xCtr=0.0, yCtr=0.0, zCtr=0.0, a=1.0, b=1.0, c=1.0 ):
	selection_list = om.MSelectionList()
	dag_path = om.MDagPath()
	component = om.MObject()
	om.MGlobal.getActiveSelectionList(selection_list)
	selection_list.getDagPath(0, dag_path, component)
	mItVtx = om.MItMeshVertex( dag_path )
	if not component.isNull():
		if (component.apiType() == om.MFn.kMeshVertComponent):
			mItVtx = om.MItMeshVertex( dag_path, component )
			if mItVtx.count() == 0:
				mItVtx = om.MItMeshVertex( dag_path )
	cmds.select( clear=True )
	compVtx = om.MObject()
	vtxList = om.MIntArray()
	while not mItVtx.isDone():
		vtxId = mItVtx.index()
		point = mItVtx.position(om.MSpace.kWorld)
		mItVtx.next()
		if ( math.pow((point.x-xCtr)/a,2) + math.pow((point.y-yCtr)/b,2) + math.pow((point.z-zCtr)/c,2) < 1.0 ):
			vtxList.append(vtxId)
	mFnCompVtx = om.MFnSingleIndexedComponent()
	compVtx = mFnCompVtx.create(om.MFn.kMeshVertComponent)
	map(mFnCompVtx.addElement, vtxList)
	add_sel_vtx = om.MSelectionList()
	add_sel_vtx.add( dag_path, compVtx )
	om.MGlobal.setActiveSelectionList(add_sel_vtx)
	return vtxList

def selectFromList( name, selList, element='vtx' ):
	cmds.select( clear=True )
	for i in selList:
		cmds.select(name+'.'+element+'['+str(i)+']', add=True)
