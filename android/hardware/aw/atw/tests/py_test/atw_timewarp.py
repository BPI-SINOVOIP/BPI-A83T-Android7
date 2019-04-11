# -*- coding: utf-8 -*-
import numpy as np
import math
import atw_math

print("load atw_data v1.1");
def verts2blks(ux,uy, tx,ty): #顶点转block
    xnum = ux.shape[0];
    ynum = uy.shape[0];

    if xnum != ynum or xnum != (tx+1)*(ty+1):
        print("GetQuads meet error data.", xnum, ynum, (tx+1)*(ty+1));
        return;

    ttx = tx + 1;
    out = [];
    for y in range(0,ty):
        for x in range(0,tx):
            i1 = y*ttx+x;
            u1 = ux[i1];
            v1 = uy[i1];

            i2 = y*ttx+x+1;
            u2 = ux[i2];
            v2 = uy[i2];

            i3 = (y+1)*ttx+x;
            u3 = ux[i3];
            v3 = uy[i3];

            i4 = (y+1)*ttx+x+1;
            u4 = ux[i4];
            v4 = uy[i4];

            out.append([u1,v1,u2,v2,u4,v4,u3,v3]); # 注意这里！ 我们用的是逆时针存储方式。
    return np.array(out).reshape(tx*ty,8);

def ATW(DistX, DistY, tx, ty, rot0, rot1):
    out = []
    TexM = np.matrix(np.array([ 0.500000, 0.000000,-0.500000, 0.000000,
                                0.000000, 0.500000,-0.500000, 0.000000,
                                0.000000, 0.000000,-1.000000, 0.000000,
                                0.000000, 0.000000,-1.000000, 0.000000])).reshape(4,4);
    pitch = math.radians(rot0[0]);
    yaw   = math.radians(rot0[1]);
    roll  = math.radians(rot0[2]);
    M0 = atw_math.QuatToMatrix(atw_math.EulerToQuat( pitch, yaw, roll ));

    pitch = math.radians(rot1[0]);
    yaw   = math.radians(rot1[1]);
    roll  = math.radians(rot1[2]);    
    M1 = atw_math.QuatToMatrix(atw_math.EulerToQuat( pitch, yaw, roll ));

    M0 = TexM * M0;
    M1 = TexM * M1;
    
    xnum = DistX.shape[0];
    ynum = DistX.shape[0];
    if xnum != ynum or xnum != (tx+1)*(ty+1):
        print("ATW meet error data.", xnum, ynum, (tx+1)*(ty+1));
        return;

    TX = tx + 1;
    TY = ty + 1;
    for y in range(0,TY):
        for x in range(0,TX):
            vert = np.matrix([DistX[y*TX+x], DistY[y*TX+x], -1., 1.]).reshape(4,1);

            left  = M0 * vert;
            right = M1 * vert;

            xf = x / float(tx);
            proj = left * (1.-xf) + right * xf;
            projIZ = 1. / proj[2];
            if proj[2] < 0.00001:
                projIZ = 10000;
            uvw = proj * projIZ;
            if(proj[2] < 0.00001):
                vlen  = math.sqrt(proj[0]**2 + proj[1]**2);
                scale = 1000./vlen;
                uvw[0] = proj[0] * scale;
                uvw[1] = proj[1] * scale;

            out.append(np.array([uvw[0], uvw[1]]));
    return np.array(out).reshape(TY*TX,2);

def main(mesh, tx, ty, warpInfo):
    pitch = warpInfo[0];
    yaw   = warpInfo[1];
    roll  = warpInfo[2];
    delta = warpInfo[3];

    out = [];
    for ch in range(0,3):
        dx = mesh.T[ch*2];
        dy = mesh.T[ch*2+1];
        rot0 = np.array([pitch,yaw,roll]);
        rot1 = rot0 + delta;
        verts  = ATW(dx,dy, tx,ty, rot0,rot1);
        blks = verts2blks(verts.T[0],verts.T[1], tx,ty);# 四边形blocks。
        out.append(list([verts, blks]));
    return out;

def _main_(mesh, tx, ty, rot0, rot1):
    out = [];
    for ch in range(0,3):
        dx = mesh.T[ch*2];
        dy = mesh.T[ch*2+1];
        verts  = ATW(dx,dy, tx,ty, rot0,rot1);
        blks = verts2blks(verts.T[0],verts.T[1], tx,ty);
        out.append(list([verts, blks]));
    return out;

def _warp_per_ch_(mesh, tx, ty, rot0, rot1, ch):
    dx = mesh.T[ch*2];
    dy = mesh.T[ch*2+1];
    verts  = ATW(dx,dy, tx,ty, rot0,rot1);
    blks = verts2blks(verts.T[0],verts.T[1], tx,ty);
    return list([verts, blks]);

def debug_verts2blks(ux,uy, tx,ty):#列扫描方向, 支持 1080p
    tty = ty+1;
    out = [];
    for x in range(0,tx):
        for y in range(0,ty):
            i1 = y*tty+x;
            i2 = (y+1)*tty+x;
            i3 = i1+1;
            i4 = i2+1;
            u1,v1 = ux[i1], uy[i1];
            u2,v2 = ux[i2], uy[i2];
            u3,v3 = ux[i3], uy[i3];
            u4,v4 = ux[i4], uy[i4];
            out.append([u1,v1,u2,v2,u4,v4,u3,v3]);
    return np.array(out).reshape(tx*ty,8);
def debug_main_by_scan_order(mesh, tx, ty, rot0, rot1):
    out = [];
    for ch in range(0,3):
        dx = mesh.T[ch*2];
        dy = mesh.T[ch*2+1];
        verts  = ATW(dx,dy, tx,ty, rot0,rot1);
        blks = debug_verts2blks(verts.T[0],verts.T[1], tx,ty);
        out.append(list([verts, blks]));
    return out;

def _warp_per_ch_by_scan_order_(mesh, tx, ty, rot0, rot1, ch):
    dx = mesh.T[ch*2];
    dy = mesh.T[ch*2+1];
    verts  = ATW(dx,dy, tx,ty, rot0,rot1);
    blks = debug_verts2blks(verts.T[0],verts.T[1], tx,ty);
    return list([verts, blks]);
