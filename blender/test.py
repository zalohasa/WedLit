#####################################
# unbake ¿? f-curve to a new action #
#####################################

import bpy
obj = bpy.context.object

newAction = bpy.data.actions.new(name='Lot_of_Keys_new')
for c in obj.animation_data.action.fcurves:
    print("DataPath: ", c.data_path, " Index: ", c.array_index)
    if c.sampled_points and c.select:
        print("Convertingggg")
        fcu = newAction.fcurves.new(data_path=c.data_path, index=c.array_index)
        sam = c.sampled_points
        fcu.keyframe_points.add(len(sam))
        for i in range(len(sam)):
            w = fcu.keyframe_points[i]
            w.co = w.handle_left = w.handle_right = sam[i].co
            
obj.animation_data.action = newAction