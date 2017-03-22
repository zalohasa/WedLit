import bpy
import math
import struct

class Key:
    def __init__(self, time, r, g, b):
        self.time = time
        self.r = r
        self.g = g
        self.b = b
        
    time = 0
    r = 0
    g = 0
    b = 0
    

file = open("/home/zalo/test.anim", "wb")
obj = bpy.data.objects["Controller"]
action = obj.animation_data.action
number_of_channels = len(action.groups)
nKeyframes = []
fps = bpy.context.scene.render.fps
Scene = bpy.context.scene
firstFramePlay = Scene.frame_start

def process_sampled(r, g, b):
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
            time = int(frame_to_time(point.co[0]))
            keys.append(Key(time, rval, gval, bval))
    return keys
    
def frame_to_time(frame_number):
    first_frame_time = (1000/fps) * (firstFramePlay-1)
    raw_time = (1000/fps) * (frame_number-1)
    return raw_time - first_frame_time

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
        keys = process_sampled(r,g,b)
    else:
        print("Keyframe curves not supported yet")

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
    
