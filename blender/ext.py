import bpy
from math import radians
from mathutils import Matrix

num_channels = 3
bpy.ops.object.empty_add(type='PLAIN_AXES')

controller = bpy.data.objects["Empty"]
controller.name = "Controller"

controller.animation_data_create()
controller.animation_data.action = bpy.data.actions.new(name="Channels")

degrees_per_channel = 360/num_channels
current_degrees = 0;
material = bpy.data.materials.new("LightMat")
material.use_object_color = True

for i in range(num_channels):
	bpy.ops.mesh.primitive_torus_add(major_radius=0.5)
	torus = bpy.context.object
	torus.name = "Light_" + "{num:02d}".format(num=i)
	rot_mat = Matrix.Rotation(radians(-current_degrees), 4, 'Z')
	trans_mat = Matrix.Translation((-6, 0, 0))
	orig_loc, orig_rot, orig_scale = torus.matrix_world.decompose()
	orig_scale_mat = Matrix.Scale(orig_scale[0],4,(1,0,0)) * Matrix.Scale(orig_scale[1],4,(0,1,0)) * Matrix.Scale(orig_scale[2],4,(0,0,1))
	torus.matrix_world = rot_mat * trans_mat * orig_scale_mat

	#Add drivers for color
	t_r_c = torus.driver_add("color", 0)
	t_g_c = torus.driver_add("color", 1)
	t_b_c = torus.driver_add("color", 2)

	#Set material
	torus.active_material = material

	#Create names for the custom property
	customP = "channel_" + "{num:02d}".format(num=i)
	customPRef = "[\"" + customP + "\"]"

	#Add custom property, channel group for this channel and three fcurves, one for each index.
	controller[customP] = [0.0,0.0,0.0]
	grp = controller.animation_data.action.groups.new("Channel_"+str(i))
	r_crv = controller.animation_data.action.fcurves.new(customPRef, 0, "Channel_"+str(i))
	r_crv.color_mode = 'CUSTOM'
	r_crv.color = [1,0,0]
	g_crv = controller.animation_data.action.fcurves.new(customPRef, 1, "Channel_"+str(i))
	g_crv.color_mode = 'CUSTOM'
	g_crv.color = [0,1,0]
	b_crv = controller.animation_data.action.fcurves.new(customPRef, 2, "Channel_"+str(i))
	b_crv.color_mode = 'CUSTOM'
	b_crv.color = [0,0,1]

	#add curve modifier to limit the value to 1
	mod = r_crv.modifiers.new('LIMITS')
	mod.use_max_y = True
	mod.max_y = 1.0
	mod = g_crv.modifiers.new('LIMITS')
	mod.use_max_y = True
	mod.max_y = 1.0
	mod = b_crv.modifiers.new('LIMITS')
	mod.use_max_y = True
	mod.max_y = 1.0

	#Connect the drivers to the custom property of the controller
	rv = t_r_c.driver.variables.new()
	t_r_c.driver.expression = "var[0]"
	rv.name = "var"
	rv.targets[0].id_type = 'OBJECT'
	rv.targets[0].id = controller
	rv.targets[0].data_path = customPRef

	gv = t_g_c.driver.variables.new()
	t_g_c.driver.expression = "var[1]"
	gv.name = "var"
	gv.targets[0].id_type = 'OBJECT'
	gv.targets[0].id = controller
	gv.targets[0].data_path = customPRef

	bv = t_b_c.driver.variables.new()
	t_b_c.driver.expression = "var[2]"
	bv.name = "var"
	bv.targets[0].id_type = 'OBJECT'
	bv.targets[0].id = controller
	bv.targets[0].data_path = customPRef

	#Calculate next degrees for the next object.
	current_degrees += degrees_per_channel

