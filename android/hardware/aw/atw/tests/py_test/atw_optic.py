# -*- coding: utf-8 -*-
import numpy as np
import math

def EvalCatmullRomSpline(K, scaledVal, NumSegments):
    scaledValFloor =  math.floor(scaledVal);
    scaledValFloor = max ( 0.0, min ( (NumSegments-1.), scaledValFloor ) );
    t = scaledVal - scaledValFloor;
    k = int(scaledValFloor);

    p0 = 0.0;
    p1 = 0.0;
    m0 = 0.0;
    m1 = 0.0;

    # 分析:
    #     k=0    =>      scaledValFloor = 0  =>  0.0 <= scaledVal < 1.0 (1)
    # 1 <= k < 9 => 1 <= scaledValFloor < 9  =>  1.0 <  scaledVal < 9.0  (2)
    #     k=9    =>      scaledValFloor = 9  =>  9.0 <= scaledVal < 10.0   (3)
    #     k=10   =>      scaledValFloor = 10 => 10.0 <= scaledVal < 11.0   (4)
    #  而：
    #   scaledVal = 10 * rsq = 10 * (tan(theta)**2);
    #  最终
    #  (1) =>    0 <= theta < 17.5
    #  (2) => 17.4 <= theta < 43.5
    #  (3) => 43.5 <= theta < 45.0
    #  (4) => 45.0 <= theta < maxTheta = 50.7
    #  maxTheta = 距离中心最远的点所对应的角度，即屏幕的四个角。
    #  model3, 2048x1024 ,w = 0.126 meters, h = 0.063meters , lensMetersPerTanAngleAtCenter = 0.0365,  => maxTheta = 50.7
    if (k == 0):
        p0 = K[0]; #1.0f;
        m0 = ( K[1] - K[0] );    # general case would have been (K[1]-K[-1])/2
        p1 = K[1];
        m1 = 0.5 * ( K[2] - K[0] );
    elif (k < NumSegments-2):
        p0 = K[k];
        m0 = 0.5 * ( K[k+1] - K[k-1] );
        p1 = K[k+1];
        m1 = 0.5 * ( K[k+2] - K[k] );
    elif (k == NumSegments-2):
        p0 = K[NumSegments-2];
        m0 = 0.5 * ( K[NumSegments-1] - K[NumSegments-2] );
        p1 = K[NumSegments-1];
        m1 = K[NumSegments-1] - K[NumSegments-2];
    elif (k == NumSegments-1):
        p0 = K[NumSegments-1];
        m0 = K[NumSegments-1] - K[NumSegments-2];
        p1 = p0 + m0;
        m1 = m0;

    omt = 1.0 - t;
    res = ( p0 * ( 1.0 + 2.0 * t ) + m0 * t ) * omt * omt \
            + ( p1 * ( 1.0 + 2.0 * omt ) - m1 * omt ) * t * t;
    return res;

def DistortionFn_CatmullRom10(K, rsq):
    #A Catmull-Rom spline through the values 1.0, K[1], K[2] ... K[10]
    #evenly spaced in R^2 from 0.0 to MaxR^2
    #K[0] controls the slope at radius=0.0, rather than the actual value.
    NumSegments = 11;
    MaxR = 1.;
    scaledRsq = (NumSegments-1) * rsq / ( MaxR * MaxR );
    scale = EvalCatmullRomSpline(K, scaledRsq, NumSegments);
    return scale;

def DistortionFn_CatmullRom20(K, rsq):
    NumSegments = 21;
    MaxR = 1.;
    scaledRsq = (NumSegments-1) * rsq / ( MaxR * MaxR );
    scale = EvalCatmullRomSpline(K, scaledRsq, NumSegments);
    return scale;

def DisortionFn_Pol4(K, rsq):
    return ( K[0] + rsq * ( K[1] + rsq * ( K[2] + rsq * K[3] ) ) );

def DistortionFn_RecipPoly4(K, rsq):
    return 1. / (K[0] + rsq *( K[1] + rsq * ( K[2] + rsq * K[3] )));

def Distortion(wp, hp, wm, hm, ipd, K, DisortionFn, lensMetersPerTanAngleAtCenter):
    out = []
    aspect = wp*0.5 / hp;
    shift = ipd/2. - wm/4.;
    shiftview = 2 * aspect * shift / wm;
    print "Warning ! This Function only support 32x32";
    #TODO
    #-0.006000 0.000000 0.014000 0.000000
    ChromaticAberration = list([-0.006, 0., 0.014, 0.0]);

    for y in range(0,33):
        for x in range(0,33):
            xf = shiftview + x/32. * aspect + (1. - aspect) * 0.5;
            # xf 计算含义：
            # shiftview : left eye
            # x/32. * aspect : 32 等分，转到 [0，1] 后，再转到 [0,aspect]
            # (1. - aspect)*0.5 :  如果 aspect = 1., 那么就不用移动， 这是为了让 网格 正好在 瞳孔 位置。
            #                      在后面的 dx 计算中，(1.-aspect)*0.5 是关键的区别。
            # 对于理解算法， 可以认为 xf = x/32.;
            yf = y/32.;

            dx = (xf - 0.5) * hp * (wm /wp);# 注意， xf != yf,  在 aspect != 0 的情况下。
            dy = (yf - 0.5) * hp * (wm /wp);# wm/wp = 每个像素的大小
                                          
            tanx = dx / lensMetersPerTanAngleAtCenter;
            tany = dy / lensMetersPerTanAngleAtCenter;            
            rsq = tanx*tanx + tany*tany;
            scale = DisortionFn(K, rsq);#1. / (K[0] + rsq *( K[1] + rsq * ( K[2] + rsq * K[3] )));

            scaleRed   = scale * ( 1. + ChromaticAberration[0] + rsq * ChromaticAberration[1] );
            scaleGreen = scale;
            scaleBlue  = scale * ( 1. + ChromaticAberration[2] + rsq * ChromaticAberration[3] );


            Rx = scaleRed * tanx;
            Ry = scaleRed * tany;

            Gx = scaleGreen * tanx;
            Gy = scaleGreen * tany;

            Bx = scaleBlue * tanx;
            By = scaleBlue * tany;

            out.append(np.array([Rx,Ry, Gx,Gy, Bx,By, xf,yf]));
    return np.array(out).reshape(33*33,8);

def GetOutpushBlocks(tx,ty):
    out = np.zeros(shape=(tx,ty,8));
    xf = 1./tx;
    yf = 1./ty;
    for y in range(0,ty):
        for x in range(0,tx):
            u1 = x*xf;
            v1 = y*yf;

            u2 = (x+1)*xf;
            v2 = y*yf;

            u3 = x*xf;
            v3 = (y+1)*yf;

            u4 = (x+1)*xf;
            v4 = (y+1)*yf;

            out[y,x] = list([u1,v1, u2,v2, u4,v4, u3,v3]); #[row, col]
    return out;

def GetModelOpticInfo(model):
    if model == 3: ## HMD_NOTE_4
        ipd = 0.063;
        fov = 90.0;
        Eqn = DistortionFn_CatmullRom10;
        scale = 0.0365;
        K = np.array([[1.0],[1.029],[1.0565],[1.088],[1.127],[1.175],[1.232],[1.298],[1.375],[1.464],[1.570]]);
        optical_parms = np.array([ipd, fov, K, Eqn, scale, model]);
        return optical_parms;

    if model == 4: # HMD_PM_GALAXY
        ipd = 0.062;
        fov = 90.0;
        Eqn = DistortionFn_CatmullRom10;
        scale = 0.039;
        K = np.array([[1.0],[1.022],[1.049],[1.081],[1.117],[1.159],[1.204],[1.258],[1.32],[1.39],[1.47]]);
        optical_parms = np.array([ipd, fov, K, Eqn, scale, model]);
        return optical_parms;

    if model == 5:
        ipd = 0.062;
        fov = 90.0;
        Eqn = DistortionFn_CatmullRom10;
        scale = 0.037;
        K = np.array([[ 1.0],[ 1.021],[ 1.051],[ 1.086],[ 1.128],[ 1.177],[ 1.232],[ 1.295],[ 1.368],[ 1.452],[ 1.560]]);
        optical_parms = np.array([ipd, fov, K, Eqn, scale, model]);
        return optical_parms;

    if model == 6: # HMD_PM_GALAXY_WQHD
        ipd = 0.062;
        fov = 90.0;
        Eqn = DistortionFn_CatmullRom10;
        scale = 0.039;
        K = np.array([[ 0.039],[ 1.0],  [ 1.022],[ 1.049],[ 1.081],[ 1.117],[ 1.159],[ 1.204],[ 1.258],[ 1.32], [ 1.39], [ 1.47]]);
        optical_parms = np.array([ipd, fov, K, Eqn, scale, model]);
        return optical_parms;

    if model == 7:# HMD_QM_GALAXY_WQHD
        ipd = 0.062;
        fov = 90.0;
        Eqn = DistortionFn_CatmullRom10;
        scale = 0.037;
        K = np.array([[ 1.0], [ 1.021],[ 1.051],[ 1.086],[ 1.128],[ 1.177],[ 1.232],[ 1.295],[ 1.368],[ 1.452],[ 1.560]]);
        optical_parms = np.array([ipd, fov, K, Eqn, scale, model]);
        return optical_parms;
    print("don't support model " + bytes(model));
    return;

def GetModelMesh(model, modelscreeninfo, tx, ty):
    eye = 0;#left eye

    wp = modelscreeninfo[0];
    hp = modelscreeninfo[1];
    wm = modelscreeninfo[2];
    print(tx,ty);
    
    opticInfo = GetModelOpticInfo(model);#opticInfo=np.array([ipd, fov, K, Eqn, scale, model]);
    ipd = opticInfo[0];
    K = opticInfo[2];
    DisortionFn =opticInfo[3];
    lensMetersPerTanAngleAtCenter = opticInfo[4];

    ChromaticAberration = list([-0.006, 0., 0.014, 0.0]);
    aspect = wp*0.5 / hp;
    shift = ipd/2. - wm/4.;
    shiftview = 2 * aspect * shift / wm;

    if eye == 1:# right eye
        shiftview = -1 * shiftview;

    out = [];
    for y in range(0,ty+1):
        for x in range(0,tx+1):
            # 假设:
            #     ipd = wm / 2 ， 此时 shiftview = 0
            #     wp/2 = hp, 此时 aspect = 0
            # 简化之得到：
            #     xf = x/tx;
            #     yf = y/ty;
            # 实际上将坐标[0,..,32] 归一化到 [0,1]
            xf = shiftview + (x*1.)/tx * aspect + (1. - aspect) * 0.5;
            yf = (y*1.)/ty;
            # TODO: xf,yf 应该是输出blks使用的坐标，因为是反畸变是基于 [xf,yf] 做的，而不一定是 [x/32,y/32]
            #       这个暂时不予考虑

            # xf - 0.5, yf - 0.5 是指 pt(xf,yf) 到 中心点(0.5,0.5) 的距离。单位是归一化的 [0，1]
            # 然后，[0，1] 乘以 hp 转成 像素单位. TODO: 均乘以 hp 可能是一个 BUG
            # 最后乘以 wm/wp， 转成距离单位。 wm/wp = 每个像素的大小
            dx = (xf - 0.5) * hp * (wm /wp);# hp should be wp/2
            dy = (yf - 0.5) * hp * (wm /wp);

            tanx = dx / lensMetersPerTanAngleAtCenter;
            tany = dy / lensMetersPerTanAngleAtCenter;            
            rsq = tanx*tanx + tany*tany;
            scale = DisortionFn(K, rsq);

            scaleRed   = scale * ( 1. + ChromaticAberration[0] + rsq * ChromaticAberration[1] );
            scaleGreen = scale;
            scaleBlue  = scale * ( 1. + ChromaticAberration[2] + rsq * ChromaticAberration[3] );

            Rx = scaleRed * tanx;
            Ry = scaleRed * tany;

            Gx = scaleGreen * tanx;
            Gy = scaleGreen * tany;

            Bx = scaleBlue * tanx;
            By = scaleBlue * tany;

            #out.append(np.array([Rx,Ry, Gx,Gy, Bx,By, xf,yf]));
            out.append(np.array([Rx,Ry, Gx,Gy, Bx,By, tanx,tany]));
    return np.array(out).reshape((ty+1)*(tx+1),8);


def GetModelMesh_Per_Eye(model, modelscreeninfo, tx, ty, eye):
    wp = modelscreeninfo[0];
    hp = modelscreeninfo[1];
    wm = modelscreeninfo[2];
    print(tx,ty);
    
    opticInfo = GetModelOpticInfo(model);#opticInfo=np.array([ipd, fov, K, Eqn, scale, model]);
    ipd = opticInfo[0];
    K = opticInfo[2];
    DisortionFn =opticInfo[3];
    lensMetersPerTanAngleAtCenter = opticInfo[4];

    ChromaticAberration = list([-0.006, 0., 0.014, 0.0]);
    aspect = wp*0.5 / hp;
    shift = ipd/2. - wm/4.;
    shiftview = 2 * aspect * shift / wm; # 即: aspect * (shift / (wm/2)) ， aspect 是半屏宽高的比例关系。 可以假设 aspect = 1, 即屏幕实际宽高比 为 2:1

    if eye == 1:# right eye
        shiftview = -1 * shiftview;

    out = [];
    for y in range(0,ty+1):
        for x in range(0,tx+1):
            # 假设:
            #     ipd = wm / 2 ， 此时 shiftview = 0
            #     wp/2 = hp, 此时 aspect = 0
            # 简化之得到：
            #     xf = x/tx;
            #     yf = y/ty;
            # 实际上将坐标[0,..,32] 归一化到 [0,1]
            xf = shiftview + (x*1.)/tx * aspect + (1. - aspect) * 0.5;
            yf = (y*1.)/ty;
            # TODO: xf,yf 应该是输出blks使用的坐标，因为是反畸变是基于 [xf,yf] 做的，而不一定是 [x/32,y/32]
            #       这个暂时不予考虑

            # xf - 0.5, yf - 0.5 是指 pt(xf,yf) 到 中心点(0.5,0.5) 的距离。单位是归一化的 [0，1]
            # 然后，[0，1] 乘以 hp 转成 像素单位. TODO: 均乘以 hp 可能是一个 BUG
            # 最后乘以 wm/wp， 转成距离单位。 wm/wp = 每个像素的大小
            dx = (xf - 0.5) * hp * (wm /wp);
            dy = (yf - 0.5) * hp * (wm /wp);

            tanx = dx / lensMetersPerTanAngleAtCenter;
            tany = dy / lensMetersPerTanAngleAtCenter;            
            rsq = tanx*tanx + tany*tany;
            scale = DisortionFn(K, rsq);

            scaleRed   = scale * ( 1. + ChromaticAberration[0] + rsq * ChromaticAberration[1] );
            scaleGreen = scale;
            scaleBlue  = scale * ( 1. + ChromaticAberration[2] + rsq * ChromaticAberration[3] );

            Rx = scaleRed * tanx;
            Ry = scaleRed * tany;

            Gx = scaleGreen * tanx;
            Gy = scaleGreen * tany;

            Bx = scaleBlue * tanx;
            By = scaleBlue * tany;

            #out.append(np.array([Rx,Ry, Gx,Gy, Bx,By, xf,yf]));
            out.append(np.array([Rx,Ry, Gx,Gy, Bx,By, tanx,tany]));
    return np.array(out).reshape((ty+1)*(tx+1),8);
    
def _DebugChromaticAberration_GetModelMeshPerEye(model, modelscreeninfo, tx, ty, eye, ChromaticAberration):
    wp = modelscreeninfo[0];
    hp = modelscreeninfo[1];
    wm = modelscreeninfo[2];
    print(tx,ty);
    
    opticInfo = GetModelOpticInfo(model);#opticInfo=np.array([ipd, fov, K, Eqn, scale, model]);
    ipd = opticInfo[0];
    K   = opticInfo[2];
    DisortionFn =opticInfo[3];
    lensMetersPerTanAngleAtCenter = opticInfo[4];

    aspect = wp*0.5 / hp;
    shift = ipd/2. - wm/4.;
    shiftview = 2 * aspect * shift / wm;

    if eye == 1:# right eye
        shiftview = -1 * shiftview;

    out = [];
    for y in range(0,ty+1):
        for x in range(0,tx+1):
            xf = shiftview + (x*1.)/tx * aspect + (1. - aspect) * 0.5;
            yf = (y*1.)/ty;
            dx = (xf - 0.5) * hp * (wm /wp);# hp should be wp/2
            dy = (yf - 0.5) * hp * (wm /wp);

            tanx = dx / lensMetersPerTanAngleAtCenter;
            tany = dy / lensMetersPerTanAngleAtCenter;            
            rsq = tanx*tanx + tany*tany;
            scale = DisortionFn(K, rsq);

            scaleRed   = scale * ( 1. + ChromaticAberration[0] + rsq * ChromaticAberration[1] );
            scaleGreen = scale;
            scaleBlue  = scale * ( 1. + ChromaticAberration[2] + rsq * ChromaticAberration[3] );

            Rx = scaleRed * tanx;
            Ry = scaleRed * tany;

            Gx = scaleGreen * tanx;
            Gy = scaleGreen * tany;

            Bx = scaleBlue * tanx;
            By = scaleBlue * tany;

            out.append(np.array([Rx,Ry, Gx,Gy, Bx,By, tanx,tany]));
    return np.array(out).reshape((ty+1)*(tx+1),8);


def DEBUG_GetModelEyeMesh(model, modelscreeninfo, tx, ty, eye):
    wp,hp,wm = modelscreeninfo[0],modelscreeninfo[1],modelscreeninfo[2];
    print("DEBUG_GetModelEyeMesh: tx,ty=",tx,ty, "wp,hp=",wp,hp);

    opticInfo = GetModelOpticInfo(model);
    ipd,K,DisortionFn,lensMetersPerTanAngleAtCenter = opticInfo[0], opticInfo[2], opticInfo[3], opticInfo[4];

    ChromaticAberration = list([-0.006, 0., 0.014, 0.0]);
    aspect,shift = wp*0.5/hp, ipd/2. - wm/4.;
    shiftview = 2 * aspect * shift / wm;
    if eye == 1:#right eye
        shiftview = -1 * shiftview;
    print("DEBUG_GetModelEyeMesh: shiftview=",shiftview);
    print("DEBUG_GetModelEyeMesh: aspect, 1.-aspect=",aspect, 1.-aspect);

    out = [];
    for y in range(0,ty+1):
        for x in range(0,tx+1):
            # 将坐标[0,..,32] 归一化到 [0,1]
            xf = shiftview + (x*1.)/tx * aspect + (1. - aspect) * 0.5;# 将 x/tx 乘以 aspect，相当于转换到 y/32 的轴上。
            yf = (y*1.)/ty;
            dx = (xf - 0.5) * (hp*(wm /wp));# (hp*(wm /wp)) = hp个像素的长度， 所以 dx = xf 距离0.5所对应的实际像素距离。
            dy = (yf - 0.5) * (hp*(wm /wp));

            #xf = shiftview + (x*1.)/tx * aspect + (1. - aspect) * 0.5;
            #yf = (y*1.)/ty;
            #dx = (xf - 0.5) * (wp/2) * (wm /wp);
            #dy = (yf - 0.5) * hp * (wm /wp);

            #xf = (x*1.)/tx;
            #yf = (y*1.)/ty;
            #dx = (xf - 0.5) * (wp/2) * (wm /wp);
            #dy = (yf - 0.5) * hp * (wm /wp);

            tanx,tany = dx/lensMetersPerTanAngleAtCenter, dy/lensMetersPerTanAngleAtCenter;
            rsq = tanx*tanx + tany*tany;
            scale = DisortionFn(K, rsq);

            scaleRed   = scale * ( 1. + ChromaticAberration[0] + rsq * ChromaticAberration[1] );
            scaleGreen = scale;
            scaleBlue  = scale * ( 1. + ChromaticAberration[2] + rsq * ChromaticAberration[3] );

            Rx,Ry = scaleRed * tanx, scaleRed * tany;
            Gx,Gy = scaleGreen * tanx, scaleGreen * tany;
            Bx,By = scaleBlue * tanx, scaleBlue * tany;
            
            #if y == 0:
            #    print xf,yf,dx,dy,tanx,tany,rsq,scale,Gx,Gy;

            out.append(np.array([Rx,Ry, Gx,Gy, Bx,By, tanx,tany]));
    return np.array(out).reshape((ty+1)*(tx+1),8);