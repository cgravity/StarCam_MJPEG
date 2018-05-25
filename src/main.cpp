#include "mjpeg_grabber.h"
#include "camera.h"

#include <cstdio>
#include <cstdlib>
#include <vector>
#include <ctime>
using namespace std;

extern void display_amt(AM_MEDIA_TYPE*);


bool filter_all_cameras(IMoniker*, IPropertyBag*)
{
    return true;
}

bool filter_see3cam_130(IMoniker* pMoniker, IPropertyBag* pPropBag)
{
    VARIANT var;
    VariantInit(&var);
    
    HRESULT hr = pPropBag->Read(L"FriendlyName", &var, 0);
    
    if(FAILED(hr) || var.vt != VT_BSTR)
    {
        VariantClear(&var);
        return false;
    }
    
    bool keep = (wcscmp(var.bstrVal, L"See3CAM_130") == 0);
    
    VariantClear(&var);
    return keep;
}

// Finds see3cam devices and constructs Camera instances for them. 
// The Camera objects are appended to the results vector.
//
// The filter function provided must return true if the camera should be
// constructed, and false if it should be skipped. Use filter_all_cameras
// if you do not want to restrict the devices matched.
//
// Assumes COM has been initialized as a prerequisite.
// See: [2], [4], [5]
HRESULT find_cameras(std::vector<Camera*>& results, 
    bool (*filter)(IMoniker*, IPropertyBag*))
{
    ICreateDevEnum* pCreateDevEnum = NULL;
    IEnumMoniker* pVideoEnum = NULL;
    
    HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL,
        CLSCTX_INPROC_SERVER, IID_ICreateDevEnum,(void**)&pCreateDevEnum);
    
    if(FAILED(hr))
    {
        fprintf(stderr, "Failed to create SystemDeviceEnum\n");
        return hr;
    }
    
    
    hr = pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory,
        &pVideoEnum,0);
    
    if(FAILED(hr))
    {
        fprintf(stderr, "Failed to create enumerator for video devices\n");
        return hr;
    }
    
    
    // Iterate over all video input devices. A moniker is basically a "path"
    // to an object, which we can use to look up its properties and bind to
    // the instance.
    IMoniker* pMoniker = NULL;
    
    while(pVideoEnum->Next(1, &pMoniker, NULL) == S_OK)
    {
        IPropertyBag* pPropBag;
        hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void**)&pPropBag);
        
        if(FAILED(hr))
        {
            fprintf(stderr, "Warning: Failed to query properties of a camera\n");
            
            pMoniker->Release();
            pMoniker = NULL;
            continue;
        }
        
        bool keep = filter(pMoniker, pPropBag);
        
        if(keep)
        {
            Camera* c = new Camera();
            c->pMoniker = pMoniker;
            results.push_back(c);
            
            VARIANT var;
            VariantInit(&var);
            hr = pPropBag->Read(L"DevicePath", &var, 0);
            
            if(SUCCEEDED(hr) && var.vt == VT_BSTR)
            {
                c->device_path = SysAllocString(var.bstrVal);
            }
            
            VariantClear(&var);
        }
        else
        {
            // the enumerator calls AddRef() on the monikers it returns, 
            // according to [6], so we must release it now that we are done 
            // with it.
            pMoniker->Release();
            pMoniker = NULL;
        }
    }
    
    // clean up
    pVideoEnum->Release();
    pVideoEnum = NULL;
    
    pCreateDevEnum->Release();
    pCreateDevEnum = NULL;
    
    return S_OK;
}

int main()
{
    IFilterGraph2* filter_graph = NULL;
    IBaseFilter* camera_filter = NULL;
    IPin* capture_pin = NULL;
    IMediaControl* graph_control = NULL;
    IEnumPins* enum_pins = NULL;
    
    IBaseFilter* save_filter = NULL;
    IPin* save_pin = NULL;
    
    IEnumMediaTypes* enum_amt = NULL;
    AM_MEDIA_TYPE* pAmt = NULL;
    
    time_t start;
    time_t now;
    
    // Init COM
    // See: [0], [1]
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    int com_counter = 0;
    
    if(SUCCEEDED(hr))
        com_counter++;
    else
    {
        fprintf(stderr, "COM Init failed\n");
        return EXIT_FAILURE;
    }

    vector<Camera*> cameras;
    hr = find_cameras(cameras, filter_all_cameras);
    
    if(FAILED(hr))
    {
        fprintf(stderr, "find cameras error code: 0x%lX\n", hr);
    }
    
    if(cameras.size() == 0)
    {
        fprintf(stderr, "ERROR: No cameras detected\n");
        goto cleanup;
    }
    
    // construct the filter graph manager
    
    hr = CoCreateInstance(CLSID_FilterGraph, 0, CLSCTX_INPROC_SERVER, 
        IID_IFilterGraph2, (void**)&filter_graph);
    
    if(FAILED(hr))
    {
        fprintf(stderr, "Failed to create filter graph\n");
        goto cleanup;
    }
    
    
    hr = filter_graph->QueryInterface(IID_IMediaControl, 
        (void**)&graph_control);
    
    if(FAILED(hr))
    {
        fprintf(stderr, "Failed to access control interface of filter graph\n");
        goto cleanup;
    }
    
    hr = filter_graph->AddSourceFilterForMoniker(
        cameras[0]->pMoniker,
        NULL,
        L"Camera",
        &camera_filter);
    
    if(FAILED(hr))
    {
        fprintf(stderr, "Failed to add the camera to the filter graph\n");
        goto cleanup;
    }
    
    
    hr = camera_filter->EnumPins(&enum_pins);
    
    if(FAILED(hr))
    {
        fprintf(stderr, "Failed to enumerate pins\n");
        goto cleanup;
    }
    
    
    while(enum_pins->Next(1,&capture_pin,NULL) == S_OK)
    {
        PIN_INFO info;
        capture_pin->QueryPinInfo(&info);
        wprintf(L"PIN NAME: '%s'\n", info.achName);
        if(info.pFilter)
            info.pFilter->Release();
        
        if(wcscmp(info.achName, L"Capture") == 0)
            break;
        
        capture_pin->Release();
        capture_pin = NULL;
    }
    
    if(capture_pin == NULL)
    {
        fprintf(stderr, "Failed to find capture pin!\n");
        goto cleanup;
    }
    
    save_filter = new MJ_GrabberFilter();
    save_filter->AddRef();
    hr = save_filter->FindPin(L"input", &save_pin);
    
    if(FAILED(hr))
    {
        fprintf(stderr, "Failed to find save pin\n");
        goto cleanup;
    }
    
    
    hr = filter_graph->AddFilter(save_filter, L"MJ_GrabberFilter");
    
    if(FAILED(hr))
    {
        fprintf(stderr, "Failed to add grabber to filter graph\n");
        goto cleanup;
    }
    
    
    hr = capture_pin->EnumMediaTypes(&enum_amt);
    
    if(FAILED(hr))
    {
        fprintf(stderr, "Failed to enum media types\n");
        goto cleanup;
    }
    
    // find a usable media type
    while(enum_amt->Next(1, &pAmt, NULL) == S_OK)
    {
        fprintf(stderr, "--------------------------------------------\n");
        
        if( pAmt->majortype == MEDIATYPE_Video && 
            pAmt->subtype == MEDIASUBTYPE_MJPG &&
            ) //pAmt->lSampleSize == 39386880)
        {
            display_amt(pAmt);
            break;
        }
        
        //DeleteMediaType(pAmt);
    }
    
    //fprintf(stderr, "DEBUG: Stopped here\n");
    //return 0;
    
    /*
    AM_MEDIA_TYPE amt;
        memset(&amt, 0, sizeof(amt));
        
        amt.majortype = MEDIATYPE_Video;
        amt.subtype = MEDIASUBTYPE_MJPG;
        amt.bFixedSizeSamples = false;
        amt.bTemporalCompression = false;
        amt.lSampleSize = 0;
        amt.formattype = FORMAT_VideoInfo;
        amt.pUnk = NULL;
        amt.cbFormat = sizeof(VIDEOINFOHEADER);
        amt.pbFormat = (BYTE*)CoTaskMemAlloc(sizeof(VIDEOINFOHEADER));
    
    memset(amt.pbFormat, 0, sizeof(VIDEOINFOHEADER));
    {
        VIDEOINFOHEADER* vih = (VIDEOINFOHEADER*)amt.pbFormat;
        vih->rcSource.left   = 0;
        vih->rcSource.top    = 0;
        vih->rcSource.right  = 1920;
        vih->rcSource.bottom = 1080;
        
        vih->rcTarget.left   = 0;
        vih->rcTarget.top    = 0;
        vih->rcTarget.right  = 1920;
        vih->rcTarget.bottom = 1080;
        
        vih->dwBitRate = 0;
        vih->AvgTimePerFrame = 400000; // 1/25 second in nanoseconds, div 100
    }
    */
    
    
    //hr = filter_graph->ConnectDirect(capture_pin, save_pin, &amt);
    hr = filter_graph->ConnectDirect(capture_pin, save_pin, pAmt);
    
    if(FAILED(hr))
    {
        fprintf(stderr, "Failed to connect camera to grabber\n");
        fprintf(stderr, "REASON: %lx\n", (unsigned long)hr);
        goto cleanup;
    }
    
    hr = graph_control->Run();
    
    if(FAILED(hr))
    {
        fprintf(stderr, "Failed to run\n");
        fprintf(stderr, "REASON: %lx\n", (unsigned long)hr);
        goto cleanup;
    }
    
    start = time(0);
    fprintf(stderr, "running...\n");
    while(1)
    {
        now = time(0);
        if(now - start > 10)
            break;
    }
    
    graph_control->Stop();
    
cleanup:
    fprintf(stderr, "FIXME: Proper cleanup crashes; need to debug.\n");
    return 0;
    
    if(capture_pin)
    {
        filter_graph->Disconnect(capture_pin);
        capture_pin->Release();
        capture_pin = NULL;
    }
    
    if(save_pin)
    {
        save_pin->Release();
        save_pin = NULL;
    }
    
    if(save_filter)
    {
        filter_graph->RemoveFilter(save_filter);
        save_filter->Release();
        save_filter = NULL;
    }
    
    if(camera_filter)
    {
        filter_graph->RemoveFilter(camera_filter);
        camera_filter->Release();
        camera_filter = NULL;
    }
    
    if(graph_control)
    {
        graph_control->Release();
        graph_control = NULL;
    }
    
    if(filter_graph)
    {
        filter_graph->Release();
        filter_graph = NULL;
    }
        

    // FIXME: Remove this testing code
    for(size_t i = 0; i < cameras.size(); i++)
    {
        delete cameras[i];
    }
    cameras.clear();
    
    // Shutdown COM
    // See: [3]
    while(com_counter)
    {
        com_counter--;
        CoUninitialize();
    }
    
    return EXIT_SUCCESS;
}

