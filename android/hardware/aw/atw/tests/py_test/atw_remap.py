# -*- coding: utf-8 -*-
import numpy as np

print("load atw_remap v1.21");
def proj_matrix(normalQuad, distQuad):
    x1 = distQuad[0];
    y1 = distQuad[1];

    x2 = distQuad[2];
    y2 = distQuad[3];

    x3 = distQuad[4];
    y3 = distQuad[5];

    x4 = distQuad[6];
    y4 = distQuad[7];

    u1 = normalQuad[0];
    v1 = normalQuad[1];

    u2 = normalQuad[2];
    v2 = normalQuad[3];

    u3 = normalQuad[4];
    v3 = normalQuad[5];

    u4 = normalQuad[6];
    v4 = normalQuad[7];

    A = np.matrix([
        u1, v1, 1, 0, 0, 0, -u1*x1, -v1*x1,
        u2, v2, 1, 0, 0, 0, -u2*x2, -v2*x2,
        u3, v3, 1, 0, 0, 0, -u3*x3, -v3*x3,
        u4, v4, 1, 0, 0, 0, -u4*x4, -v4*x4,
        0, 0, 0, u1, v1, 1, -u1*y1, -v1*y1,
        0, 0, 0, u2, v2, 1, -u2*y2, -v2*y2,
        0, 0, 0, u3, v3, 1, -u3*y3, -v3*y3,
        0, 0, 0, u4, v4, 1, -u4*y4, -v4*y4]).reshape(8,8);

    B = np.matrix([x1,x2,x3,x4,y1,y2,y3,y4]).reshape(8,1);   
    M = A.getI()*B;
    return M;

def debug_mono(coef, x,y):
    coef = coef*1024*8;
    a,b,c,d = int(coef[0]),int(coef[0]),int(coef[0]),int(coef[0]);
    e,f,g,h = int(coef[4]),int(coef[5]),int(coef[6]),int(coef[7]);

    q = g * x + h * y + 1024*1024*8;
    xx = a*x+b*y+c*1024;
    xx = (xx*256.0) / q;
    
    print x,xx;

# xy = [961,857];
# txty = [30,26];
def interpolation_distor_color(distMesh, normalMesh, scw, sch, eyew, eyeh, tx, ty):
    out = np.zeros(shape=(sch,scw,3), dtype=np.int16);# row x col, 行列式。
    nBlkH = sch / ty;
    nBlkW = scw / tx;

    if sch%ty!=0 or scw%tx !=0 :
        print "Warning ! remap find sch%ty!=0 or scw%tx!=0 " + ", scw = " + bytes(scw) + ", sch = " + bytes(sch) + ", tx = " + bytes(tx) + ", ty = " + bytes(ty);

    distMesh = np.array(distMesh).reshape(ty,tx,8);
    normalMesh = np.array(normalMesh).reshape(ty,tx,8);

    for y in range(0,ty):
        for x in range(0,tx):
            distQuad   = distMesh[y, x];
            normalQuad = normalMesh[y, x];

            distQuad = np.array(distQuad).reshape(8,1);
            normalQuad = np.array(normalQuad).reshape(8,1);
            coef  = proj_matrix(normalQuad, distQuad);

            for i in range(0,nBlkH):
                for j in range(0,nBlkW):
                    screen_x, screen_y = x*nBlkW+j, y*nBlkH+i;

                    #if screen_x == 961 and screen_y == 857:
                    #    print "Blks = " + bytes(y) + "," + bytes(x) + "]. details:";
                    #    print i,j;
                    #    print distQuad.reshape(1,8);
                    #    print normalQuad.reshape(1,8);
                    #    print coef.reshape(1,8);

                    scale_scx, scale_scy = (screen_x*1.0)/scw, (screen_y*1.0)/sch;
                    scale = coef[6] * scale_scx + coef[7] * scale_scy + 1.;
                    tex_x = (coef[0] * scale_scx + coef[1] * scale_scy + coef[2]) / scale;
                    tex_y = (coef[3] * scale_scx + coef[4] * scale_scy + coef[5]) / scale;
                    
                    #if (screen_x == 959 or screen_x == 960 or screen_x == 961) and screen_y == 857:
                    if (screen_x in range(955,965)) and screen_y == 857:
                        print screen_x,screen_y,tex_x, tex_y, tex_x * eyew;

                        
                        new_coe = list([0.006736,-0.315066,0.412333,-0.247849,-0.186038,0.564262,-0.342485,-0.551111]);
                        new_scale_scx, new_scale_scy = (screen_x*1.0)/scw, (screen_y*1.0)/sch;
                        new_scale = new_coe[6] * new_scale_scx + new_coe[7] * new_scale_scy + 1.;
                        new_tex_x = (new_coe[0] * new_scale_scx + new_coe[1] * new_scale_scy + new_coe[2]) / new_scale;
                        print 0.1111, screen_x, new_tex_x, new_tex_x*1024;

                    if (tex_x < 0.0) or (tex_y < 0.0) or (tex_x > 1.0) or (tex_y > 1.0):
                        out[screen_y, screen_x] = list([0,0,0]);                        
                    else:
                        px = int(tex_x * eyew);
                        py = int(tex_y * eyeh);
                        out[screen_y, screen_x] = list([px,py,1]);
    return out;


def DEBUG_Blk(nBlkW, nBlkH, scw,sch, eyew,eyeh, x,y, coef):#x,y 是指 blk 在 二维mesh 中的索引。
    for i in range(0,nBlkH): #i,j = 21,18;
        for j in range(0,nBlkW):
            screen_x, screen_y = x*nBlkW+j, y*nBlkH+i;
            scale_scx, scale_scy = (screen_x*1.0)/scw, (screen_y*1.0)/sch;
            scale = coef[6] * scale_scx + coef[7] * scale_scy + 1.;
            tex_x = (coef[0] * scale_scx + coef[1] * scale_scy + coef[2]) / scale;
            tex_y = (coef[3] * scale_scx + coef[4] * scale_scy + coef[5]) / scale;
            px = int(tex_x * eyew);
            py = int(tex_y * eyeh);
            print i,j, tex_x,tex_y, screen_x,screen_y, px,py;

def remap(warpMesh, normalMesh, scw, sch, eyew, eyeh, tx, ty):
    out = np.zeros(shape=(sch,scw,3), dtype=np.int16);# row x col, 行列式。
    nBlkH = sch / ty;
    nBlkW = scw / tx;

    if sch%ty!=0 or scw%tx !=0 :
        print "Warning ! remap find sch%ty!=0 or scw%tx!=0 " + ", scw = " + bytes(scw) + ", sch = " + bytes(sch) + ", tx = " + bytes(tx) + ", ty = " + bytes(ty);

    print "remap";
    print nBlkH,nBlkW, sch%ty, scw%tx, scw, sch, tx, ty;

    ybase = np.arange(0,nBlkH,1);
    xbase = np.arange(0,nBlkW,1);

    warpMesh = np.array(warpMesh).reshape(ty,tx,8);
    normalMesh = np.array(normalMesh).reshape(ty,tx,8);

    for y in range(0,ty):
        for x in range(0,tx):
            _in  = warpMesh[y, x];
            _ou  = normalMesh[y, x];
            coef = proj_matrix(_ou, _in);
            if y==0 and x==0:
                print _in;
                print _ou;
                print coef;

            screen_y  = ybase + y*nBlkH;
            scale_scy = (screen_y*1.0) / sch;            
            coef_y_7 = coef[7] * scale_scy;
            coef_y_1 = coef[1] * scale_scy;
            coef_y_4 = coef[4] * scale_scy;
            coef_y_7 = np.array(coef_y_7).reshape(nBlkH);
            coef_y_1 = np.array(coef_y_1).reshape(nBlkH);
            coef_y_4 = np.array(coef_y_4).reshape(nBlkH);

            screen_x = xbase + x*nBlkW;
            scale_scx = (screen_x*1.0) / scw;
            coef_x_6 = coef[6] * scale_scx;
            coef_x_0 = coef[0] * scale_scx;
            coef_x_3 = coef[3] * scale_scx;
            coef_x_6 = coef_x_6.reshape(nBlkW);
            coef_x_0 = coef_x_0.reshape(nBlkW);
            coef_x_3 = coef_x_3.reshape(nBlkW);

            for yy in range(0,nBlkH):
                cy7 = coef_y_7[yy];
                cy1 = coef_y_1[yy];
                cy4 = coef_y_4[yy];

                #算出每行所有点
                scale =  coef_x_6 + cy7 + 1.;
                tex_x = (coef_x_0 + cy1 + coef[2]) / scale;
                tex_y = (coef_x_3 + cy4 + coef[5]) / scale;

                tex_x = np.array(tex_x).reshape(nBlkW);
                tex_y = np.array(tex_y).reshape(nBlkW);
                for xx in range(0,nBlkW):
                    ttx = tex_x[xx];
                    tty = tex_y[xx];
                    scx = screen_x[xx];
                    scy  = screen_y[yy];

                    if (ttx < 0.0) or (tty < 0.0) or (ttx > 1.0) or (tty > 1.0):
                        out[scy, scx] = list([0,0,0]);
                    else:
                        px = ttx * eyew;
                        py = tty * eyeh;
                        out[scy, scx] = list([px,py,1]);
    return out;

def main(warpMesh, normalMesh, tx, ty, mapinfo):    
    scw = mapinfo[0];#输出区域的宽，也就是单眼所对应的屏幕
    sch = mapinfo[1];
    eyew = mapinfo[2];
    eyeh = mapinfo[3];

    data = remap(warpMesh, normalMesh, scw, sch, eyew, eyeh, tx, ty);
    data = np.array(data).reshape(sch,scw,3);

    speed_max = [];#统计加速度
    speed_max.append(0);
    for y in range(0,sch):
        line  = data[y,0:scw].T[1];
        speed_max.append(max(line));
    speed_max = np.array(speed_max).reshape(sch+1,1);    
    accel = speed_max[1:sch+1] - speed_max[0:sch];

    tmp = np.array(accel, dtype=np.int16).reshape(sch).tolist();
    cnt = [];
    for i in range(0,30):
        cnt.append(list([i, tmp.count(i)]));# 加速度值， 加速度值对应的行数。
    cnt = np.array(cnt).reshape(30,2);
    return list([data, accel, cnt]);

def _cacl_accelerations(data, eyew, eyeh, step):
    outh = data.shape[0];
    outw = data.shape[1];

    blksNum = eyew / step;
    rngs = [];
    for line in data[0:outh]:
        line = np.array(line).reshape(outw, 3).T;# outw 个 [tx,ty,0|1], 转置成 tx, ty, 0|1 三个数组。
        line[0] = (line[0] + step * line[2] - step) / step;# 对 tx 数组做处理， tx 转成 [-1,31]
        line = line.T;

        _range    = np.zeros(shape=([blksNum,3]), dtype=np.int16);
        for vert in line[0:outw]:#TODO: 如何快速根据 vert[0] = {-1,0,1,2,3,...,31} 分类出 33个数组，然后求各自数组的min和max，  而不要走这样的循环？
            blkindex = vert[0];
            if blkindex >= 0:
                _range[blkindex, 1] = max(_range[blkindex, 1], vert[1]);# _range[blkindex, 1] 的默认值是0
                if (_range[blkindex, 2] == 0):
                    _range[blkindex, 0] = vert[1];# _range[blkindex, 1] 的默认值是0。
                else:
                    _range[blkindex, 0] = min(_range[blkindex, 0], vert[1]);
                _range[blkindex, 2] = 1;#TAG

        rngs.append(_range);
    return np.array(rngs).reshape(outh, blksNum, 3);# outh 行， 每行是 blksNum 个{min,max}