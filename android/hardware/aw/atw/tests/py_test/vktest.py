# -*- coding: utf-8 -*-
import sys,os,math, shutil
import numpy as np

def cur_file_dir():
	path = sys.path[0];
	if os.path.isdir(path):
		return path;
	elif os.path.isfile(path):
		return os.path.dirname(path);
atw_path = cur_file_dir();
print("add " + atw_path + " to sys.path")
sys.path.append(atw_path);

import atw_optic
import atw_math
import atw_remap
import generator as gen
import atw_timewarp as atw

import atw_float2int as atw_convert

def GenerateMatrix( warpsInfo ):
    out = [];
    TexM = np.matrix(np.array([ 0.500000, 0.000000,-0.500000, 0.000000,
                                0.000000, 0.500000,-0.500000, 0.000000,
                                0.000000, 0.000000,-1.000000, 0.000000,
                                0.000000, 0.000000,-1.000000, 0.000000])).reshape(4,4);
    for warp in warpsInfo:
        #xx,yy,zz
        pitch = math.radians(warp[0]);
        yaw   = math.radians(warp[2]);
        roll  = math.radians(warp[4]);
        M0 = atw_math.QuatToMatrix(atw_math.EulerToQuat( pitch, yaw, roll ));
        pitch = math.radians(warp[1]);
        yaw   = math.radians(warp[3]);
        roll  = math.radians(warp[5]);
        M1 = atw_math.QuatToMatrix(atw_math.EulerToQuat( pitch, yaw, roll ));

        print(M0);
        print(M1);

        M0 = TexM * M0;
        M1 = TexM * M1;
        out.append(M0);
        out.append(M1);
    return np.array(out);

def GenDistortionInputQuads(tx,ty, wp,hp):
    #wp,hp = 1920,1080;
    wm = 0.126;
    hm = wm * hp / wp;
    distortionLeft = atw_optic.GetModelMesh_Per_Eye(3, list([wp,hp, wm,hm]), tx, ty, 0);
    # only left eye, and red channel
    dx = distortionLeft.T[0];
    dy = distortionLeft.T[1];
    blks = atw.verts2blks(dx,dy,tx,ty);
    return blks;

def DoSoftAtw(wp,hp,tx,ty, rot0,rot1,sdir):
    input_blks = [];
    output_blks = [];
    transformed_input_blks = [];
    output_coeffs = [];

    #get input_blks and output_blks
    input_blks = GenDistortionInputQuads(tx,ty,wp,hp);
    input_blks = np.array(input_blks).reshape(tx*ty,8);
    output_blks = atw_optic.GetOutpushBlocks(tx,ty);
    output_blks = np.array(output_blks).reshape(tx*ty,8);

    #get tansformed blks
    TexM = np.matrix(np.array([ 0.500000, 0.000000,-0.500000, 0.000000,
                                0.000000, 0.500000,-0.500000, 0.000000,
                                0.000000, 0.000000,-1.000000, 0.000000,
                                0.000000, 0.000000,-1.000000, 0.000000])).reshape(4,4);
    pitch = math.radians(rot0[0]);
    yaw   = math.radians(rot0[1]);
    roll  = math.radians(rot0[2]);
    M0 = atw_math.QuatToMatrix(atw_math.EulerToQuat( pitch, yaw, roll ));
    M0 = TexM * M0;
    print(M0);
    
    pitch = math.radians(rot1[0]);
    yaw   = math.radians(rot1[1]);
    roll  = math.radians(rot1[2]);    
    M1 = atw_math.QuatToMatrix(atw_math.EulerToQuat( pitch, yaw, roll ));
    M1 = TexM * M1;
    print(M1);
    
    for idx in range(0,tx*ty):
        blk = input_blks[idx];
        tmp_blk = [];
        blkHorizonIdx = idx%tx;
        blkIdx = idx%tx;
        xf = np.array([float(blkIdx)/float(tx), float((blkIdx+1))/float(tx)]);
        weight = np.array([xf[0],xf[1],xf[1],xf[0]]); # quad 的四个顶点为逆时针排序
        for i in range(0,4):
            vert = np.matrix([blk[i*2],blk[i*2+1], -1., 1.]).reshape(4,1);

            left  = M0 * vert;
            right = M1 * vert;
            proj = left * (1.-weight[i]) + right * weight[i];

# PATH0:
# 这是 ovr 原始 path:
            projZ = max(float(proj[2]), 0.00001);
            projIZ = 1.0 / projZ;
            uvw = proj * projIZ;

# PATH1:
# 这是为硬件做的 round up. 我们先关闭。 如果硬件挂死，我们就需要检查这里。
#            projIZ = 1. / proj[2];
#            if proj[2] < 0.00001:
#                projIZ = 10000;
#            uvw = proj * projIZ;
#            if(proj[2] < 0.00001):
#                vlen  = math.sqrt(proj[0]**2 + proj[1]**2);
#                scale = 1000./vlen;
#                uvw[0] = proj[0] * scale;
#                uvw[1] = proj[1] * scale;

            tmp_blk.append(list([uvw[0],uvw[1]]));
        tmp_blk = np.array(tmp_blk).reshape(8,1);
        transformed_input_blks.append(tmp_blk);
    transformed_input_blks = np.array(transformed_input_blks).reshape(tx*ty,8);

    # used for float to int conversion.    
    ouw = wp/2;
    ouh = hp;
    shift = atw_convert.GetShift(ouw, ouh);
    print ("shift is ",shift);
    output_coeffs_int32 = [];
    output_coeffs_flag = [];
    log = 0;
    for i in range(0,tx*ty):
        in_quad = transformed_input_blks[i];
        ou_quad = output_blks[i];
        coefficent = atw_remap.proj_matrix(ou_quad, in_quad);
        output_coeffs.append(coefficent);
        # convert float coefficents to int32 coefficents:
        fdata = np.array(coefficent).reshape(8,1);
        abs_dd = abs(fdata).max();
        flag = atw_convert.GetShiftBits(abs_dd);
        ndata = atw_convert.ConvertFromFloat2Int(wp,hp,flag,shift,fdata, log);
        output_coeffs_int32.append(ndata);
        output_coeffs_flag.append(flag);
    output_coeffs = np.array(output_coeffs).reshape(tx*ty,8);
    output_coeffs_int32 = np.array(output_coeffs_int32).reshape(tx*ty,8);
    output_coeffs_flag = np.array(output_coeffs_flag).reshape(tx*ty,1)

    #save all
    if True == os.path.exists(sdir):
        print "trying to delete dirs: " + sdir;
        shutil.rmtree(sdir);
    print "trying to build dirs: " + sdir;
    os.makedirs(sdir);
    print "change to dirs: " + sdir;
    os.chdir(sdir);
    np.savetxt("input_blks.txt", input_blks, fmt="%.8f");
    np.savetxt("output_blks.txt", output_blks, fmt="%.8f");
    np.savetxt("transformed_blks.txt", transformed_input_blks, fmt="%.8f");
    np.savetxt("coeffients_result_double.txt", output_coeffs, fmt="%.8f");
    np.savetxt("coeffients_result_int32.txt", output_coeffs_int32, fmt='%.8d');
    np.savetxt("coeffients_result_flags.txt", output_coeffs_flag, fmt='%.8d');
    return list([input_blks, output_blks, transformed_input_blks, output_coeffs, output_coeffs_int32, output_coeffs_flag]);

#wp,hp = 1920,1080;
wp,hp = 1440,1440;
tx,ty = 40, 40;
rot0,rot1 = np.array([10,0,0]), np.array([10,0,0]);
sdir = cur_file_dir() + "\data";
DoSoftAtw(wp,hp, tx,ty, rot0,rot1, sdir);
