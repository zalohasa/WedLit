import bpy
import math
from mathutils import Matrix
import struct

class Key:
    """ Basic class to save keyframe data in the file"""
    def __init__(self, time, r, g, b):
        self.time = time
        self.r = r
        self.g = g
        self.b = b
        
    time = 0
    r = 0
    g = 0
    b = 0

class ChannelsPanel(bpy.types.Panel):
    bl_label = "Channels"
    bl_idname = "OBJECT_PT_channels"
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_context = "object"

    @classmethod
    def poll(cls, context):
        if context.object is not None:
            return len(context.object.channels) > 0
        return false

    def draw(self, context):
        layout = self.layout
        obj = context.object
        for cc in obj.channels:
            layout.prop(cc, "color", text=cc.name)

class ChannelProperty(bpy.types.PropertyGroup):
    color = bpy.props.FloatVectorProperty(subtype='COLOR', size=3)
    name = bpy.props.StringProperty()

class CreateWedLitOperator(bpy.types.Operator):
    bl_idname = "scene.init_wedlit"
    bl_label = "Initialize WedLit scene"

    num_channels = bpy.props.IntProperty(name="Number of channels")

    def invoke(self, context, event):
        wm = context.window_manager
        return wm.invoke_props_dialog(self)

    def execute(self, context):
        bpy.ops.object.empty_add(type='PLAIN_AXES')

        controller = bpy.data.objects["Empty"]
        controller.name = "Controller"

        controller.animation_data_create()
        controller.animation_data.action = bpy.data.actions.new(name="Channels")

        degrees_per_channel = 360/self.num_channels
        current_degrees = 0;
        material = bpy.data.materials.new("LightMat")
        material.use_object_color = True

        controller["t1"] = 0
        controller["t2"] = 0
        controller["t3"] = 0

        controller.animation_data.action.fcurves.new("[\"t1\"]")
        controller.animation_data.action.fcurves.new("[\"t2\"]")
        controller.animation_data.action.fcurves.new("[\"t3\"]")

        for i in range(self.num_channels):
            bpy.ops.mesh.primitive_torus_add(major_radius=0.5)
            torus = bpy.context.object
            torus.name = "Light_" + "{num:02d}".format(num=i)
            rot_mat = Matrix.Rotation(math.radians(-current_degrees), 4, 'Z')
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
            customPRef = "channels[" + str(i) + "].color"

            #Add custom property, channel group for this channel and three fcurves, one for each index.
            controller.channels.add()
            controller.channels[i].color = [0,0,0]
            controller.channels[i].name = "Channel_" + str(i)
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
        return {'FINISHED'}

class AddLimitsToSelectedCurves(bpy.types.Operator):
    bl_idname = "object.add_limits_to_curves"
    bl_label = "WedLit Add limit 1 to curves"

    def execute(self, context):
        obj = context.object
        action = obj.animation_data.action
        for grp in action.groups:
            r = grp.channels[0]
            g = grp.channels[1]
            b = grp.channels[2]
            if r.select:
                mod = r.modifiers.new('LIMITS')
                mod.use_max_y = True
                mod.max_y = 1.0
            if g.select:
                mod = g.modifiers.new('LIMITS')
                mod.use_max_y = True
                mod.max_y = 1.0
            if b.select:
                mod = b.modifiers.new('LIMITS')
                mod.use_max_y = True
                mod.max_y = 1.0

        return {'FINISHED'}



class SaveKeyframeFile(bpy.types.Operator):
    bl_idname = "scene.save_anim_file"
    bl_label = "Save WedLit animation file"

    filepath = bpy.props.StringProperty(subtype = "FILE_PATH")

    fps = 0
    firstFramePlay = 0

    def frame_to_time(self, frame_number):
        first_frame_time = (1000/self.fps) * (self.firstFramePlay-1)
        raw_time = (1000/self.fps) * (frame_number-1)
        return raw_time - first_frame_time

    def process_sampled(self, r, g, b):
        keys = []
        n = 0
        sel = None
        rs = r.sampled_points
        gs = g.sampled_points
        bs = b.sampled_points
        
        if len(r.sampled_points) > n:
            n = len(r.sampled_points)
            sel = rs
        if len(g.sampled_points) > n:
            n = len(g.sampled_points)
            sel = gs
        if len(b.sampled_points) > n:
            n = len(b.sampled_points)
            sel = bs
        
        if sel is not None:    
            for point in sel:
                rval = math.trunc(r.evaluate(point.co[0]) * 255)
                gval = math.trunc(g.evaluate(point.co[0]) * 255)
                bval = math.trunc(b.evaluate(point.co[0]) * 255)
                time = int(self.frame_to_time(point.co[0]))
                keys.append(Key(time, rval, gval, bval))
        return keys

    def process_keyframed(self, r, g, b):
        keys = []
        for i in range(len(r.keyframe_points)):
            rval = math.trunc(r.keyframe_points[i].co[1] * 255)
            gval = math.trunc(g.keyframe_points[i].co[1] * 255)
            bval = math.trunc(b.keyframe_points[i].co[1] * 255)
            time = int(self.frame_to_time(r.keyframe_points[i].co[0]))
            keys.append(Key(time, rval, gval, bval))
        return keys

    def invoke(self, context, event):
        context.window_manager.fileselect_add(self)
        return {"RUNNING_MODAL"}

    def execute(self, context):
        file = open(self.filepath, "wb")
        obj = bpy.data.objects["Controller"]
        action = obj.animation_data.action
        number_of_channels = len(obj.channels)
        nKeyframes = []
        self.fps = bpy.context.scene.render.fps
        Scene = bpy.context.scene
        self.firstFramePlay = Scene.frame_start

        nChBuff = bytearray(1)
        print("Number of channels: ", number_of_channels)
        struct.pack_into("<B", nChBuff, 0, number_of_channels)
        file.write(nChBuff)
        file.seek((number_of_channels*4) + 1)

        nChannel = 0
        for grp in action.groups:
            r = grp.channels[0]
            g = grp.channels[1]
            b = grp.channels[2]
            keys = []
            if len(r.sampled_points) > 0 or len(g.sampled_points) > 0 or len(b.sampled_points) > 0:
                print("Processing sampled curve")
                keys = self.process_sampled(r,g,b)
            else:
                print("Processing keyframed curve")
                keys = self.process_keyframed(r, g, b)

            keyBuff = bytearray(7)    
            for key in keys:
                struct.pack_into("<IBBB", keyBuff, 0, key.time, key.r, key.g, key.b)
                file.write(keyBuff)

            nKeyframes.append(len(keys))
            
        file.seek(1)
        for num in nKeyframes:
            nKeyBuff = bytearray(4)
            struct.pack_into("<I", nKeyBuff, 0, num)
            file.write(nKeyBuff)
            
        file.close()
        return {'FINISHED'}



def register():
    bpy.utils.register_class(ChannelProperty)
    bpy.types.Object.channels = bpy.props.CollectionProperty(type=ChannelProperty)
    bpy.utils.register_class(ChannelsPanel)
    bpy.utils.register_class(CreateWedLitOperator)
    bpy.utils.register_class(SaveKeyframeFile)
    bpy.utils.register_class(AddLimitsToSelectedCurves)

def unregister():
    bpy.utils.unregister_class(AddLimitsToSelectedCurves)
    bpy.utils.unregister_class(SaveKeyframeFile)
    bpy.utils.unregister_class(CreateWedLitOperator)
    bpy.utils.unregister_class(ChannelsPanel)
    bpy.utils.unregister_class(ChannelProperty)

if __name__ == "__main__":
    register()

