#include "camera.h"

Camera::~Camera()
{
    if(pMoniker)
    {
        pMoniker->Release();
        pMoniker = NULL;
    }
    
    SysFreeString(device_path);
    device_path = NULL;
}

