#include <cstdio>
#include <dshow.h>
#include <dvdmedia.h>
using namespace std;

typedef struct {
    const char* szName;
    GUID  guid;
} GUID_STRING_ENTRY;

GUID_STRING_ENTRY g_GuidNames[] = {
#define OUR_GUID_ENTRY(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
{ #name, { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } } },
    #include <uuids.h>
};

const char* find_guid_name(const GUID& guid)
{
    for(size_t i = 0; i < sizeof(g_GuidNames) / sizeof(g_GuidNames[0]); i++)
    {
        if(g_GuidNames[i].guid == guid)
            return g_GuidNames[i].szName;
    }
    
    if(guid == GUID_NULL)
        return "GUID_NULL";
    
    return "Unknown GUID";
}

const char* boolstr(bool b)
{
    return b?"true" : "false";
}

void print_rect(RECT r)
{
    fprintf(stderr, "  LEFT:   %ld\n", r.left);
    fprintf(stderr, "  TOP:    %ld\n", r.top);
    fprintf(stderr, "  RIGHT:  %ld\n", r.right);
    fprintf(stderr, "  BOTTOM: %ld\n", r.bottom);
}

void print_bmp(const BITMAPINFOHEADER* pbmi)
{
    fprintf(stderr, "  biSize = %ld\n", pbmi->biSize);
    fprintf(stderr, "  biWidth = %ld\n", pbmi->biWidth);
    fprintf(stderr, "  biHeight = %ld\n", pbmi->biHeight);
    fprintf(stderr, "  biPlanes = %d\n", pbmi->biPlanes);
    fprintf(stderr, "  biBitCount = %d\n", pbmi->biBitCount);
    fprintf(stderr, "  biCompression = %ld\n", pbmi->biCompression);
    fprintf(stderr, "  biSizeImage = %ld\n", pbmi->biSizeImage);
    fprintf(stderr, "  biXPelsPerMeter = %ld\n", pbmi->biXPelsPerMeter);
    fprintf(stderr, "  biClrUsed = %ld\n", pbmi->biClrUsed);
    fprintf(stderr, "  biClrImportant = %ld\n", pbmi->biClrImportant);
}

void display_amt(AM_MEDIA_TYPE* pAmt)
{
    fprintf(stderr, "majortype: %s\n", find_guid_name(pAmt->majortype));
    fprintf(stderr, "subtype: %s\n", find_guid_name(pAmt->subtype));
    fprintf(stderr, "fixed size samples: %s\n", boolstr(pAmt->bFixedSizeSamples));
    
    if(pAmt->bFixedSizeSamples)
    {
        fprintf(stderr, "Sample size: %ld\n", pAmt->lSampleSize);
    }
    
    fprintf(stderr, "temporal compression: %s\n", boolstr(pAmt->bTemporalCompression));
    
    
    if(pAmt->formattype == FORMAT_VideoInfo)
    {
        VIDEOINFOHEADER* pVideoInfo = (VIDEOINFOHEADER*)pAmt->pbFormat;
        
        fprintf(stderr, "SOURCE RECT\n");
        print_rect(pVideoInfo->rcSource);
        fprintf(stderr, "TARGET RECT\n");
        print_rect(pVideoInfo->rcTarget);
        
        fprintf(stderr, "BMP INFO\n");
        print_bmp((BITMAPINFOHEADER*)pAmt->pbFormat);
    }
    else if(pAmt->formattype == FORMAT_VideoInfo2)
    {
        VIDEOINFOHEADER2* pVideoInfo = (VIDEOINFOHEADER2*)pAmt->pbFormat;
        
        fprintf(stderr, "SOURCE RECT\n");
        print_rect(pVideoInfo->rcSource);
        fprintf(stderr, "TARGET RECT\n");
        print_rect(pVideoInfo->rcTarget);
        
        fprintf(stderr, "BMP INFO\n");
        print_bmp((BITMAPINFOHEADER*)pAmt->pbFormat);
        fprintf(stderr, "Aspect = %ld:%ld\n",
            pVideoInfo->dwPictAspectRatioX,
            pVideoInfo->dwPictAspectRatioY);
    }
    else
    {
        fprintf(stderr, "format type: %s\n", find_guid_name(pAmt->formattype));
        fprintf(stderr, "   ^^^ NO DECODER AVAILABLE! ^^^\n");
    }
    
}