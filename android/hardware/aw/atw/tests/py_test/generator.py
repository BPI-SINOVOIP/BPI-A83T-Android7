# -*- coding: utf-8 -*-
import sys,os
import numpy as np

def cur_file_dir():
	path = sys.path[0];
	if os.path.isdir(path):
		return path;
	elif os.path.isfile(path):
		return os.path.dirname(path);
atw_path = cur_file_dir();
sys.path.append(atw_path);

import atw_optic
import atw_timewarp

def rotate_xyz(xx,yy,zz):
    warps = [];
    for x in xx:
        for y in yy:
            for z in zz:
                warps.append(list([x,x, y,y, z,z]));
    return warps;

def rotate_x_y_z(xx,yy,zz):
    warps = [];
    for x in xx:
        warps.append(list([x,x, 0,0, 0,0]));
    for y in yy:
        warps.append(list([0,0, y,y, 0,0]));
    for z in zz:
        warps.append(list([0,0, 0,0, z,z]));
    return warps;
    
def rotate_x_y_z_with_delta(xx,yy,zz,dd):
    warps = [];
    for delta in dd:
        for x in xx:
            warps.append(list([x,x+delta, 0,0, 0,0]));
        for y in yy:
            warps.append(list([0,0, y,y+delta, 0,0]));
        for z in zz:
            warps.append(list([0,0, 0,0, z,z+delta]));
    return warps;

def convert_atw_info_to_readable_single_axis_rotate(_dir, fname):
    cwd = os.getcwd();
    os.chdir(_dir);
    atwinfos = np.loadtxt(fname);
    
    f = open("atw_info_read_me.txt", "w+");

    index = 0;
    for line in atwinfos:
        x = '%5.1f' % line[0];
        y = '%5.1f' % line[2];
        z = '%5.1f' % line[4];

        delta = ((line[1]+line[3]+line[5])-(line[0]+ line[2]+line[4]));# 注意，单轴和多轴的 delta 算法不同。
        delta = '%5.1f' % delta;
        fname = '%4d' % index + ".txt";

        _readable_info = fname + " : " + "  x = "+ x + ", y = "+ y + ", z = " + z + ", delta = "+ delta;

        f.write(_readable_info + "\r\n");
        index = index+1;

    f.close();
    os.chdir(cwd);

def save_floats_array_in_dir(_dir, array, fname):
    cwd = os.getcwd();
    if os.path.isdir(_dir):
        os.chdir(_dir);
    else:
        os.mkdir(_dir);
        os.chdir(_dir);
    np.savetxt(fname, array);
    os.chdir(cwd);

def CreateHmdTestDataPerEye(hmd, wp, hp, tx, ty, warps, eye):
    cwd = os.getcwd();
    eye_dir = "";
    if eye == 0:
        eye_dir = "left_eye";
    if eye == 1:
        eye_dir = "right_eye";
    if os.path.isdir(eye_dir) == False:
        os.mkdir(eye_dir);
    os.chdir(eye_dir);
    
    wm = 0.126;
    hm = wm * hp / wp;

    #获得基础网格
    modelscreenInfo = list([wp,hp, wm,hm]);
    modelVerts = atw_optic.GetModelMesh_Per_Eye(hmd, modelscreenInfo, tx, ty, eye);
    
    fileindex = 0;#文件名从0开始。
    for warp in warps:
        rot0 = np.array([warp[0],warp[2],warp[4]]);
        rot1 = np.array([warp[1],warp[3],warp[5]]);
        warpResult = atw_timewarp._main_(modelVerts, tx, ty, rot0, rot1);
        print (fileindex, warps.shape[0]);
        for ch in range(0,3):# warpResult是list，共3个elem，分别是 r,g,b 旋转后的 [verts,blks]
            warpBlks = warpResult[ch][1].reshape(tx*ty,8);
            if ch == 0:
                save_floats_array_in_dir("r",warpBlks, bytes(fileindex) + ".txt");
            elif ch == 1:
                save_floats_array_in_dir("g",warpBlks, bytes(fileindex) + ".txt");
            else:
                save_floats_array_in_dir("b",warpBlks, bytes(fileindex) + ".txt");
        fileindex += 1;
    os.chdir(cwd);

def CreateHmdTestDataPerEyePerCh(hmd, wp, hp, tx, ty, warps, eye, ch):
    cwd = os.getcwd();
    eye_dir = "";
    if eye == 0:
        eye_dir = "left_eye";
    if eye == 1:
        eye_dir = "right_eye";
    if os.path.isdir(eye_dir) == False:
        os.mkdir(eye_dir);
    os.chdir(eye_dir);

    chtag = "r";
    if ch == 1:
        chtag = "g";
    if ch == 2:
        chtag = "b";    

    wm = 0.126;
    hm = wm * hp / wp;
    modelscreenInfo = list([wp,hp, wm,hm]);
    modelVerts = atw_optic.GetModelMesh_Per_Eye(hmd, modelscreenInfo, tx, ty, eye);#获得基础网格

    fileindex = 0;#文件名从0开始。
    for warp in warps:
        print "output: " + bytes(fileindex+1) + " of " + bytes(warps.shape[0]) + " for " + chtag;
        rot0,rot1 = np.array([warp[0],warp[2],warp[4]]), np.array([warp[1],warp[3],warp[5]]);
        out = atw_timewarp._warp_per_ch_(modelVerts, tx, ty, rot0, rot1, ch);
        warpBlks = out[1].reshape(tx*ty,8);
        save_floats_array_in_dir(chtag,warpBlks, bytes(fileindex) + ".txt");
        fileindex = fileindex + 1;
    os.chdir(cwd);

def Generator(model, _rootdir, wp, hp, tx, ty, warps):
    if os.path.isdir(_rootdir) == False:
        os.mkdir(_rootdir);
    os.chdir(_rootdir);

    _subdir = "hmd" + bytes(model) + "_w=" + bytes(wp/2) + "_h=" + bytes(hp) + "_" + bytes(tx) + "x" + bytes(ty);
    if os.path.isdir(_subdir) == False:
        os.mkdir(_subdir);
    os.chdir(_subdir);

    warps = np.array(warps);
    np.savetxt("atw_info.txt", warps, "%6f");#Lx,Rx, Ly,Ry, Lz,Rz.
    cwd = os.getcwd();
    CreateHmdTestDataPerEye(model, wp, hp, tx, ty, warps, 0);
    CreateHmdTestDataPerEye(model, wp, hp, tx, ty, warps, 1);
    return cwd;

def Fast_Generator(model, _rootdir, wp, hp, tx, ty, warps, eye, ch):
    if os.path.isdir(_rootdir) == False:
        os.mkdir(_rootdir);
    os.chdir(_rootdir);

    _subdir = "hmd" + bytes(model) + "_w=" + bytes(wp/2) + "_h=" + bytes(hp) + "_" + bytes(tx) + "x" + bytes(ty);
    if os.path.isdir(_subdir) == False:
        os.mkdir(_subdir);
    os.chdir(_subdir);

    warps = np.array(warps);
    np.savetxt("atw_info.txt", warps, "%6f");#Lx,Rx, Ly,Ry, Lz,Rz.
    cwd = os.getcwd();
    CreateHmdTestDataPerEyePerCh(model, wp, hp, tx, ty, warps, eye, ch);
    return cwd;

def rotate_xyz_with_delta(xx,yy,zz, dd):
    warps = [];
    for delta in dd:
        for x in xx:
            for y in yy:
                for z in zz:
                    warps.append(list([x,x+delta, y,y+delta, z,z+delta]));
    return warps;

def CreateHmdTestDataPerEyePerChByScanOrder(hmd, wp, hp, tx, ty, warps, eye, ch):
    cwd = os.getcwd();
    eye_dir = "";
    if eye == 0:
        eye_dir = "left_eye";
    if eye == 1:
        eye_dir = "right_eye";
    if os.path.isdir(eye_dir) == False:
        os.mkdir(eye_dir);
    os.chdir(eye_dir);

    chtag = "r";
    if ch == 1:
        chtag = "g";
    if ch == 2:
        chtag = "b";    

    wm = 0.126;
    hm = wm * hp / wp;
    modelscreenInfo = list([wp,hp, wm,hm]);
    modelVerts = atw_optic.GetModelMesh_Per_Eye(hmd, modelscreenInfo, tx, ty, eye);#获得基础网格

    fileindex = 0;#文件名从0开始。
    for warp in warps:
        print "output: " + bytes(fileindex+1) + " of " + bytes(warps.shape[0]) + " for " + chtag;
        rot0,rot1 = np.array([warp[0],warp[2],warp[4]]), np.array([warp[1],warp[3],warp[5]]);
        out = atw_timewarp._warp_per_ch_by_scan_order_(modelVerts, tx, ty, rot0, rot1, ch);
        warpBlks = out[1].reshape(tx*ty,8);
        save_floats_array_in_dir(chtag,warpBlks, bytes(fileindex) + ".txt");
        fileindex = fileindex + 1;
    os.chdir(cwd);
def FastGeneratorByScanOrder(model, _rootdir, wp, hp, tx, ty, warps, eye, ch):
    if os.path.isdir(_rootdir) == False:
        os.mkdir(_rootdir);
    os.chdir(_rootdir);

    _subdir = "hmd" + bytes(model) + "_w=" + bytes(wp/2) + "_h=" + bytes(hp) + "_" + bytes(tx) + "x" + bytes(ty);
    if os.path.isdir(_subdir) == False:
        os.mkdir(_subdir);
    os.chdir(_subdir);

    warps = np.array(warps);
    np.savetxt("atw_info.txt", warps, "%6f");#Lx,Rx, Ly,Ry, Lz,Rz.
    cwd = os.getcwd();
    CreateHmdTestDataPerEyePerChByScanOrder(model, wp, hp, tx, ty, warps, eye, ch);
    return cwd;

#dd = np.array(list([0]));
#xx = np.array(list([10]));
#yy = dd;
#zz = dd;
#rotate_x_y_z_with_delta(xx,yy,zz,dd);
#Generator(3, "testdir", 1080*2, 960, 40, 40, list());