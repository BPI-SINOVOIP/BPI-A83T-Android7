#include "atw_math.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h> // for system
extern int ReadInputBlksFromFile(const char *filename, int col, int row, double **out);
extern void GetOutputBlks(int col, int row, double **out);
extern void GetBlkCoef(const double ouBlk[], const double inBlk[], double coef[]);

//计算指定 block 的系数
//inBlks: 40x40的 input blocks
//ouBlks: 40x40的 ouput blocks
//blk_x blk_y: 指定block在二维 (40,40) 中的坐标
void CaculatorFloatCoeffients40x40(double *inBlks, double *ouBlks, int blk_x, int blk_y)
{
	double *in = inBlks + (blk_y * 40 + blk_x) * 8;
	double *ou = ouBlks + (blk_y * 40 + blk_x) * 8;
	double p[8];
	GetBlkCoef(ou, in, p);
	printf("InputBlks: %f, %f, %f, %f, %f, %f, %f, %f \n",
		in[0], in[1], in[2], in[3],
		in[4], in[5], in[6], in[7]);
	printf("OuputBlks: %f, %f, %f, %f, %f, %f, %f, %f \n",
		ou[0], ou[1], ou[2], ou[3],
		ou[4], ou[5], ou[6], ou[7]);
	printf("Coeffients: %f, %f, %f, %f, %f, %f, %f, %f \n",
		p[0], p[1], p[2], p[3],
		p[4], p[5], p[6], p[7]);

	//TODO: 增加整数化流程。
	return;
}

int main(int argc, char **argv)
{
	const char* fpath = "transformed_blks.txt";//看工作目录的设置
	if (argv[1] != NULL)
	{
		fpath = argv[1];
	}
	printf("Do remap for %s, tx=ty=40 \n",fpath);

	double *inBlks = NULL;
	ReadInputBlksFromFile(fpath, 40, 40, &inBlks);
	if (inBlks == NULL)
	{
		printf("Invalid input blks filename %s \n", fpath);
		return -1;
	}
	double *ouBlks;
	GetOutputBlks(40, 40, &ouBlks);

	//TODO 参数化 blk_x 和 blk_y
	int blk_x = 0;
	int blk_y = 0;
	if(argv[2] != NULL && argv[3] != NULL)
	{
		int tmp_x = atoi(argv[2]);
		int tmp_y = atoi(argv[3]);
		if( tmp_x>=0 && tmp_x<40 )
		{
			printf("input %s for blk_x, get %d. \n", argv[2], tmp_x);
			blk_x = tmp_x;
		}
		if( tmp_y>=0 && tmp_y<40 )
		{
			printf("input %s for blk_y, get %d. \n", argv[3], tmp_y);
			blk_y = tmp_y;
		}
	}
	printf("handle: blk_x=%d, blk_y=%d , blk_idx=%d \n", blk_x, blk_y, (blk_y * 40 + blk_x));
	CaculatorFloatCoeffients40x40(inBlks, ouBlks, blk_x, blk_y);

	system("pause");
	return 0;
}
