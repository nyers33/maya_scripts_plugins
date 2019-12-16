import maya.OpenMaya as openMaya
import maya.OpenMayaUI as openMayaUI
view = openMayaUI.M3dView.active3dView()
width = view.portWidth()
height = view.portHeight()
image = openMaya.MImage()

startHold=24*3
endHold=24*5+8

cmds.currentTime( 1 )
view.readColorBuffer(image, True)
for iTime in range( startHold ):
	image.writeToFile("pinwheel_"+format(iTime, '05')+".jpg", "jpg")

for iTime in range( 400 ):
	cmds.currentTime( iTime )
	view.readColorBuffer(image, True)
	image.writeToFile("pinwheel_"+format(startHold+iTime, '05')+".jpg", "jpg")

cmds.currentTime( 400 )
view.readColorBuffer(image, True)
for iTime in range( endHold ):
	image.writeToFile("pinwheel_"+format(startHold+400+iTime, '05')+".jpg", "jpg")

# camera eye ( 0 6 18 )
# camera aim ( 0 1.5 0 )
