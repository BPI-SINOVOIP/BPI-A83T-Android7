#include <stdlib.h>
#include <stdint.h>
#include <math.h>

static double GetMaxOfArray(double p[])//TODO: 求8个浮点数中的最大绝对值
{
        double tmp = fabs(p[0]);
        for(int i = 1; i < 8; i++){
                tmp = tmp < fabs(p[i]) ? tmp : fabs(p[i]);
        }
        return tmp;
}

static size_t GetShiftBits(double tmp)
{
        size_t flag = 0;
        if(tmp<1.0){
                flag = 0;
        }
        else if(tmp<2.0){
                flag = 1;
        }
        else if(tmp<4.0){
                flag = 2;
        }
        else if(tmp<8.0){
                flag = 3;
        }
        else if(tmp<16.0){
                flag = 4;
        }
        else if(tmp<32.0){
                flag = 5;
        }
        else if(tmp<64.0){
                flag = 6;
        }
        else if(tmp<128.0){
                flag = 7;
        }
        else if(tmp<256.0){
                flag = 8;
        }
        else if(tmp<512.0){
                flag = 9;
        }
        else if(tmp<1024.0){
                flag = 10;
        }
        else if(tmp<2048.0){
                flag = 11;
        }
        else{
                flag = 12;
        }
        return flag;
}

unsigned char GetShift(int ouw, int ouh)
{
    unsigned char shift = 0;
    if(ouw >= 0x800)
        shift = 2;
    else if(ouw >= 0x400)
        shift = 3;
    else if(ouw >= 0x200)
        shift = 4;
    else
        shift = 8;
    unsigned char tmpshift = shift;
    if(ouh >= 0x800)
        shift = 2;
    else if(ouh >= 0x400)
        shift = 3;
    else if(ouh >= 0x200)
        shift = 4;
    else
        shift = 8;
    if(shift < tmpshift)
        shift = tmpshift;
    return shift;
}

size_t ConvertFromFloat2Int(int wp, int hp, unsigned char shift, double floatCoeffients[], int32_t outputIntCoeffients[])
{
    double maxd = GetMaxOfArray(floatCoeffients);
    size_t flag = GetShiftBits(maxd);
    for(int i=0; i<8; i++)
    {
        double tmp = floatCoeffients[i];
        if(flag)
        {
            tmp = tmp/(1<<flag);
        }
        switch(i)
        {
            case 0:
            case 3:
            case 6:
                tmp = tmp*hp;
                break;
            default:
                tmp = tmp*wp;
                break;
        }
        outputIntCoeffients[i] = (int32_t)(tmp*(1<<shift));
    }
    return flag;
}
