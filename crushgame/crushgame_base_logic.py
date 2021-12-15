import sys

import PySide2.QtCore as QtCore
import PySide2.QtGui as QtGui
import PySide2.QtWidgets as QtWidgets
import maya.OpenMayaUI as OpenMayaUI  

try:
    from shiboken2 import wrapInstance
except:
    from shiboken import wrapInstance

import maya.cmds as cmds

# This is a function that is run from your class object to get a handle
# to the main Maya window, it uses a combination of the Maya API as well as the SIP module
def getPyQtMayaWindow():  
    # Get the maya main window as a QMainWindow instance
    accessMainWindow = OpenMayaUI.MQtUtil.mainWindow()
    if sys.version_info.major >=3:
        return wrapInstance(int(accessMainWindow), QtWidgets.QWidget)
    else:
        return wrapInstance(long(accessMainWindow), QtWidgets.QWidget)

import functools
import random
import re

class MayaCrushGame:
    def __init__(self, nX=4, nY=6, nShape=4):
        
        self.cellSize = 7
        
        self.createAnim = True
        self.swapTime = 240
        self.dropTime = 60
        self.delayTime = 100
        self.currTime = cmds.currentTime( query=True )
        
        self.nX = nX
        self.nY = nY
        self.nShape = nShape
        
        self.shapes = []
        self.shaders = []
        self.shading_networks = []
        
        self.frames = []
        
        self.items = []
        
        self.nMatch = 0
        self.hMatch = [0] * self.nX * self.nY
        self.vMatch = [0] * self.nX * self.nY
        
        self.dropTable = [0] * self.nX * self.nY * 2
        
        self.streak = 0
        self.lastSwaped = []
        self.dumped_items = []
        
        cmds.select( clear=True )
        
    def set_params(self, nX, nY, nShape):
        self.nX = nX
        self.nY = nY
        self.nShape = nShape
        
    def setup_game(self, nX=4, nY=6, nShape=4):
        print('Game creation started!')
        
        self.set_params(nX, nY, nShape)
        
        self.shapes = []
        self.shaders = []
        self.shading_networks = []
        self.create_master_items()
        
        self.frames = []
        self.create_frames()
        
        self.items = [None] * self.nX * self.nY * 2
        self.populate_grid()
                
        self.hMatch = [0] * self.nX * self.nY
        self.vMatch = [0] * self.nX * self.nY
        
        self.dropTable = [0] * self.nX * self.nY * 2
        
        self.lastSwaped = []
        self.dumped_items = []
        
        if (self.nX < 3 or self.nY < 3):
            return
        
        print('Distributing items without matches!')
        while True:
            self.match_items()
            nMatch = self.count_matches()
            print(nMatch)
            self.print_matches()
            if nMatch == 0:
                break
            else:
                self.destroy_matches()
                self.create_dropTable()
                print('dropTable')
                self.print_dropTable()
                self.drop_items()
                self.populate_grid()
        
        if self.createAnim:
            cmds.cutKey( self.items, time=(1, self.currTime-1), attribute='translateX' )
            cmds.cutKey( self.items, time=(1, self.currTime-1), attribute='translateY' )
            cmds.cutKey( self.items, time=(1, self.currTime-1), attribute='visibility' )
            cmds.keyframe( self.items, time=(self.currTime,self.currTime),absolute=True, timeChange=1)
            
            for i_item in self.items:
                if i_item != None:
                    cmds.selectKey( i_item, time=(self.currTime,self.currTime), attribute='visibility' )
                    cmds.cutKey( animation='keys', clear=True)
                    cmds.showHidden( i_item )
            
            self.currTime = 1
            cmds.currentTime( 1 )
    
    def say_hello(self):
        print('Hello, World from Game!')
    
    def create_master_items(self):
        
        shader = cmds.shadingNode( 'lambert', name='lamert_red', asShader=True )
        cmds.setAttr( 'lamert_red.color',  1.0, 0.0, 0.0, type='double3' )
        shading_network = cmds.sets( renderable=True, noSurfaceShader=True, empty=True, name='lambert_red_SG' )
        cmds.connectAttr( 'lamert_red.outColor', 'lambert_red_SG.surfaceShader' )
        self.shaders.append(shader)
        self.shading_networks.append(shading_network)
		
        item_cube = cmds.polyCube(w=4, h=4, d=2, ax=(0,1,0), sx=1, sy=1, sz=1, name="itemCube")
        cmds.makeIdentity( apply=True, t=1, r=1, s=1, n=0 )
        cmds.delete( constructionHistory=True )
        cmds.sets( 'itemCube', e=True, forceElement='lambert_red_SG' )
        cmds.hide( 'itemCube' )
        self.shapes.append(item_cube[0])
        
        shader = cmds.shadingNode( 'lambert', name='lambert_blue', asShader=True )
        cmds.setAttr( 'lambert_blue.color',  0.0, 0.0, 1.0, type='double3' )
        shading_network = cmds.sets( renderable=True, noSurfaceShader=True, empty=True, name='lambert_blue_SG' )
        cmds.connectAttr( 'lambert_blue.outColor', 'lambert_blue_SG.surfaceShader' )
        self.shaders.append(shader)
        self.shading_networks.append(shading_network)
        
        item_cylinder = cmds.polyCylinder(h=3, r=1, ax=(0,1,0), sx=20, sy=1, sz=4, sc=True, rcp=1, name="itemCylinder")
        cmds.setAttr("{0}.rotate".format(item_cylinder[0]), 0, 0, -45)
        cmds.makeIdentity( apply=True, t=1, r=1, s=1, n=0 )
        cmds.delete( constructionHistory=True )
        cmds.sets( 'itemCylinder', e=True, forceElement='lambert_blue_SG' )
        cmds.hide( 'itemCylinder' )
        self.shapes.append(item_cylinder[0])
        
        shader = cmds.shadingNode( 'lambert', name='lambert_green', asShader=True )
        cmds.setAttr( 'lambert_green.color',  0.0, 1.0, 0.0, type='double3' )
        shading_network = cmds.sets( renderable=True, noSurfaceShader=True, empty=True, name='lambert_green_SG' )
        cmds.connectAttr( 'lambert_green.outColor', 'lambert_green_SG.surfaceShader' )
        self.shaders.append(shader)
        self.shading_networks.append(shading_network)
        
        item_sphere = cmds.polySphere(r=2, ax=(0,1,0), sx=16, sy=16, name="itemSphere")
        cmds.makeIdentity( apply=True, t=1, r=1, s=1, n=0 )
        cmds.delete( constructionHistory=True )
        cmds.sets( 'itemSphere', e=True, forceElement='lambert_green_SG' )
        cmds.hide( 'itemSphere' )
        self.shapes.append(item_sphere[0])
        
        shader = cmds.shadingNode( 'lambert', name='lambert_yellow', asShader=True )
        cmds.setAttr( 'lambert_yellow.color',  1.0, 1.0, 0.0, type='double3' )
        shading_network = cmds.sets( renderable=True, noSurfaceShader=True, empty=True, name='lambert_yellow_SG' )
        cmds.connectAttr( 'lambert_yellow.outColor', 'lambert_yellow_SG.surfaceShader' )
        self.shaders.append(shader)
        self.shading_networks.append(shading_network)
		
        item_torus = cmds.polyTorus(r=1.75, sr=0.75, ax=(0,1,0), sx=16, sy=16, name="itemTorus")
        cmds.setAttr("{0}.rotate".format(item_torus[0]), 45, 0, 45)
        cmds.makeIdentity( apply=True, t=1, r=1, s=1, n=0 )
        cmds.delete( constructionHistory=True )
        cmds.sets( 'itemTorus', e=True, forceElement='lambert_yellow_SG' )
        cmds.hide( 'itemTorus' )
        self.shapes.append(item_torus[0])
        
        shader = cmds.shadingNode( 'lambert', name='lambert_white', asShader=True )
        cmds.setAttr( 'lambert_white.color',  1.0, 1.0, 1.0, type='double3' )
        shading_network = cmds.sets( renderable=True, noSurfaceShader=True, empty=True, name='lambert_white_SG' )
        cmds.connectAttr( 'lambert_white.outColor', 'lambert_white_SG.surfaceShader' )
        self.shaders.append(shader)
        self.shading_networks.append(shading_network)
        
        cmds.polyCone( r=2, h=2, sx=4, sy=1, sz=0, ax=(0,1,0), name="itemOctahedronTop")
        cmds.delete( 'itemOctahedronTop.f[0]' )
        cmds.move( 1.0, y=True )
        cmds.polyCone( r=2, h=2, sx=4, sy=1, sz=0, ax=(0,-1,0), name="itemOctahedronBottom")
        cmds.delete( 'itemOctahedronBottom.f[0]' )
        cmds.move( -1.0, y=True )
        cmds.polyUnite( 'itemOctahedronTop', 'itemOctahedronBottom', name='itemOctahedron' )
        cmds.polyMergeVertex( d=0.01, am=1, ch=1, name='itemOctahedron' )
        cmds.makeIdentity( apply=True, t=1, r=1, s=1, n=0 )
        cmds.delete( constructionHistory=True )
        cmds.sets( 'itemOctahedron', e=True, forceElement='lambert_white_SG' )
        cmds.hide( 'itemOctahedron' )
        self.shapes.append('itemOctahedron')
    
    def create_frames(self):
        for x in range(0, self.nX+1):
            cmds.polyPlane(w=self.cellSize*0.5, h=self.cellSize*self.nY, sx=1, sy=1, name="frame_0")
            selected = cmds.ls(selection=True)
            cmds.polyExtrudeFacet(selected[0], kft=True, thickness=0.2)
            cmds.setAttr("{0}.rotate".format(selected[0]), 0, 90, 90)
            cmds.setAttr("{0}.translate".format(selected[0]), self.cellSize*x, self.cellSize*0.5*self.nY, 0)
            self.frames.append(selected[0])
        for y in range(0, self.nY+1):
            cmds.polyPlane(w=self.cellSize*0.5, h=self.cellSize*self.nX, sx=1, sy=1, name="frame_0")
            selected = cmds.ls(selection=True)
            cmds.polyExtrudeFacet(selected[0], kft=True, thickness=0.1)
            cmds.setAttr("{0}.rotate".format(selected[0]), 0, 90, 0)
            cmds.setAttr("{0}.translate".format(selected[0]), self.cellSize*0.5*self.nX, self.cellSize*y, 0)
            self.frames.append(selected[0])
    
    def instance_item(self,tx,ty,tz):
        randID = random.randint(1, self.nShape)
        if randID == 1:
            instanced_item = cmds.instance(self.shapes[0])
        elif randID == 2:
            instanced_item = cmds.instance(self.shapes[1])
        elif randID == 3:
            instanced_item = cmds.instance(self.shapes[2])
        elif randID == 4:
            instanced_item = cmds.instance(self.shapes[3])
        elif randID == 5:
            instanced_item = cmds.instance(self.shapes[4])
    
        cmds.showHidden( instanced_item[0] )
        cmds.setAttr("{0}.translate".format(instanced_item[0]), self.cellSize*0.5+tx, self.cellSize*0.5+ty, tz)
        
        return instanced_item[0]
    
    def duplicate_item(self,tx,ty,tz):
        randID = random.randint(1, self.nShape)
        if randID == 1:
            duplicated_item = cmds.duplicate(self.shapes[0])
        elif randID == 2:
            duplicated_item = cmds.duplicate(self.shapes[1])
        elif randID == 3:
            duplicated_item = cmds.duplicate(self.shapes[2])
        elif randID == 4:
            duplicated_item = cmds.duplicate(self.shapes[3])
        elif randID == 5:
            duplicated_item = cmds.duplicate(self.shapes[4])
        
        cmds.showHidden( duplicated_item[0] )
        cmds.setAttr("{0}.translate".format(duplicated_item[0]), self.cellSize*0.5+tx, self.cellSize*0.5+ty, tz)
        
        return duplicated_item[0]
    
    def populate_grid(self):
        for y in range(0, self.nY*2):
            for x in range(0, self.nX):
                if self.items[x + y * self.nX] == None:
                    self.items[x + y * self.nX] = self.duplicate_item(self.cellSize*x,self.cellSize*y,0)
                    if self.createAnim:
                        self.setKeyFrame(self.items[x + y * self.nX], 'translateX', self.currTime)
                        self.setKeyFrame(self.items[x + y * self.nX], 'translateY', self.currTime)
                        self.setKeyFrame(self.items[x + y * self.nX], 'visibility', 1, 0)
                        if y >= self.nY:
                            self.setKeyFrame(self.items[x + y * self.nX], 'visibility', self.currTime, 0)
                        else:
                            self.setKeyFrame(self.items[x + y * self.nX], 'visibility', self.currTime, 1)
    
    def swap_items(self, selected=None):
        if (self.createAnim and self.delayTime > 0):
            self.delay_game()
        
        if (selected == None or type(selected) is not list):
            selected = cmds.ls(selection=True)
        if len(selected) != 2:
            print('Select exactly 2 items!')
        else:
            for i_sel in selected:
                if not(i_sel.startswith("item")):
                    print('Select items only!')
                    return None
            print(selected)
            
            # retrieve positions in list
            aID = self.items.index(selected[0])
            bID = self.items.index(selected[1])
            
            aIDX = aID % self.nX
            aIDY = aID / self.nX
            bID_candidate = []
            
            if aIDX != 0:
                bID_candidate.append(aID-1)
            if aIDX != self.nX-1:
                bID_candidate.append(aID+1)
            if aIDY != 0:
                bID_candidate.append(aID-self.nX)
            if aIDY != self.nY-1:
                bID_candidate.append(aID+self.nX)
            if bID not in bID_candidate:
                print('Select items next to each other!')
                cmds.select( clear=True )
                return None
            
            self.lastSwaped = selected  
            
            # change positions in list
            self.items[bID], self.items[aID] = self.items[aID], self.items[bID]
            
            if self.createAnim:
                # set swap start key for moving objects
                print(aID)
                print(bID)
                if abs(bID - aID) == 1:
                    print('swap in x direction')
                    self.setKeyFrame( selected[0], 'translateX', self.currTime )
                    self.setKeyFrame( selected[1], 'translateX', self.currTime )
                    self.setKeyFrame( selected[0], 'translateY', self.currTime )
                    self.setKeyFrame( selected[1], 'translateY', self.currTime )
                else:
                    print('swap in y direction')
                    self.setKeyFrame( selected[0], 'translateX', self.currTime )
                    self.setKeyFrame( selected[1], 'translateX', self.currTime )
                    self.setKeyFrame( selected[0], 'translateY', self.currTime )
                    self.setKeyFrame( selected[1], 'translateY', self.currTime )
                self.setKeyFrame( selected[0], 'visibility', self.currTime )
                self.setKeyFrame( selected[1], 'visibility', self.currTime )
                
                # set swap start key for standing objects
                for y in range(0, 2*self.nY):
                    for x in range(0, self.nX):
                        if (aID != x + y * self.nX) and (bID != x + y * self.nX):
                            self.setKeyFrame( self.items[x + y * self.nX], 'translateX', self.currTime )
                            self.setKeyFrame( self.items[x + y * self.nX], 'translateY', self.currTime )
                            self.setKeyFrame( self.items[x + y * self.nX], 'visibility', self.currTime )
            
            # set transformation node accordingly
            aTranslate = cmds.getAttr("{0}.translate".format(selected[0]), time=self.currTime, type=False)
            bTranslate = cmds.getAttr("{0}.translate".format(selected[1]), time=self.currTime, type=False)
            cmds.setAttr("{0}.translate".format(selected[0]), type='double3', *(bTranslate[0]))
            cmds.setAttr("{0}.translate".format(selected[1]), type='double3', *(aTranslate[0]))
            
            if self.createAnim:
                # set swap end key for moving objects
                self.currTime += self.swapTime
                cmds.playbackOptions( maxTime=self.currTime )
                if abs(bID - aID) == 1:
                    self.setKeyFrame( selected[0], 'translateX', self.currTime, keyValue=None )
                    self.setKeyFrame( selected[1], 'translateX', self.currTime, keyValue=None )
                else:
                    self.setKeyFrame( selected[0], 'translateY', self.currTime, keyValue=None )
                    self.setKeyFrame( selected[1], 'translateY', self.currTime, keyValue=None )
                cmds.currentTime( self.currTime )
                
                # set swap end key for standing objects
                for y in range(0, 2*self.nY):
                    for x in range(0, self.nX):
                        if (aID != x + y * self.nX) and (bID != x + y * self.nX):
                            self.setKeyFrame( self.items[x + y * self.nX], 'translateX', self.currTime, keyValue=None )
                            self.setKeyFrame( self.items[x + y * self.nX], 'translateY', self.currTime, keyValue=None )
                            self.setKeyFrame( self.items[x + y * self.nX], 'visibility', self.currTime )
                            
        self.streak = 0
        
        if (self.createAnim and self.delayTime > 0):
            self.delay_game()
    
    def match_items(self):
        # search for matches in horizontal direction
        for y in range(0, self.nY):
            for x in range(0, self.nX):
                i_type = self.items[x + y * self.nX][4:].strip("0123456789")
                
                if x == 0:
                    self.hMatch[x + y * self.nX] = 1
                else:
                    i_type_comp = self.items[x - 1 + y * self.nX][4:].strip("0123456789")
                    if i_type == i_type_comp:
                        self.hMatch[x + y * self.nX] = self.hMatch[x - 1 + y * self.nX]
                        continue
                    else:
                        self.hMatch[x + y * self.nX] = 1
                
                for x_comp in range(x + 1, self.nX):
                    i_type_comp = self.items[x_comp + y * self.nX][4:].strip("0123456789")
                    if i_type == i_type_comp:
                        self.hMatch[x + y * self.nX] += 1
                    else:
                        break
        
        # search for matches in vertical direction
        for x in range(0, self.nX):
            for y in range(0, self.nY):
                i_type = self.items[x + y * self.nX][4:].strip("0123456789")
                
                if y == 0:
                    self.vMatch[x + y * self.nX] = 1
                else:
                    i_type_comp = self.items[x + (y - 1) * self.nX][4:].strip("0123456789")
                    if i_type == i_type_comp:
                        self.vMatch[x + y * self.nX] = self.vMatch[x + (y - 1) * self.nX]
                        continue
                    else:
                        self.vMatch[x + y * self.nX] = 1
                
                for y_comp in range(y + 1, self.nY):
                    i_type_comp = self.items[x + y_comp * self.nX][4:].strip("0123456789")
                    if i_type == i_type_comp:
                        self.vMatch[x + y * self.nX] += 1
                    else:
                        break
    
    def count_matches(self):
        hMatch = len( [elem for elem in self.hMatch if elem > 2] )
        vMatch = len( [elem for elem in self.vMatch if elem > 2] )
        
        return hMatch + vMatch
    
    def print_matches(self):
        for y in reversed(range(0, self.nY)):
            hPrint = self.hMatch[(y * self.nX):(y * self.nX + self.nX)]
            for idx, val in enumerate(hPrint):
                if val < 3:
                    hPrint[idx] = 0
            vPrint = self.vMatch[(y * self.nX):(y * self.nX + self.nX)]
            for idx, val in enumerate(vPrint):
                if val < 3:
                    vPrint[idx] = 0
            print('|' + '|'.join(map(str,[x + y for x, y in zip(hPrint, vPrint)])) + '|')
    
    def destroy_matches(self):
        for y in range(0, self.nY):
            for x in range(0, self.nX):
                if (self.vMatch[x + y * self.nX]>2 or self.hMatch[x + y * self.nX]>2):
                    cmds.delete( self.items[x + y * self.nX] )
                    self.items[x + y * self.nX] = None
    
    def dump_matches(self):
        for y in range(0, self.nY):
            for x in range(0, self.nX):
                if (self.vMatch[x + y * self.nX]>2 or self.hMatch[x + y * self.nX]>2):
                    self.dumped_items.append( self.items[x + y * self.nX] )
                    self.setKeyFrame( self.items[x + y * self.nX], 'visibility', self.currTime, 0)
                    self.items[x + y * self.nX] = None
    
    def create_dropTable(self):
        for x in range(0, self.nX):
            self.dropTable[x + 0 * self.nX] = 0
        
        for x in range(0, self.nX):
            for y in range(1, self.nY*2):
                self.dropTable[x + y * self.nX] = self.dropTable[x + (y - 1) * self.nX]
                if self.items[x + (y - 1) * self.nX] == None:
                    self.dropTable[x + y * self.nX] += 1
        
        for y in range(0, self.nY):
            for x in range(0, self.nX):
                if self.items[x + y * self.nX] == None:
                    self.dropTable[x + y * self.nX] = 0
    
    def print_dropTable(self):
        for y in reversed(range(0, self.nY*2)):
            print('|' + '|'.join(map(str,self.dropTable[(y * self.nX):(y * self.nX + self.nX)])) + '|')
    
    def drop_items(self):
        if (self.createAnim and self.delayTime > 0):
            self.delay_game()
        
        for y in range(0, self.nY*2):
            for x in range(0, self.nX):
                item = self.items[x + y * self.nX]
                if self.dropTable[x + y * self.nX] != 0:
                    # change position in list
                    self.items[x + (y - self.dropTable[x + y * self.nX]) * self.nX] = self.items[x + y * self.nX]
                    self.items[x + y * self.nX] = None
                    
                    if self.createAnim:
                        # set drop start key for moving objects
                        self.setKeyFrame( item, 'translateX', self.currTime )
                        self.setKeyFrame( item, 'translateY', self.currTime )
                        if y < self.nY:
                            self.setKeyFrame( item, 'visibility', self.currTime, 1)
                        elif y < self.nY + self.dropTable[x + y * self.nX]:
                            self.setKeyFrame( item, 'visibility', self.currTime, 1)
                        else:
                            self.setKeyFrame( item, 'visibility', self.currTime, 0)
                    
                    # set transformation node accordingly
                    cmds.move(-(self.dropTable[x + y * self.nX]*self.cellSize), item, moveY=True, worldSpace=True, relative=True)
                    
                    if self.createAnim:
                        # set drop end key for moving objects
                        self.setKeyFrame( item, 'translateX', self.currTime + self.dropTime * self.dropTable[x + y * self.nX] )
                        self.setKeyFrame( item, 'translateY', self.currTime + self.dropTime * self.dropTable[x + y * self.nX] )
                        if y < self.nY:
                            self.setKeyFrame( item, 'visibility', self.currTime + self.dropTime * self.dropTable[x + y * self.nX], 1)
                        elif y < self.nY + self.dropTable[x + y * self.nX]:
                            self.setKeyFrame( item, 'visibility', self.currTime + self.dropTime * self.dropTable[x + y * self.nX], 1)
                        else:
                            self.setKeyFrame( item, 'visibility', self.currTime + self.dropTime * self.dropTable[x + y * self.nX], 0)
                        
                elif item != None:
                    if self.createAnim:
                        # set drop start key for standing objects
                        self.setKeyFrame( item, 'translateX', self.currTime )
                        self.setKeyFrame( item, 'translateY', self.currTime )
                        if y < self.nY:
                            self.setKeyFrame( item, 'visibility', self.currTime, 1)
                        else:
                            self.setKeyFrame( item, 'visibility', self.currTime, 0)
                    
        if self.createAnim:
            self.currTime += self.dropTime * max(self.dropTable)
            cmds.playbackOptions( maxTime=self.currTime )
            cmds.currentTime( self.currTime )
            cmds.select( clear=True )
        
        for y in range(0, self.nY*2):
            for x in range(0, self.nX):
                item = self.items[x + y * self.nX]
                if (item != None and self.createAnim):
                    # set drop end final key
                    self.setKeyFrame( item, 'translateX', self.currTime )
                    self.setKeyFrame( item, 'translateY', self.currTime )
                    if y < self.nY:
                        self.setKeyFrame( item, 'visibility', self.currTime, 1)
                    else:
                        self.setKeyFrame( item, 'visibility', self.currTime, 0)
        
        if (self.createAnim and self.delayTime > 0):
            self.delay_game()
    
    def update_game_single(self):
        self.match_items()
        self.print_matches()
        print(self.count_matches())
        self.destroy_matches()
        self.create_dropTable()
        self.drop_items()
        self.populate_grid()
        
    def update_game_recursive(self):
        while True:
            self.match_items()
            nMatch = self.count_matches()
            print(nMatch)
            self.print_matches()
            if nMatch == 0:
                if self.streak == 0:
                    self.swap_items()
                break
            else:
                if self.createAnim:
                    self.dump_matches()
                else:
                    self.destroy_matches()
                self.create_dropTable()
                self.drop_items()
                self.populate_grid()
                self.streak += 1
    
    def setKeyFrame(self, item, targetAttribute, keyTime, keyValue=None):
        # set key
        if keyValue == None:
            cmds.setKeyframe( item, attribute=targetAttribute, time=keyTime )
        else:
            cmds.setKeyframe( item, attribute=targetAttribute, time=keyTime, value=keyValue )
        
        if (targetAttribute == 'translateX' or targetAttribute == 'translateY'):
            cmds.selectKey( item, time=(keyTime, keyTime), attribute=targetAttribute )
            cmds.keyTangent( inTangentType='linear', outTangentType='linear' )
            cmds.selectKey( clear=True )
            
    def setKeyFrameSE(self, item, targetAttribute, startTime, endTime, startValue=None, endValue=None):
        # overwrite any existing animations
        cmds.cutKey( item, time=(startTime, endTime), attribute=targetAttribute )
        
        # set keys
        self.setKeyFrame( self, item, targetAttribute, startTime, startValue )
        self.setKeyFrame( self, item, targetAttribute, endTime, endValue )
        
        # set tangent type
        cmds.selectKey( item, time=(startTime, endTime), attribute=targetAttribute, keyframe=True )
        cmds.keyTangent( inTangentType='linear', outTangentType='linear' )
    
    def delay_game(self):
        for i_item in self.items:
            if i_item == None:
                continue
            self.setKeyFrame( i_item, 'translateX', self.currTime )
            self.setKeyFrame( i_item, 'translateY', self.currTime )
            self.setKeyFrame( i_item, 'visibility', self.currTime )
        
        self.currTime += self.delayTime
        for i_item in self.items:
            if i_item == None:
                continue
            self.setKeyFrame( i_item, 'translateX', self.currTime )
            self.setKeyFrame( i_item, 'translateY', self.currTime )
            self.setKeyFrame( i_item, 'visibility', self.currTime )

    def clear_game(self):
        for i_shape in self.shapes:
            cmds.delete( i_shape )
        self.shapes = []
        
        for i_frame in self.frames:
            cmds.delete( i_frame )
        self.frames = []
        
        for i_item in self.items:
            if i_item != None:
                cmds.delete( i_item )
        self.items = []
        
        for i_item in self.dumped_items:
            if i_item != None:
                cmds.delete( i_item )
        self.dumped_items = []
        
        for i_SG in self.shading_networks:
            cmds.delete( i_SG )
        self.shading_networks = []
        
        for i_shader in self.shaders:
            cmds.delete( i_shader )
        self.shaders = []
        
        self.hMatch = [0] * self.nX * self.nY
        self.vMatch = [0] * self.nX * self.nY
        self.dropTable = [0] * self.nX * self.nY * 2

# This class object is what creates the window, here you can specify what you would like to 
# go into your custom UI
class GameDialog(QtWidgets.QDialog):  
    # My custom window, which i want to parent to the maya main window 
    def __init__(self, parent=getPyQtMayaWindow()):  
        # Init my main window, and pass in the maya main window as it's parent  
        QtWidgets.QDialog.__init__(self, parent)
        
        # Window title
        self.setWindowTitle("MayaCrushGame")   
        
        # This is where I can start adding my PyQt widgets and controls to build up my GUI
        self.nXLabel = QtWidgets.QLabel("nX", parent=self)
        self.nXSB = QtWidgets.QSpinBox(parent=self)
        self.nXSB.setValue(5)
        self.nYLabel = QtWidgets.QLabel("nY", parent=self)
        self.nYSB = QtWidgets.QSpinBox(parent=self)
        self.nYSB.setValue(7)
        self.nShapeLabel = QtWidgets.QLabel("nShape", parent=self)
        self.nShapeSB = QtWidgets.QSpinBox(parent=self)
        self.nShapeSB.setValue(4)
        self.generateButton = QtWidgets.QPushButton("Generate Game", parent=self)
        self.destroyButton = QtWidgets.QPushButton("Destroy Game", parent=self)
        self.swapButton = QtWidgets.QPushButton("Swap Items", parent=self)
        
        # Add connections
        self.connect(self.generateButton, QtCore.SIGNAL("clicked()"), self.generate_game)
        self.connect(self.destroyButton, QtCore.SIGNAL("clicked()"), self.destroy_game)
        self.connect(self.swapButton, QtCore.SIGNAL("clicked()"), self.step_game)
        
        # Create layout for the widgets
        inputDescLayout = QtWidgets.QBoxLayout(QtWidgets.QBoxLayout.TopToBottom)
        inputDescLayout.addWidget(self.nXLabel)
        inputDescLayout.addWidget(self.nYLabel)
        inputDescLayout.addWidget(self.nShapeLabel)
        
        inputFieldLayout = QtWidgets.QBoxLayout(QtWidgets.QBoxLayout.TopToBottom)
        inputFieldLayout.addWidget(self.nXSB)
        inputFieldLayout.addWidget(self.nYSB)
        inputFieldLayout.addWidget(self.nShapeSB)
        
        buttonLayout = QtWidgets.QBoxLayout(QtWidgets.QBoxLayout.TopToBottom)
        buttonLayout.addWidget(self.generateButton)
        buttonLayout.addWidget(self.destroyButton)
        buttonLayout.addWidget(self.swapButton)
        
        self.layout = QtWidgets.QBoxLayout(QtWidgets.QBoxLayout.LeftToRight, self)
        self.layout.addLayout(inputDescLayout)
        self.layout.addLayout(inputFieldLayout)
        self.layout.addLayout(buttonLayout)
        
        self.game = MayaCrushGame(0,0,4)
        
    def say_hello(self):
        print('Hello, World from UI!')
        
    def generate_game(self):
        if(self.nShapeSB.value() > 5):
            self.nShapeSB.setValue(5)
        self.game.setup_game(self.nXSB.value(),self.nYSB.value(),self.nShapeSB.value())
        
    def destroy_game(self):
        self.game.clear_game()
    
    def step_game(self):
        self.game.swap_items()
        self.game.update_game_recursive()
        
    def read_UI(self):
        print('nX: %s' % (self.nXSB.value()))
        print('nY: %s' % (self.nYSB.value()))
        print('nShape: %s' % (self.nShapeSB.value()))

if __name__ == "__main__":
    random.seed(0)
    
    cmds.currentTime( 1 )
    cmds.playbackOptions( maxTime=256 )
    
    GameDialog().show()
