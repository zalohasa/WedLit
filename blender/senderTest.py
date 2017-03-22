import bpy
import math
import socket
import struct
import time
import threading

max_frames_per_packet = 40
obj = bpy.data.objects["Controller"]
action = obj.animation_data.action
framegroups = [action.groups[0], action.groups[1], action.groups[2]]
fps = bpy.context.scene.render.fps
Scene = bpy.context.scene
firstFramePlay = Scene.frame_start

startTime = math.trunc(time.time() * 1000)

cs = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
cs.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)

insertCmd = bytes("in", "ascii")
clearCmd = bytes("cl", "ascii")
cycleCmd = bytes("cy", "ascii")
endCmd = bytes("en", "ascii")
stopCmd = bytes("sp", "ascii")
playCmd = bytes("pl", "ascii")
offCmd = bytes("of", "ascii")



def send_data(data, ip="192.168.2.255", port=1234):
    cs.sendto(data, (ip, port))

def frame_to_time(frame_number):
    first_frame_time = (1000/fps) * (firstFramePlay-1)
    raw_time = (1000/fps) * (frame_number-1)
    return raw_time - first_frame_time

def sayHi():
    last_keyframe_sent = Scene.frame_start + (4*fps)

    buff = bytearray(6)
    inTime = math.trunc(time.time() * 1000)
    elapsed = inTime - startTime
    struct.pack_into("<BBI", buff, 0, 115, 155, elapsed)
    send_data(buff)
    print("Sent sync packet with elapsed: ", elapsed)
    while bpy.data.screens[0].is_animation_playing:
        to_frame = min(last_keyframe_sent+100, int(action.frame_range[1]))
        if last_keyframe_sent < int(action.frame_range[1]):
            sendKeyframes(last_keyframe_sent+1, to_frame)
            last_keyframe_sent = to_frame
        time.sleep(3.8)

    send_data(stopCmd)
    send_data(offCmd)

def sendKeyframes(fromFrame, toFrame):
    print("Sending from frame: ", fromFrame, " to frame: ", toFrame)
    print("Number of groups: ", len(framegroups))
    nGroups = len(framegroups)
    arrai = bytearray((4*nGroups) + 8*40)
    for grp in range(len(framegroups)):
        print("Sending group: ", framegroups[grp].name)
        r = framegroups[grp].channels[0]
        g = framegroups[grp].channels[1]
        b = framegroups[grp].channels[2]
        offset = 4 * nGroups
        number_sent = 0
        
        number_of_frames = toFrame - fromFrame
        if number_of_frames > max_frames_per_packet:
            if grp == 0:
                struct.pack_into("<BBHBBHBBH", arrai, 0, max_frames_per_packet, 0, 12, 0, 1, 12, 0,2,12)
            elif grp == 1:
                struct.pack_into("<BBHBBHBBH", arrai, 0, 0, 0, 12, max_frames_per_packet, 1, 12, 0,2,12)
            else:
                struct.pack_into("<BBHBBHBBH", arrai, 0, 0, 0, 12, 0, 1, 12, max_frames_per_packet, 2, 12)

        else:
            if grp == 0:
                struct.pack_into("<BBHBBHBBH", arrai, 0, number_of_frames, 0, 12, 0, 1, 12, 0, 2, 12)
            if grp == 1:
                struct.pack_into("<BBHBBHBBH", arrai, 0, 0, 0, 12, number_of_frames, 1, 12, 0, 2, 12)
            else:
                struct.pack_into("<BBHBBHBBH", arrai, 0, 0, 0, 12, 0, 1, 12, number_of_frames, 2, 12)

        for i in range(fromFrame, toFrame):
            rval = math.trunc(r.evaluate(i)*255)
            gval = math.trunc(g.evaluate(i)*255)
            bval = math.trunc(b.evaluate(i)*255)
            struct.pack_into("<IBBBB", arrai, offset, int(frame_to_time(i)), rval, gval, bval, 0)
            offset += 8
            number_sent += 1
            #print("Sending frame: ", i, "(", frame_to_time(i), ")")
            if number_sent == max_frames_per_packet:
                print("Max reached, sending")
                send_data(arrai)
                time.sleep(0.05)
                if grp == 0:
                    struct.pack_into("<BBHBBHBBH", arrai, 0, max_frames_per_packet, 0, 12, 0, 1, 12, 0,2,12)
                elif grp == 1:
                    struct.pack_into("<BBHBBHBBH", arrai, 0, 0, 0, 12, max_frames_per_packet, 1, 12, 0,2,12)
                else:
                    struct.pack_into("<BBHBBHBBH", arrai, 0, 0, 0, 12, 0, 1, 12, max_frames_per_packet, 2, 12)

                offset = 4 * nGroups
                number_sent = 0

        if number_sent > 0:
            #Overwrite the number of keyframes in the last packet.
            print("Sending last packet with: ", number_sent, " frames.")
            if grp == 0:
                struct.pack_into("<B", arrai, 0, number_sent)
            elif grp == 1:
                struct.pack_into("<B", arrai, 4, number_sent)
            else:
                struct.pack_into("<B", arrai, 8, number_sent)
                
            send_data(arrai)

t = threading.Timer(2, sayHi)

send_data(stopCmd)
send_data(clearCmd)
time.sleep(0.05)
send_data(insertCmd)
time.sleep(0.01)

sendKeyframes(Scene.frame_start, Scene.frame_start + (4*fps))

send_data(endCmd)
time.sleep(1)
#bpy.context.scene.frame_set(1)
send_data(playCmd)
time.sleep(0.01)
startTime = math.trunc(time.time() * 1000)
bpy.ops.screen.animation_play()

t.start()
