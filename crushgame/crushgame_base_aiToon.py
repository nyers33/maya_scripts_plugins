import maya.cmds as cmds
# defaultArnoldFilter ---> Filter Type = contour
cmds.setAttr( 'defaultArnoldFilter.width', 2.0 )

def create_aiToon(name, r=1.0, g=1.0, b=1.0):
	ai_shader_name = 'aiToon_'+name
	ai_shader_sg_name = 'aiToon_'+name+'_SG'
	shader = cmds.shadingNode( 'aiToon', name=ai_shader_name, asShader=True )
	shading_network = cmds.sets( renderable=True, noSurfaceShader=True, empty=True, name=ai_shader_sg_name )
	cmds.connectAttr( ai_shader_name+'.outColor', ai_shader_sg_name+'.surfaceShader' )
	cmds.setAttr( ai_shader_name+'.base', 0.8)
	cmds.setAttr( ai_shader_name+'.silhouetteWidthScale', 2.0)

	ramp_name = 'ramp_'+name
	p2dtext_name = 'place2dTexture_'+name
	ramp = cmds.shadingNode( 'ramp', name=ramp_name, asTexture=True )
	place2dTexture = cmds.shadingNode( 'place2dTexture', name=p2dtext_name, asUtility=True )
	cmds.connectAttr( p2dtext_name+'.outUV', ramp_name+'.uv' )
	cmds.connectAttr( p2dtext_name+'.outUvFilterSize', ramp_name+'.uvFilterSize' )
	cmds.setAttr( ramp_name+'.interpolation', 0 )
	cmds.setAttr( ramp_name+'.colorEntryList[2].position', 0.333 )
	cmds.setAttr( ramp_name+'.colorEntryList[3].position', 0.666 )
	cmds.setAttr( ramp_name+'.colorEntryList[0].color', 0.000, 0.000, 0.000, type='double3' )
	cmds.setAttr( ramp_name+'.colorEntryList[2].color', r/2.0, g/2.0, b/2.0, type='double3' )
	cmds.setAttr( ramp_name+'.colorEntryList[3].color', r/1.0, g/1.0, b/1.0, type='double3' )
	cmds.setAttr( ramp_name+'.colorEntryList[1].color', r/1.0, g/1.0, b/1.0, type='double3' )
	cmds.setAttr( ramp_name+'.colorEntryList[1].position', 1 )
	cmds.connectAttr( ramp_name+'.outColor', ai_shader_name+'.baseTonemap', force=True )
	
create_aiToon('red', 1.0, 0.0, 0.0)
create_aiToon('green', 0.0, 1.0, 0.0)
create_aiToon('blue', 0.0, 0.0, 1.0)
create_aiToon('yellow', 1.0, 1.0, 0.0)
create_aiToon('white', 1.0, 1.0, 1.0)

cmds.select(cmds.sets('lambert_red_SG', q=True))
cmds.sets( e=True, forceElement='aiToon_red_SG' )
cmds.select(cmds.sets('lambert_green_SG', q=True))
cmds.sets( e=True, forceElement='aiToon_green_SG' )
cmds.select(cmds.sets('lambert_blue_SG', q=True))
cmds.sets( e=True, forceElement='aiToon_blue_SG' )
cmds.select(cmds.sets('lambert_yellow_SG', q=True))
cmds.sets( e=True, forceElement='aiToon_yellow_SG' )
cmds.select(cmds.sets('lambert_white_SG', q=True))
cmds.sets( e=True, forceElement='aiToon_white_SG' )

cmds.shadingNode( 'aiToon', name='aiToon_default', asShader=True )
cmds.sets( renderable=True, noSurfaceShader=True, empty=True, name='aiToon_default_SG' )
cmds.connectAttr( 'aiToon_default.outColor', 'aiToon_default_SG.surfaceShader' )
cmds.setAttr( 'aiToon_default.base', 0.8)
cmds.setAttr( 'aiToon_default.silhouetteWidthScale', 2.0)

cmds.select(cmds.sets('initialShadingGroup', q=True))
cmds.sets( e=True, forceElement='aiToon_default_SG' )

light = cmds.directionalLight(rotation=(0, 0, 0))
cmds.directionalLight( light, e=True, intensity=4.0 )

camera_name = cmds.camera(orthographic=True)
camera_shape = camera_name[1]
cmds.setAttr( camera_name[0]+'.translateX', 17.4 )
cmds.setAttr( camera_name[0]+'.translateY', 24.55 )
cmds.setAttr( camera_name[0]+'.translateZ', 26.5 )
cmds.setAttr( camera_shape+'.orthographicWidth', 50.0 )

# 720x800
