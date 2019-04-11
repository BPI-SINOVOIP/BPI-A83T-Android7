// 如果要在 android apk 中使用，记得在 AndroidManifest.xml 文件中声明对 external_sdcard 的读写权限：
// <uses-permission android:name="android.permission.READ_EXTERNAL_STORAGE" />
// <uses-permission android:name="android.permission.WRITE_EXTERNAL_STORAGE" />

#ifndef _ALLWINNER_BMP_H_
#define _ALLWINNER_BMP_H_

#include <stdint.h>				// 为了跨平台的一致性，使用 std 标准 int
typedef int32_t			LONG; 	// windows 平台下， long 也是 4bytes, long long 才是 64bits
typedef int32_t			DWORD;
typedef int32_t			BOOL;
typedef unsigned char   BYTE;
typedef unsigned short	WORD;
typedef BYTE			*LPBYTE;
typedef DWORD			*LPDWORD;

#pragma pack(1)
typedef struct tagRGBQUAD
{
	BYTE	rgbBlue;
	BYTE	rgbGreen;
	BYTE	rgbRed;
	BYTE	rgbReserved;
}RGBQUAD;

typedef struct  tagBITMAPFILEHEADER
{
	WORD	bfType;				// 文件类型，必须是0x424D，即字符“BM”, 2bytes
	DWORD	bfSize;				// 文件大小, 4bytes
	WORD	bfReserved1;		// 保留字, 2bytes
	WORD	bfReserved2;		// 保留字, 2bytes
	DWORD	bfOffBits;			// 从文件头到实际位图数据的偏移字节数, 4bytes
	//共计 14 = 2 + 4 + 2 + 2 + 4 bytes
}BITMAPFILEHEADER;				// 位图文件头定义 

typedef struct tagBITMAPINFOHEADER
{
	// 共 40 bytes
	DWORD	biSize;				// 信息头大小
	LONG	biWidth;			// 图像宽度
	LONG	biHeight;			// 图像高度
	WORD	biPlanes;			// 位平面数，必须为1   
	WORD	biBitCount;			// 每像素位数: 1, 2, 4, 8, 16, 24, 32
	DWORD	biCompression;		// 压缩类型   
	DWORD	biSizeImage;		// 压缩图像大小字节数   
	LONG	biXPelsPerMeter;	// 水平分辨率   
	LONG	biYPelsPerMeter;	// 垂直分辨率   
	DWORD	biClrUsed;			// 位图实际用到的色彩数   
	DWORD	biClrImportant;		// 本位图中重要的色彩数   
}BITMAPINFOHEADER;				// 位图信息头定义   
#pragma pack()

bool SaveAsBmp(const void *pixels, const size_t width, const size_t height, const size_t bitsPerPixel, const char* fpath);
bool ReadAsBmp(void*& pixels, size_t &bytes, size_t &width, size_t &height, size_t &bitsPerPixel, const char *fpath);

#endif // _ALLWINNER_BMP_H_