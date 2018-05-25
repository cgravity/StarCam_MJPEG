#pragma once

#include <dshow.h>

struct Camera
{
    IMoniker* pMoniker;
    BSTR device_path;
    
    Camera() : pMoniker(0), device_path(0) {}
    ~Camera();
};

