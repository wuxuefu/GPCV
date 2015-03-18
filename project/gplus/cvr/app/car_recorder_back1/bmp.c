//ReadBitMap
//
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include "CloudDog.h"
#include <strings.h>

#define RGB565_MASK_RED                     0xF800
#define RGB565_MASK_GREEN                   0x07E0
#define RGB565_MASK_BLUE                    0x001F

static    BITMAPFILEHEADER   bitHead;
static    BITMAPINFOHEADER bitInfoHead;
static    FILE *pfile;
UINT32 *getBimpFileHeadInfo(void){return &bitInfoHead;}
UINT32 rgb565_2_rgb24(UINT16 rgb565)
{
    UINT8 rgb24[4]={0};
    UINT32 returnRgb;
     //extract RGB
     rgb24[2] = (rgb565 & RGB565_MASK_RED) >> 11;
     rgb24[1] = (rgb565 & RGB565_MASK_GREEN) >> 5;
     rgb24[0] = (rgb565 & RGB565_MASK_BLUE);

     //amplify the image
     rgb24[2] <<= 3;
     rgb24[1] <<= 2;
     rgb24[0] <<= 3;
    returnRgb = *((UINT32 *)rgb24);
    return returnRgb;
}
void showBmpHead(BITMAPFILEHEADER *pBmpHead)
{
    printf("\n------------------------------------------------------\n");
    printf("Bmp Head:\n");
    printf("File Size:%d\n", pBmpHead->bfSize);
    printf("reserve:%d\n", pBmpHead->bfReserved1);
    printf("reserve:%d\n", pBmpHead->bfReserved2);
    printf("bfOffBits:%d\n", pBmpHead->bfOffBits);
}


void showBmpInforHead(BITMAPINFOHEADER *pBmpInforHead)
{
    printf("Bmp Infor Head:\n");
    printf("struct biSize:%d\n", pBmpInforHead->biSize);
    printf("biWidth:%d\n", pBmpInforHead->biWidth);
    printf("biHeight:%d\n", abs(pBmpInforHead->biHeight));
    printf("biPlanes:%d\n", pBmpInforHead->biPlanes);
    printf("biBitCount:%d\n", pBmpInforHead->biBitCount);
    printf("biCompression:%d\n", pBmpInforHead->biCompression);
    printf("biSizeImage:%d\n", pBmpInforHead->biSizeImage);
    printf("biXPelsPerMeter:%d\n", pBmpInforHead->biXPelsPerMeter);
    printf("biYPelsPerMeter:%d\n", pBmpInforHead->biYPelsPerMeter);
    printf("biClrUsed:%d\n", pBmpInforHead->biClrUsed);
    printf("biClrImportant:%d\n", pBmpInforHead->biClrImportant);
}

void *showRgbQuan(RGBQUAD *pRGB)
{
    printf("(%-3d,%-3d,%-3d)   ", pRGB->rgbRed, pRGB->rgbGreen, pRGB->rgbBlue);
    return NULL;
}
static RGBQUAD *loadPaletteInfo(void)
{
    RGBQUAD *pRgb;
    INT32 nPlantNum = (INT32)(pow(2, (double)(bitInfoHead.biBitCount)));   //   Mix color Plant Number;
    pRgb = (RGBQUAD *)malloc(nPlantNum * sizeof(RGBQUAD));
    memset(pRgb, 0, nPlantNum * sizeof(RGBQUAD));
    int num = fread(pRgb, 4, nPlantNum, pfile);

    printf("Color Plate Number: %d\n", nPlantNum);
#if 0
    printf("Palette Info:\n");
    int i;
    for (i = 0; i < nPlantNum; i++)
    {
        if (i % 5 == 0)
        {
            printf("\n");
        }
        showRgbQuan(&pRgb[i]);

    }

    printf("\n");
#endif
    return pRgb;
}

static void load1BitSubroutine(LOAD_BMPBIT_STR *bmpInfo)
{
    int k;
    int index = 0;
    int i, j;
    int width = bmpInfo->width;
    int height= bmpInfo->height;
    int l_width = bmpInfo->dataSize;
    UINT8 *pColorData = bmpInfo->pColorData;
    RGBQUAD *dataOfBmp = bmpInfo->dataOfBmp;
    RGBQUAD *pRgb = bmpInfo->pRgb; 
    for(i = 0; i < height; i++)
        for(j = 0; j < width; j++)
        {
            UINT8 mixIndex = 0;
            k = i * l_width + j / 8; //k:取得该像素颜色数据在实际数据数组中的序号
            //j:提取当前像素的颜色的具体值
            mixIndex = pColorData[k];
            switch(j % 8)
            {
            case 0:
                mixIndex = mixIndex << 7;
                mixIndex = mixIndex >> 7;
                break;
            case 1:
                mixIndex = mixIndex << 6;
                mixIndex = mixIndex >> 7;
                break;
            case 2:
                mixIndex = mixIndex << 5;
                mixIndex = mixIndex >> 7;
                break;

            case 3:
                mixIndex = mixIndex << 4;
                mixIndex = mixIndex >> 7;
                break;
            case 4:
                mixIndex = mixIndex << 3;
                mixIndex = mixIndex >> 7;
                break;

            case 5:
                mixIndex = mixIndex << 2;
                mixIndex = mixIndex >> 7;
                break;
            case 6:
                mixIndex = mixIndex << 1;
                mixIndex = mixIndex >> 7;
                break;

            case 7:
                mixIndex = mixIndex >> 7;
                break;
            }

            dataOfBmp[index].rgbRed = pRgb[mixIndex].rgbRed;
            dataOfBmp[index].rgbGreen = pRgb[mixIndex].rgbGreen;
            dataOfBmp[index].rgbBlue = pRgb[mixIndex].rgbBlue;
            dataOfBmp[index].rgbReserved = pRgb[mixIndex].rgbReserved;
            index++;

        }
}
static void load2BitSubroutine(LOAD_BMPBIT_STR *bmpInfo)
{
    int k;
    int index = 0;
    int i, j;
    int width = bmpInfo->width;
    int height= bmpInfo->height;
    int l_width = bmpInfo->dataSize;
    UINT8 *pColorData = bmpInfo->pColorData;
    RGBQUAD *dataOfBmp = bmpInfo->dataOfBmp;
    RGBQUAD *pRgb = bmpInfo->pRgb; 
    for( i = 0; i < height; i++)
        for( j = 0; j < width; j++)
        {
            UINT8 mixIndex = 0;
            k = i * l_width + j / 4; //k:取得该像素颜色数据在实际数据数组中的序号
            //j:提取当前像素的颜色的具体值
            mixIndex = pColorData[k];
            switch(j % 4)
            {
            case 0:
                mixIndex = mixIndex << 6;
                mixIndex = mixIndex >> 6;
                break;
            case 1:
                mixIndex = mixIndex << 4;
                mixIndex = mixIndex >> 6;
                break;
            case 2:
                mixIndex = mixIndex << 2;
                mixIndex = mixIndex >> 6;
                break;
            case 3:
                mixIndex = mixIndex >> 6;
                break;
            }
            //Save the pixel data to the corresponding position in the array
            dataOfBmp[index].rgbRed = pRgb[mixIndex].rgbRed;
            dataOfBmp[index].rgbGreen = pRgb[mixIndex].rgbGreen;
            dataOfBmp[index].rgbBlue = pRgb[mixIndex].rgbBlue;
            dataOfBmp[index].rgbReserved = pRgb[mixIndex].rgbReserved;
            index++;


        }
}
static void load4BitSubroutine(LOAD_BMPBIT_STR *bmpInfo)
{
    int k;
    int index = 0;
    int i, j;
    int width = bmpInfo->width;
    int height= bmpInfo->height;
    int l_width = bmpInfo->dataSize;
    UINT8 *pColorData = bmpInfo->pColorData;
    RGBQUAD *dataOfBmp = bmpInfo->dataOfBmp;
    RGBQUAD *pRgb = bmpInfo->pRgb;
    for( i = 0; i < height; i++)
        for( j = 0; j < width; j++)
        {
            UINT8 mixIndex = 0;
            k = i * l_width + j / 2;
            mixIndex = pColorData[k];
            if(j % 2 == 0)
            {
                //low
                mixIndex = mixIndex << 4;
                mixIndex = mixIndex >> 4;
            }
            else
            {
                //high
                mixIndex = mixIndex >> 4;
            }

            dataOfBmp[index].rgbRed = pRgb[mixIndex].rgbRed;
            dataOfBmp[index].rgbGreen = pRgb[mixIndex].rgbGreen;
            dataOfBmp[index].rgbBlue = pRgb[mixIndex].rgbBlue;
            dataOfBmp[index].rgbReserved = pRgb[mixIndex].rgbReserved;
            index++;

        }
}
static void load8BitSubroutine(LOAD_BMPBIT_STR *bmpInfo)
{
    int k;
    int index = 0;
    int i, j;
    int width = bmpInfo->width;
    int height= bmpInfo->height;
    int l_width = bmpInfo->dataSize;
    UINT8 *pColorData = bmpInfo->pColorData;
    RGBQUAD *dataOfBmp = bmpInfo->dataOfBmp;
    RGBQUAD *pRgb = bmpInfo->pRgb;
    for( i = 0; i < height; i++)
        for( j = 0; j < width; j++)
        {
            UINT8 mixIndex = 0;
            k = i * l_width + j;
            mixIndex = pColorData[k];
            dataOfBmp[index].rgbRed = pRgb[mixIndex].rgbRed;
            dataOfBmp[index].rgbGreen = pRgb[mixIndex].rgbGreen;
            dataOfBmp[index].rgbBlue = pRgb[mixIndex].rgbBlue;
            dataOfBmp[index].rgbReserved = pRgb[mixIndex].rgbReserved;
            index++;
        }
}
static void load16BitSubroutine(LOAD_BMPBIT_STR *bmpInfo)
{
    int k;
    int index = 0;
    int i, j;
    int width = bmpInfo->width;
    int height= bmpInfo->height;
    int l_width = bmpInfo->dataSize;
    UINT8 *pColorData = bmpInfo->pColorData;
    RGBQUAD *dataOfBmp = bmpInfo->dataOfBmp;
    RGBQUAD *pRgb = bmpInfo->pRgb;
    for( i = 0; i < height; i++)
        for( j = 0; j < width; j++)
        {
            UINT16 mixIndex = 0;

            k = i * l_width + j * 2;
            UINT16 shortTemp;
            shortTemp = pColorData[k + 1];
            shortTemp = shortTemp << 8;

            mixIndex = pColorData[k] + shortTemp;

            dataOfBmp[index].rgbRed = pRgb[mixIndex].rgbRed;
            dataOfBmp[index].rgbGreen = pRgb[mixIndex].rgbGreen;
            dataOfBmp[index].rgbBlue = pRgb[mixIndex].rgbBlue;
            dataOfBmp[index].rgbReserved = pRgb[mixIndex].rgbReserved;
            index++;
        }
}
// 
static void *ConvertedDispbuf(LOAD_BMPBIT_STR *bmpInfo)
{
    int width = bmpInfo->width;
    int height= bmpInfo->height;
    RGBQUAD *dataOfBmp = bmpInfo->dataOfBmp;
    int i=0,w=0,h=0,dispBufSize=width * height*sizeof(UINT16)+1;
    static UINT16 *dispBuf = NULL;
    UINT16 currColor=0;
    dispBuf = (UINT16 *)malloc(dispBufSize);
    printf("dispBufAddress:0x%X\n",dispBuf);
    memset(dispBuf,0,dispBufSize);
        //lcd Displayed from the lower left corner
    int bufIndex = width*height,offsetWidth=0;

    for(h=1;h<=height;h++)
    {
        offsetWidth = h*width;
        for(w=width; w>0; w--)
        {
            i++;
            offsetWidth--;
            if(bitInfoHead.biHeight > 0)
            {
                currColor = RGB888_TO_RGB565(dataOfBmp[bufIndex-i].rgbRed<<16 | \
                                             dataOfBmp[bufIndex-i].rgbGreen<<8 | \
                                             dataOfBmp[bufIndex-i].rgbBlue);
            }
            else
            {
                currColor = RGB888_TO_RGB565(dataOfBmp[i].rgbRed<<16 | \
                                             dataOfBmp[i].rgbGreen<<8 | \
                                             dataOfBmp[i].rgbBlue);
            }

            switch(currColor)
            {
            case 0x8c71:
                    currColor = 0x8450;
                    //printf("当前为透明色\n");
                    break;
             case RGB888_TO_RGB565(0xc0c0c0):
             case RGB888_TO_RGB565(0xff6b08):
             case RGB888_TO_RGB565(0xffda10):
                break;
            default: 
                //printf("currColor:0x%X RGB:0x%X \n",currColor,rgb565_2_rgb24(currColor));
                break;
            }

            if(bitInfoHead.biHeight > 0)
              *(dispBuf+offsetWidth) = currColor;
            else
              *(dispBuf+i) = currColor;

        #if 0

       *(dispBuf+i) = RGB888_TO_RGB565(dataOfBmp[bufIndex-i].rgbRed<<16 | \
                                         dataOfBmp[bufIndex-i].rgbGreen<<8 | \
                                         dataOfBmp[bufIndex-i].rgbBlue);

         if (i % 5 == 0)
        {
            printf("\n");
        }
        showRgbQuan(&dataOfBmp[i]);
        #endif
        }
     }


    printf("----------------return \n");
     return (void *)dispBuf;
}
void *openBmpFile(char *strFile)
{
    LOAD_BMPBIT_STR bmpInfo={0};

    pfile = fopen(strFile, "rb"); 
    if(pfile != NULL)
    {
        UINT16 fileType;
        fread(&fileType, 1, sizeof(UINT16), pfile);
        if(fileType != 0x4d42)
        {
            printf("file is not .bmp file!\n");
            return NULL;
        }
        fread(&bitHead, 1, sizeof(BITMAPFILEHEADER), pfile);

        showBmpHead(&bitHead);

        fread(&bitInfoHead, 1, sizeof(BITMAPINFOHEADER), pfile);
        showBmpInforHead(&bitInfoHead);
        printf("\n");
    }
    else
    {
        printf("file open fail! %s\n",strFile);
        return NULL;
    }
    if(bitInfoHead.biBitCount < 24)//Palette
    {
       bmpInfo.pRgb = loadPaletteInfo();
    }

    printf("open BMP:%s\n",strFile);

    int width = bitInfoHead.biWidth;
    int height = abs(bitInfoHead.biHeight);
    int l_width   = WIDTHUINT8S(width * bitInfoHead.biBitCount); //计算位图的实际宽度并确保它为32的倍数
    bmpInfo.pColorData = (UINT8 *)malloc(height * l_width+1);
    memset(bmpInfo.pColorData, 0, height * l_width);
    INT32 nData = height * l_width;
    fread(bmpInfo.pColorData, 1, nData, pfile);
    fclose(pfile);
    //The bit map data into RGB data

    printf("width:%d height:%d l_width:%d pColorData:0x%X\n",width,height,l_width,bmpInfo.pColorData);

    bmpInfo.dataOfBmp = (RGBQUAD *)malloc(width * abs(height) * sizeof(RGBQUAD)*10); //用于保存各像素对应的RGB数据
    UINT8 *dataOfBmp_temp = (UINT8 *)bmpInfo.dataOfBmp;
    memset(bmpInfo.dataOfBmp, 0, width * abs(height) * sizeof(RGBQUAD));
    printf("bmp to rgb\n");
    bmpInfo.width = width;
    bmpInfo.height = abs(height);
    bmpInfo.dataSize = l_width;


    
    if(bitInfoHead.biBitCount < 24) //有调色板，即位图为非真彩色
    {
        if (bitInfoHead.biBitCount == 1)
            load1BitSubroutine(&bmpInfo);
        if(bitInfoHead.biBitCount == 2)
            load2BitSubroutine(&bmpInfo);
        if(bitInfoHead.biBitCount == 4)
            load4BitSubroutine(&bmpInfo);
        if(bitInfoHead.biBitCount == 8)
            load8BitSubroutine(&bmpInfo);
        if(bitInfoHead.biBitCount == 16)
            load16BitSubroutine(&bmpInfo);
    }
    else//The picture shows the 24 -bit True Color
    {
        int k;
        int index = 0;
        int i, j;
        for( i = 0; i < height; i++)
            for( j = 0; j < width; j++)
            {
                k = i * l_width + j * 3;
                bmpInfo.dataOfBmp[index].rgbRed = bmpInfo.pColorData[k + 2];
                bmpInfo.dataOfBmp[index].rgbGreen = bmpInfo.pColorData[k + 1];
                bmpInfo.dataOfBmp[index].rgbBlue = bmpInfo.pColorData[k];
                index++;
            }
    }

    static void *returnBuf=NULL;
    returnBuf = ConvertedDispbuf(&bmpInfo);
#if 1
    printf("==============%d=================\n",__LINE__);
    if (bitInfoHead.biBitCount < 24)
    {
        free(bmpInfo.pRgb);
    }
    printf("==============%d dataOfBmp:0x%X=================\n",__LINE__,dataOfBmp_temp);
    free(dataOfBmp_temp);
    printf("==============%d pColorData:0x%X=================\n",__LINE__,bmpInfo.pColorData);
    free(bmpInfo.pColorData);

#endif
    printf("return returnBuf:0x%X\n",returnBuf);
    return returnBuf;
}

