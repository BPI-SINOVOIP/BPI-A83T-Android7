# -*- coding: utf-8 -*-
import sys,os,math
import numpy as np

def GetShiftBits(tmp):
    flag = 0;
    if(tmp<1.0):
        flag = 0;
    elif(tmp<2.0):
        flag = 1;
    elif(tmp<4.0):
        flag = 2;
    elif(tmp<8.0):
        flag = 3;
    elif(tmp<16.0):
        flag = 4;
    elif(tmp<32.0):
        flag = 5;
    elif(tmp<64.0):
        flag = 6;
    elif(tmp<128.0):
        flag = 7;
    elif(tmp<256.0):
        flag = 8;
    elif(tmp<512.0):
        flag = 9;
    elif(tmp<1024.0):
        flag = 10;
    elif(tmp<2048.0):
        flag = 11;
    else:
        flag = 12;
    return flag;

def GetShift(ouw, ouh):
    shift = 0;
    if(ouw >= 0x800):
        shift = 2;
    elif(ouw >= 0x400):
        shift = 3;
    elif(ouw >= 0x200):
        shift = 4;
    else:
        shift = 8;

    tmpshift = shift;
    if(ouh >= 0x800):
        shift = 2;
    elif(ouh >= 0x400):
        shift = 3;
    elif(ouh >= 0x200):
        shift = 4;
    else:
        shift = 8;

    if(shift < tmpshift):
        shift = tmpshift;
    return shift;

def ConvertFromFloat2Int(wp, hp, flag, shift, fdata, log):
    data = np.array(fdata);
    ret_data = np.arange(8, dtype='int32');
    for i in range(0,8):
        tmp = data[i];
        if flag > 0:
            tmp = tmp / (1<<flag);
        if i == 0 or i == 3 or i == 6:
            tmp = tmp * hp;
        else:
            tmp = tmp * wp;
        ret_data[i] = tmp * (1<<shift);
        if log == 1:
            print (hp, wp);
            print ("for: i=",i ,", int=" , ret_data[i] , ", float=", data[i] , ", tmp=" , tmp);
    return ret_data;
