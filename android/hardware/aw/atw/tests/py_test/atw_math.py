# -*- coding: utf-8 -*-
import numpy as np
import matplotlib.pylab as plt
import math

def QuatMul(a,b):# a = (s1,v1), b = (s2, v2)
    s1  = a[0]
    v1x = a[1]
    v1y = a[2]
    v1z = a[3]

    s2  = b[0]
    v2x = b[1]
    v2y = b[2]
    v2z = b[3]

    s3 = s1*s2 - (v1x * v2x + v1y * v2y + v1z * v2z)   
    ex = v1y*v2z - v1z*v2y
    ey = v1z*v2x - v1x*v2z
    ez = v1x*v2y - v1y*v2x
    s1v2x = s1 * v2x
    s1v2y = s1 * v2y
    s1v2z = s1 * v2z
    s2v1x = s2 * v1x
    s2v1y = s2 * v1y
    s2v1z = s2 * v1z
    v3x = ex + s1v2x + s2v1x 
    v3y = ey + s1v2y + s2v1y
    v3z = ez + s1v2z + s2v1z
    return np.array([s3, v3x, v3y, v3z])

def EulerToQuat(X,Y,Z):
    s1 = math.sin((X/2))
    s2 = math.sin((Y/2))
    s3 = math.sin((Z/2))
    c1 = math.cos((X/2))
    c2 = math.cos((Y/2))
    c3 = math.cos((Z/2))

    Qx = np.array([c1, s1, 0, 0]) 
    Qy = np.array([c2, 0, s2, 0])
    Qz = np.array([c3, 0, 0, s3])

    return QuatMul(QuatMul(Qx, Qy), Qz)

def QuatToMatrix(quat):
    w = quat[0]
    x = quat[1]
    y = quat[2]
    z = quat[3]
    ww = w*w
    xx = x*x
    yy = y*y
    zz = z*z
    
    M00 = ww + xx - yy -zz # M[0][0] = ww + xx - yy - zz;
    M01 = 2 * (x*y - w*z)  # M[0][1] = 2 * (q.x*q.y - q.w*q.z);
    M02 = 2 * (x*z + w*y)  # M[0][2] = 2 * (q.x*q.z + q.w*q.y);

    M10 = 2 * (x*y + w*z)  # M[1][0] = 2 * (q.x*q.y + q.w*q.z);
    M11 = ww - xx + yy -zz # M[1][1] = ww - xx + yy - zz;
    M12 = 2 * (y*z - w*x)  # M[1][2] = 2 * (q.y*q.z - q.w*q.x);
    
    M20 = 2 * (x*z - w*y)  # M[2][0] = 2 * (q.x*q.z - q.w*q.y);
    M21 = 2 * (y*z + w*x)  # M[2][1] = 2 * (q.y*q.z + q.w*q.x);
    M22 = ww - xx - yy + zz # M[2][2] = ww - xx - yy + zz;
    return np.array([[M00,M01,M02,0], [M10,M11,M12,0], [M20,M21,M22,0], [0,0,0,1]])

def ATW(DistX, DistY, tx, ty, rot0, rot1):
    out = []
    TexM = np.matrix(np.array([ 0.500000, 0.000000,-0.500000, 0.000000,
                                0.000000, 0.500000,-0.500000, 0.000000,
                                0.000000, 0.000000,-1.000000, 0.000000,
                                0.000000, 0.000000,-1.000000, 0.000000])).reshape(4,4);

    print("ATW");
    pitch = math.radians(rot0[0]);
    yaw   = math.radians(rot0[1]);
    roll  = math.radians(rot0[2]);
    M0 = QuatToMatrix(EulerToQuat( pitch, yaw, roll ));

    pitch = math.radians(rot1[0]);
    yaw   = math.radians(rot1[1]);
    roll  = math.radians(rot1[2]);    
    M1 = QuatToMatrix(EulerToQuat( pitch, yaw, roll ));

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