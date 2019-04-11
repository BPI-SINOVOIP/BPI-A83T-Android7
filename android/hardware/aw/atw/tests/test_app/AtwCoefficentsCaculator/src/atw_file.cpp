#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int ReadInputBlksFromFile(const char *filename, int col, int row, double **out)
{
	if (out == NULL || filename == NULL) return -1;
	*out = NULL;

	FILE *fp = fopen(filename, "r");
	if (fp == NULL) { printf("openfile failed \n"); return -1; }

	int blkNum = col*row;
	double *buffer = (double*)malloc(blkNum * 8 * sizeof(double));
	int blkIdx = 0;
	while (!feof(fp))
	{
		char str[1024];
		const char *token = " ";
		if (fgets(str, 1024, fp) != NULL)
		{
			if (blkIdx >= blkNum)
			{
				printf("error file blkNum. \n");
				free(buffer);
				return -1;
			}
			double *line = buffer + blkIdx * 8;
			int lidx = 0;
			char *p = strtok(str, token);
			while (p)
			{
				if (lidx >= 8)
				{
					free(buffer);
					printf("error file \n");
					return -1;
				}
				line[lidx] = strtod(p, NULL);
				p = strtok(NULL, token);
				lidx++;
			}

			//printf("[%d]%s, %f, %f, %f, %f, %f, %f, %f, %f \n", blkIdx, str,
			//      line[0],line[1],line[2],line[3],line[4],line[5],line[6],line[7]);//DEBUG

			blkIdx++;
		}
	}

	fclose(fp);
	*out = buffer;
	return 0;
}

void GetOutputBlks(int col, int row, double **out)
{
	int blkNum = col*row;
	double *buffer = (double*)malloc(blkNum * 8 * sizeof(double));
	int blkIdx = 0;

	double xf = 1.0f / row;
	double yf = 1.0f / col;
	for (int y = 0; y < col; y++)
	{
		for (int x = 0; x < row; x++)
		{
			double blk[8];
			blk[0] = x*xf;
			blk[1] = y*yf;
			blk[2] = (x + 1)*xf;
			blk[3] = y*yf;
			blk[4] = (x + 1)*xf;//u4
			blk[5] = (y + 1)*yf;//v4
			blk[6] = x*xf;
			blk[7] = (y + 1)*yf;

			memcpy(buffer + blkIdx * 8, blk, sizeof(double) * 8);
			blkIdx++;
		}
	}

	*out = buffer;
}

bool ReadResultFromFile(const char *filename, void **out, size_t *outBytes)
{
	//SEEK_SET, SEEK_CUR, or SEEK_END
	FILE *fp = fopen(filename, "r");
	if (!fp)
	{
		printf("open %s failed \n", filename);
		return false;
	}

	size_t size = 0;
	size = ftell(fp);
	if (size != 0)
		printf("get first offset not zero ? \n");
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	void *buffer = malloc(size);
	if (!buffer) {
		printf("alloc buffer ReadResultFromFile failed \n");
		return false;
	}

	fread(buffer, size, 1, fp);
	fclose(fp);

	*out = buffer;
	*outBytes = size;
	return true;
}
