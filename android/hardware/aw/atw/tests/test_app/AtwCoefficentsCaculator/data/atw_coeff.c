#include "atw_coeff.h"

static double abs_matrix( double a ){
	double b;
	b = a > 0.0 ? a : -a;
	return b;
}

/* add yulangheng code */
void project_matrix( double *in_coordinate, double *out_coordinate, double *p ){

}
/*
基本流程:
0. 初始化系统信息。例如横屏 or 竖屏，单屏 or 双屏，是否是 global shutter(姿态补偿矩阵计算需要)，
1. 从文件或者memory中拿到畸变补偿网格, GetDistortionCompensationGrid(); TODO: 算法优化需要重新设计光学网格的数据结构，减少重复计算，色差校正，意味着要重复计算三次?
2. 根据timing info, 当前姿态，输入姿态，计算姿态补偿矩阵 matrix. CaclOrientationCompensationMatrix(inputPose, targetPose);
3. 针对基础光学网格，做姿态补偿。ApplyOrientationCompensation(matrix, disortionGrid);
4. 姿态补偿后的畸变补偿网格 + 输出网格信息， 计算硬件需要的系数(by cpu neon or Gpu)。
*/
int coeff_get( unsigned int inw, unsigned int inh, double *in_coordinate,
	       unsigned char mode, unsigned char m, unsigned char n,
	       unsigned char shift, double *out_coordinate, unsigned int outw,
	       unsigned int outh, int *coeff ){
	int i, w, h, len, flag;
	double tmp, div, p[8];

	w = (mode==1) ? (outw >> 1):outw;
	h = (mode==2) ? (outh >> 1):outh;

	cnt = m*n;
	cnt = mode ? (cnt << 1) : cnt;
	len = (cnt << 7);
	memset(coeff, 0, len);
	for(j = 0; j < cnt; j++){
		project_matrix(in_coordinate + (j<<3),
			       out_coordinate + (j << 3), p);

		for(i = 0; i < 8; i++){
			if(i==0){
				tmp = abs_matrix(p[i]);
			}
			tmp = tmp < abs_matrix(p[i]) ? tmp : abs_matrix(p[i]);
		}

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

		for(i = 0; i < 8; i++){
			tmp = p[i];
			if(flag){
				div = double(1<<flag);
				tmp = tmp/div;
			}
			switch(i){
				case 0:
				case 3:
				case 6:
					tmp = tmp*h;
					break;
				default:
					tmp = tmp*w;
					break;
			}
			*(coeff + bc + i) = tmp<<shift;
		}
		*(coeff + bc + 8) = flag;
		bc = bc + 24;
	}

	return 0;
}
