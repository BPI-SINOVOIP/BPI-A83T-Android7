#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "bmp.h"

bool SaveAsBmp(const void *pixels, const size_t width, const size_t height, const size_t bitsPerPixel, const char* fpath)
{
    if(!fpath)
    {
        printf("bad target path \n");
        return false;
    }

    if(!width || !height || !bitsPerPixel)
    {
        printf("can not save {w,h}={%zu,%zu}, bitsPerPixel=%zu to %s \n", width, height, bitsPerPixel, fpath);
        return false;
    }

    BITMAPFILEHEADER header;
    memset(&header, 0, sizeof(header));
    header.bfType = 0x4D42;
    header.bfSize = 54 + width * height * bitsPerPixel / 8;
    header.bfOffBits = 54;

    BITMAPINFOHEADER info;
    memset(&info, 0, sizeof(info));
    info.biSize = 40;
    info.biWidth = width;
    info.biHeight = height;
    info.biPlanes = 1;
    info.biBitCount = bitsPerPixel;

    FILE *fp = fopen(fpath, "wb");
    if(!fp)
    {
        printf("could not create %s \n", fpath);
        return false;
    }

    auto ret = fwrite(&header, 1, sizeof(header), fp);
    assert(ret == sizeof(header));

    ret = fwrite(&info, 1, sizeof(info), fp);
    assert(ret == sizeof(info));

    const size_t bytes = width * height * bitsPerPixel / 8;
    ret = fwrite(pixels, 1, bytes, fp);
    assert(ret == bytes);

    fclose(fp);
    printf("save {w,h}={%zu,%zu}, bitsPerPixel=%zu to %s success \n", width, height, bitsPerPixel, fpath);
    return true;
}

bool ReadAsBmp(void*& pixels, size_t &bytes, size_t &width, size_t &height, size_t &bitsPerPixel, const char *fpath)
{
    FILE* fp = fopen(fpath, "rb");
    if(!fp)
    {
        printf("open %s failed \n", fpath);
        return false;
    }

    char bmpHead[54];
    auto ret = fread(bmpHead, 1, sizeof(bmpHead), fp);
    if(ret != sizeof(bmpHead))
    {
        printf("read broken bmp file header . header size=%zu, expected(%zu)\n", ret, sizeof(bmpHead));
        fclose(fp);
        return false;
    }

    BITMAPFILEHEADER *header = (BITMAPFILEHEADER *)bmpHead;
    if(header->bfType != 0x4D42)
    {
        printf("bad head type(%hu) for %s \n", header->bfType, fpath);
        fclose(fp);
        return false;
    }
    if(header->bfOffBits != sizeof(bmpHead))
    {
        printf("bad offset[%d] read from file, expected(%zu) \n", header->bfOffBits, sizeof(bmpHead));
        fclose(fp);
        return false;
    }

    BITMAPINFOHEADER *info = (BITMAPINFOHEADER *)(&bmpHead[14]);
    if(info->biSize != 40)
    {
        printf("bad info size(%d) read from file. expected(40) \n", info->biSize);
        fclose(fp);
        return false;
    }

    bytes = info->biWidth * info->biHeight * info->biBitCount / 8;
    pixels = malloc(bytes);
	if(!pixels)
	{
		printf("malloc %zu bytes failed \n", bytes);
		fclose(fp);
		return false;
	}

    fseek(fp, header->bfOffBits, SEEK_SET);
    ret = fread(pixels, 1, bytes, fp);
    if(ret != bytes)
    {
        free(pixels);
        pixels = 0;
        bytes = 0;
        fclose(fp);
        return false;
    }
    width = info->biWidth;
    height = info->biHeight;
    bitsPerPixel = info->biBitCount;
    printf("read %s as bmp success. w=%zu, h=%zu, bitsPerPixel=%zu \n",fpath, width, height, bitsPerPixel);
    fclose(fp);
    return true;
}
