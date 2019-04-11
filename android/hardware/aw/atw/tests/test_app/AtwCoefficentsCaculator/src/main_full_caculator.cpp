#include "atw_math.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdio.h>

#define MAX_INT32_ERROR_TOLERANCE (20)

extern int ReadInputBlksFromFile(const char *filename, int col, int row, double **out);
extern void GetOutputBlks(int col, int row, double **out);
extern void GetBlkCoef(const double ouBlk[], const double inBlk[], double coef[]);
extern unsigned char GetShift(int ouw, int ouh);
extern size_t ConvertFromFloat2Int(int wp, int hp, unsigned char shift, double floatCoeffients[], int32_t outputIntCoeffients[]);
extern bool ReadResultFromFile(const char *filename, void **out, size_t *outBytes);

void TestCoeffGet(int inw, int inh,
	int ouw, int ouh, int mode,
	int col, int row,// 列数和行数
	double *inBlks[], double *ouBlks,
	int32_t *coeff)// [r0,r1,r2,...r7 | g0,g1,g2,...g7 | b0,b1,b2,...b7 | k0,k1,k2,...,k7] * 4bytes * col * row
{
	int w = mode == 1 ? ouw / 2 : ouw;
	int h = mode == 2 ? ouh / 2 : ouh;
	unsigned char shift = GetShift(ouw, ouh);

	for (int ch = 0; ch < 3; ch++)
	{
		for (int y = 0; y < row; y++)
		{
			for (int x = 0; x < col; x++)
			{
				uint32_t blk_idx = y*col + x;
				double *in = inBlks[ch] + blk_idx * 8;
				double *ou = ouBlks + blk_idx * 8;

				double p[8];
				int32_t np[8];
				size_t flag = 0;
				GetBlkCoef(ou, in, p);
				flag = ConvertFromFloat2Int(w, h, shift, p, np);

				//write to red or green or blue.
				int32_t* blk_base = coeff + blk_idx * 32;
				int32_t* ch_base = blk_base + ch*8;//r = data+0, g = data+8, b = data+16
				memcpy(ch_base, np, sizeof(int32_t) * 8);

				//write k.
				int32_t* k_base = blk_base + 24;
				k_base[ch] = flag;
			}
		}
	}
}

static void save_coeff(const char *filename, int32_t *coeff, int col, int row)
{
	FILE *fp = fopen(filename, "w+");
	if (!fp)
	{
		printf("open %s for writing coeff failed \n", filename);
		return;
	}

	size_t size = 2 * col*row * 4 * 8 * 4;
	fwrite(coeff, size, 1, fp);
	fclose(fp);
	return;
}

void PrintBlkResult(int32_t* std, int32_t* cmp, int32_t* dif, const char* tag)
{
	int32_t* _array = NULL;

	_array = std;
	printf("std-[%s]:  %8d %8d %8d %8d, %8d %8d %8d %8d \n",
		tag,
		_array[0], _array[1], _array[2], _array[3],
		_array[4], _array[5], _array[6], _array[7]);

	_array = cmp;
	printf("cmp-[%s]:  %8d %8d %8d %8d, %8d %8d %8d %8d \n",
		tag,
		_array[0], _array[1], _array[2], _array[3],
		_array[4], _array[5], _array[6], _array[7]);

	_array = dif;
	printf("dif-[%s]:  %8d %8d %8d %8d, %8d %8d %8d %8d \n",
		tag,
		_array[0], _array[1], _array[2], _array[3],
		_array[4], _array[5], _array[6], _array[7]);
}

void BlockCoeffientsCompare(int32_t* stdResult, int32_t* cmpResult, int ix, int iy, int row, int col, bool show = true)
{
	//memory layout:
	//block[iy][ix] = |R_INT32[8], G_INT32[8], B_INT32[8], K_INT32[8]|
	int blk_idx = iy*col + ix;
	int32_t* std_blk_ptr = stdResult + blk_idx * 32;//each blk was int32_t[32]
	int32_t* cmp_blk_ptr = cmpResult + blk_idx * 32;

	int32_t max_error = 0;
	int32_t difference[32] = { 0 };
	for (int i = 0; i < 32; i++)
	{
		int std = *(std_blk_ptr + i);
		int cmp = *(cmp_blk_ptr + i);
		difference[i] = abs(cmp - std);
		if (difference[i] > max_error)
		{
			max_error = difference[i];
		}
	}

	printf("\nblock[%d][%d] in [%d][%d]: \n", iy, ix, row, col);
	if (max_error >= MAX_INT32_ERROR_TOLERANCE || show == true)
	{
		PrintBlkResult(std_blk_ptr + 0, cmp_blk_ptr + 0, &difference[0], "Red");
		PrintBlkResult(std_blk_ptr + 8, cmp_blk_ptr + 8, &difference[8], "Green");
		PrintBlkResult(std_blk_ptr + 16, cmp_blk_ptr + 16, &difference[16], "Blue");
		PrintBlkResult(std_blk_ptr + 24, cmp_blk_ptr + 24, &difference[24], "K");
	}
	return;
}

#include <string>
static std::string GetPath()
{
#ifdef _VISUAL_STUDIO_PROJECT_
	return "../../../AtwCoefficentsCaculator/";
#else //linux
	return "";
#endif
}

void TestSample() // 唐禹谱给的 result.bin, 作比较
{
	double *inBlks[2][3];//r,g,b
	double *ouBlks;
	ReadInputBlksFromFile((GetPath() + "data/left/r/12.txt").c_str(), 40, 40, &inBlks[0][0]);
	ReadInputBlksFromFile((GetPath() + "data/left/g/12.txt").c_str(), 40, 40, &inBlks[0][1]);
	ReadInputBlksFromFile((GetPath() + "data/left/b/12.txt").c_str(), 40, 40, &inBlks[0][2]);
	ReadInputBlksFromFile((GetPath() + "data/right/r/12.txt").c_str(), 40, 40, &inBlks[1][0]);
	ReadInputBlksFromFile((GetPath() + "data/right/g/12.txt").c_str(), 40, 40, &inBlks[1][1]);
	ReadInputBlksFromFile((GetPath() + "data/right/b/12.txt").c_str(), 40, 40, &inBlks[1][2]);
	GetOutputBlks(40, 40, &ouBlks);

	int inw = 1080;
	int inh = 960;
	int mode = 2;  // 0:double screen,1:single screen left right(not support), 2:single screen top-down
	int ouw = 1080;
	int ouh = 1920;
	int col = 40;
	int row = 40;
	int eye_i32_nums = col*row * 32;//each eye has int32_t[col*row*32];  32 = 4*8;
	size_t coeff_size = 8 * 4 * 4 * col*row * 2;// rgbk(8*4*4) * col(40) * row(40) * eyeNum(2)
	int32_t *coeff = (int32_t *)malloc(coeff_size);
	memset(coeff, 0, coeff_size);
	TestCoeffGet(inw, inh, ouw, ouh, mode, col, row, inBlks[0], ouBlks, coeff + eye_i32_nums * 0);//left
	TestCoeffGet(inw, inh, ouw, ouh, mode, col, row, inBlks[1], ouBlks, coeff + eye_i32_nums * 1);//right
	save_coeff((GetPath() + "data/coeff.bin").c_str(), coeff, col, row);

	//compare.
	int32_t *buffer;
	size_t size = 0;
	ReadResultFromFile((GetPath() + "data/result.bin").c_str(), (void**)&buffer, &size);

	int32_t* std_left = buffer + eye_i32_nums * 0;
	int32_t* cmp_left = coeff + eye_i32_nums * 0;
	for (int y = 0; y < row; y++)
	{
		for (int x = 0; x < col; x++)
		{
			BlockCoeffientsCompare(std_left, cmp_left, x, y, row, col, false);
		}
	}
	printf("\n Compare Left End \n");

	int32_t* std_right = buffer + eye_i32_nums * 1;
	int32_t* cmp_right = coeff + eye_i32_nums * 1;
	for (int y = 0; y < row; y++)
	{
		for (int x = 0; x < col; x++)
		{
			BlockCoeffientsCompare(std_right, cmp_right, x, y, row, col, false);
		}
	}
	printf("\n Compare Right End \n");
}

int main(void)
{
	TestSample();
	system("pause");
	return 0;
}
