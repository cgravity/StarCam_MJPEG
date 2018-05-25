#include "mjpeg_grabber.h"
#include <iostream>
#include <cstring>
using namespace std;

// === MJ_PinMediaTypes ===

HRESULT MJ_PinMediaTypes::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
    if(ppvObj == NULL)
    {
        return E_POINTER;
    }
    
    if(riid == IID_IUnknown)
    {
        *ppvObj = (IUnknown*)this;
        AddRef();
        return S_OK;
    }
        
    if(riid == IID_IEnumMediaTypes)
    {
        *ppvObj = (IEnumMediaTypes*)this;
        AddRef();
        return S_OK;
    }
        
    *ppvObj = NULL;
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) MJ_PinMediaTypes::AddRef()
{
    ref_count++;
    return ref_count;
}

STDMETHODIMP_(ULONG) MJ_PinMediaTypes::Release()
{
    if(ref_count != 0)
        ref_count--;
    
    if(ref_count == 0)
    {
        delete this;
        return 0; // don't use 'return ref_count'; that'd be use-after-free!
    }
    
    return ref_count;
}

STDMETHODIMP MJ_PinMediaTypes::Clone(IEnumMediaTypes** ppEnum)
{
    if(ppEnum == NULL)
        return E_POINTER;
    
    MJ_PinMediaTypes* pmt = new MJ_PinMediaTypes();
    pmt->index = index;
    pmt->AddRef();
    
    *ppEnum = pmt;
    
    return S_OK;
}

STDMETHODIMP MJ_PinMediaTypes::Next(
        ULONG cMediaTypes,
        AM_MEDIA_TYPE** ppMediaTypes,
        ULONG* pcFetched)
{
    static AM_MEDIA_TYPE valid_types[1];
    static const int VALID_TYPE_COUNT = 1;
    static bool valid_types_init = false;
    
    if(!valid_types_init)
    {
        // initialize list of valid types on first call
        valid_types_init = true;
        
        // MJPEG allowed
        AM_MEDIA_TYPE& amt = valid_types[0];
        amt.majortype = MEDIATYPE_Video;
        amt.subtype = MEDIASUBTYPE_MJPG;
        amt.bFixedSizeSamples = false;
        amt.bTemporalCompression = false;
        amt.lSampleSize = 0;
        amt.formattype = FORMAT_None;
        amt.pUnk = NULL;
        amt.cbFormat = 0;
        amt.pbFormat = NULL;
    }
    
    if(ppMediaTypes == NULL)
        return E_POINTER;
    
    ULONG local_count = 0;
    if(pcFetched == NULL)
        pcFetched = &local_count;
    
    for(*pcFetched = 0; *pcFetched < cMediaTypes; (*pcFetched)++)
    {
        if(index >= VALID_TYPE_COUNT)
            return S_FALSE;
        
        // clone media type at current index for caller, and return a 
        // pointer to it through their provided pointer array -- according
        // to the documentation, this structure must be allocated with
        // CoTaskMemAlloc so that it can be freed correctly by the caller later.
        // If pbFormat was used, it must also be cloned.
        AM_MEDIA_TYPE* pAmt = (AM_MEDIA_TYPE*)CoTaskMemAlloc(
            sizeof(AM_MEDIA_TYPE));
        memcpy(pAmt, &valid_types[index], sizeof(AM_MEDIA_TYPE));
        if(valid_types[index].pbFormat != NULL)
        {
            pAmt->pbFormat = (BYTE*)CoTaskMemAlloc(valid_types[index].cbFormat);
            memcpy(pAmt->pbFormat, &valid_types[index].pbFormat,
                valid_types[index].cbFormat);
        }
        ppMediaTypes[*pcFetched] = pAmt;
        
        index++;
    }
    
    // if we got here, the for-loop did not yet try to copy from beyond the
    // end of the array -- implying that there may be more values for
    // the caller to enumerate. (If I'm reading the documentation correctly
    // though, if the number requested exactly matches the number of entries
    // that was remaining, we may return S_OK even though 0 additional entries
    // are left to be enumerated over -- we only return S_FALSE if the requested
    // number actually exceeds the number of entries left to enumerate...)
    return S_OK; 
}

STDMETHODIMP MJ_PinMediaTypes::Reset()
{
    index = 0;
    return S_OK;
}

STDMETHODIMP MJ_PinMediaTypes::Skip(ULONG cMediaTypes)
{
    index += cMediaTypes;
    
    if(index >= 1)
        return S_FALSE;
        
    return S_OK;
}

#if 0

// === MJ_Allocator ===
STDMETHODIMP MJ_Allocator::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
    if(ppvObj == NULL)
    {
        return E_POINTER;
    }
    
    if(riid == IID_IUnknown)
    {
        *ppvObj = (IUnknown*)this;
        AddRef();
        return S_OK;
    }
        
    if(riid == IID_IMemAllocator)
    {
        *ppvObj = (IMemAllocator*)this;
        AddRef();
        return S_OK;
    }
        
    *ppvObj = NULL;
    return E_NOINTERFACE;    
}

#endif

// === MJ_InputPin ===

STDMETHODIMP MJ_InputPin::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
    if(ppvObj == NULL)
    {
        return E_POINTER;
    }
    
    if(riid == IID_IUnknown)
    {
        *ppvObj = (IUnknown*)(IPin*)this;
        AddRef();
        return S_OK;
    }
        
    if(riid == IID_IPin)
    {
        *ppvObj = (IPin*)this;
        AddRef();
        return S_OK;
    }
    
    if(riid == IID_IMemInputPin)
    {
        *ppvObj = (IMemInputPin*)this;
        AddRef();
        return S_OK;
    }
        
    *ppvObj = NULL;
    return E_NOINTERFACE;    
}

STDMETHODIMP_(ULONG) MJ_InputPin::AddRef()
{
    ref_count++;
    return ref_count;
}

STDMETHODIMP_(ULONG) MJ_InputPin::Release()
{
    if(ref_count != 0)
        ref_count--;
    
    if(ref_count == 0)
    {
        delete this;
        return 0; // don't use 'return ref_count'; that'd be use-after-free!
    }
    
    return ref_count;
}


// ---- FIXME: E_NOTIMPL --------------------------------------
STDMETHODIMP MJ_InputPin::BeginFlush()
{
    return E_NOTIMPL;
}

STDMETHODIMP MJ_InputPin::Connect(IPin* pReceivePin, const AM_MEDIA_TYPE* pmt)
{
    // FIXME: Is this function actually ever used by an input pin?
    fprintf(stderr, "FIXME: Implement %s\n", __PRETTY_FUNCTION__);
    return E_NOTIMPL;
        
    #if 0
    if(other_end != NULL)
        return VFW_E_ALREADY_CONNTECTED;
    
    if(pmt != NULL)
    {
        if(pmt->majortype != MEDIATYPE_Video)
            return VFW_E_TYPE_NOT_ACCEPTED;
        
        if(pmt->subtype != MEDIASUBTYPE_MJPG && 
           pmt->subtype != GUID_NULL)
        {
            return VFW_E_TYPE_NOT_ACCEPTED;
        }
    }
    
    if(pReceivePin == NULL)
        return E_POINTER;
    
    AM_MEDIA_TYPE local_mt;
    local_mt.majortype = MEDIATYPE_Video;
    local_mt.subtype = MEDIASUBTYPE_MJPG;
    
    AM_MEDIA_TYPE* request_mt = (pmt? pmt : &local_mt);
    
    // This pin is an input pin only; if asked to connect to another input
    // pin, it should fail... but I'm not 100% sure this is the right error
    // code to use. (FIXME)
    PIN_DIRECTION dir;
    if(pReceivePin->QueryDirection(&dir) != S_OK)
    {
        // can't query direction. WTF?! FIXME: Check error code is correct
        return VFW_E_NO_TRANSPORT;
    }
    
    if(dir != PINDIR_OUTPUT)
        return VFW_E_NO_TRANSPORT; // FIXME: Confirm error code is correct
    
    #endif
}

STDMETHODIMP MJ_InputPin::ConnectedTo(IPin** ppPin)
{
    if(ppPin == NULL)
        return E_POINTER;
    
    *ppPin = other_end;
    (*ppPin)->AddRef();
    
    if(other_end == NULL)
        return VFW_E_NOT_CONNECTED;
    else
        return S_OK;
}

STDMETHODIMP MJ_InputPin::ConnectionMediaType(AM_MEDIA_TYPE* pmt)
{
    fprintf(stderr, "FIXME: Implement %s\n", __PRETTY_FUNCTION__);
    return E_NOTIMPL;
}

STDMETHODIMP MJ_InputPin::Disconnect()
{
    if(other_end)
        other_end->Release();
        
    other_end = NULL;
    return S_OK;
}

STDMETHODIMP MJ_InputPin::EndFlush()
{
    fprintf(stderr, "FIXME: Implement %s\n", __PRETTY_FUNCTION__);
    return E_NOTIMPL;
}

STDMETHODIMP MJ_InputPin::EndOfStream()
{
    fprintf(stderr, "FIXME: Implement %s\n", __PRETTY_FUNCTION__);
    return E_NOTIMPL;
}

STDMETHODIMP MJ_InputPin::EnumMediaTypes(IEnumMediaTypes** ppEnum)
{
    if(ppEnum == NULL)
        return E_POINTER;
    
    *ppEnum = new MJ_PinMediaTypes;
    (*ppEnum)->AddRef();
    return S_OK;
}

STDMETHODIMP MJ_InputPin::NewSegment(
    REFERENCE_TIME tStart,
    REFERENCE_TIME tEnd,
    double dRate)
{
    return E_NOTIMPL;
}

STDMETHODIMP MJ_InputPin::QueryAccept(const AM_MEDIA_TYPE* pmt)
{
    if(pmt == NULL)
        return S_FALSE;
    
    if(pmt->majortype == MEDIATYPE_Video && pmt->subtype == MEDIASUBTYPE_MJPG)
        return S_OK;
    else if(pmt->majortype == MEDIATYPE_Video && pmt->subtype == GUID_NULL)
        return S_OK;
    else
        return S_FALSE;
}

STDMETHODIMP MJ_InputPin::QueryDirection(PIN_DIRECTION* pPinDir)
{
    if(pPinDir == NULL)
        return E_POINTER;
    
    *pPinDir = PINDIR_INPUT;
    return S_OK;
}

STDMETHODIMP MJ_InputPin::QueryId(LPWSTR* id)
{
    static const wchar_t name[] = L"input";
    
    if(id == NULL)
        return E_POINTER;
    
    *id = (LPWSTR)CoTaskMemAlloc(sizeof name);
    memcpy(*id, name, sizeof name);
        
    return S_OK;
}

STDMETHODIMP MJ_InputPin::QueryPinInfo(PIN_INFO* pInfo)
{
    if(pInfo == NULL)
        return E_POINTER;
    
    pInfo->pFilter = filter;
    pInfo->pFilter->AddRef();
    pInfo->dir = PINDIR_INPUT;
    wcscpy_s(pInfo->achName, MAX_PIN_NAME, L"input");
    
    return S_OK;
}

STDMETHODIMP MJ_InputPin::ReceiveConnection(
    IPin* pConnector,
    const AM_MEDIA_TYPE* pmt)
{
    if(pConnector == NULL)
        return E_POINTER;
        
    if(other_end == pConnector)
        return VFW_E_ALREADY_CONNECTED;
        
    if(pmt != NULL)
        if(QueryAccept(pmt) != S_OK)
            return VFW_E_TYPE_NOT_ACCEPTED;
        
    if(other_end != NULL)
    {
        fprintf(stderr, "WARNING: ReceiveConnection while other_end set!\n");
        other_end->Release();
    }
    
    other_end = pConnector;
    other_end->AddRef();
    return S_OK;
}

// --- MJ_InputPin IMemInputPin methods ---
STDMETHODIMP MJ_InputPin::GetAllocator(IMemAllocator** ppAllocator)
{
    return VFW_E_NO_ALLOCATOR;
}

STDMETHODIMP MJ_InputPin::GetAllocatorRequirements(ALLOCATOR_PROPERTIES *pProps)
{
    return E_NOTIMPL;
}

STDMETHODIMP MJ_InputPin::NotifyAllocator(IMemAllocator* pAllocator, BOOL bReadOnly)
{
    return S_OK; // FIXME: Save the allocator if we need it for some reason
}

extern void dump_file(const char* path, void* data, size_t size);

STDMETHODIMP MJ_InputPin::Receive(IMediaSample* pSample)
{
    if(pSample == NULL)
    {
        fprintf(stderr, "ERROR: pSample is NULL!\n");
        return E_POINTER;
    }
    
    fprintf(stderr, "Frame data!\n");
    
    BYTE* ptr = NULL;
    LONG length = 0;
    
    HRESULT hr = pSample->GetPointer(&ptr);
    if(FAILED(hr))
    {
        fprintf(stderr, "ERROR: pSample->GetPointer() failed?!\n");
        fprintf(stderr, "REASON: %lx\n", (unsigned long)hr);
        pSample->Release();
    }
    
    length = pSample->GetActualDataLength();
    
    dump_file("debug_frame.jpg", ptr, length);
    
    pSample->Release();
    exit(1);
    return S_OK;
}

STDMETHODIMP MJ_InputPin::ReceiveCanBlock()
{
    return S_FALSE; // won't block
}

STDMETHODIMP MJ_InputPin::ReceiveMultiple(IMediaSample** pSamples, long nSamples,
    long *nSamplesProcessed)
{
    if(!pSamples)
        return E_POINTER;
        
    for(long i = 0; i < nSamples; i++)
    {
        Receive(pSamples[i]);
    }
    
    if(nSamplesProcessed)
        *nSamplesProcessed = nSamples;
    
    return S_OK;
}


// === MJ_EnumPins ===
MJ_EnumPins::MJ_EnumPins(MJ_GrabberFilter* pFilter) : 
    ref_count(0), filter(pFilter), index(0)
{
    filter->AddRef();
}
        
STDMETHODIMP MJ_EnumPins::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
    if(ppvObj == NULL)
    {
        return E_POINTER;
    }
    
    if(riid == IID_IUnknown)
    {
        *ppvObj = (IUnknown*)this;
        AddRef();
        return S_OK;
    }
        
    if(riid == IID_IEnumPins)
    {
        *ppvObj = (IEnumPins*)this;
        AddRef();
        return S_OK;
    }
        
    *ppvObj = NULL;
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) MJ_EnumPins::AddRef()
{
    ref_count++;
    return ref_count;
}

STDMETHODIMP_(ULONG) MJ_EnumPins::Release()
{
    if(ref_count != 0)
        ref_count--;
    
    if(ref_count == 0)
    {
        filter->Release();
        filter = NULL;
        delete this;

        return 0; // don't use 'return ref_count'; that'd be use-after-free!
    }
    
    return ref_count;
}

STDMETHODIMP MJ_EnumPins::Clone(IEnumPins** ppEnum)
{
    if(ppEnum == NULL)
        return E_POINTER;
    
    MJ_EnumPins* result = new MJ_EnumPins(filter);
    result->index = index;
    result->AddRef();
    *ppEnum = result;
    
    return S_OK;
}

STDMETHODIMP MJ_EnumPins::MJ_EnumPins::Next(
    ULONG cPins,
    IPin** ppPins,
    ULONG* pcFetched)
{
    IPin** src = (IPin**)&filter->input_pin;
    ULONG count = 1;
    
    if(ppPins == NULL)
        return E_POINTER;
    
    ULONG local_counter = 0;
    if(pcFetched == NULL)
        pcFetched = &local_counter;
    
    for(*pcFetched = 0; *pcFetched < cPins; (*pcFetched)++)
    {
        if(index >= count)
            return S_FALSE;
        
        ppPins[*pcFetched] = src[index];
        ppPins[*pcFetched]->AddRef();
        index++;
    }
    
    return S_OK;
}

STDMETHODIMP MJ_EnumPins::Reset()
{
    index = 0;
    return S_OK;
}

STDMETHODIMP MJ_EnumPins::Skip(ULONG cPins)
{
    index += cPins;
    
    if(index >= 1)
        return S_FALSE;
        
    return S_OK;
}


// === MJ_GrabberFilter ===

MJ_GrabberFilter::MJ_GrabberFilter() : ref_count(0), input_pin(NULL), 
    graph(NULL), name(NULL)
{
    input_pin = new MJ_InputPin(this);
    input_pin->AddRef();
}


STDMETHODIMP MJ_GrabberFilter::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
    if(ppvObj == NULL)
    {
        return E_POINTER;
    }
    
    if(riid == IID_IUnknown)
    {
        *ppvObj = (IUnknown*)this;
        AddRef();
        return S_OK;
    }
    
    if(riid == IID_IPersist)
    {
        *ppvObj = (IPersist*)this;
        AddRef();
        return S_OK;
    }
        
    if(riid == IID_IMediaFilter)
    {
        *ppvObj = (IMediaFilter*)this;
        AddRef();
        return S_OK;
    }
    
    if(riid == IID_IBaseFilter)
    {
        *ppvObj = (IBaseFilter*)this;
        AddRef();
        return S_OK;
    }
    
    *ppvObj = NULL;
    return E_NOINTERFACE;    
}

STDMETHODIMP_(ULONG) MJ_GrabberFilter::AddRef()
{
    ref_count++;
    return ref_count;
}

STDMETHODIMP_(ULONG) MJ_GrabberFilter::Release()
{
    if(ref_count != 0)
        ref_count--;
    
    if(ref_count == 0)
    {
        input_pin->Release();
        delete this;
        return 0;
    }
    
    return ref_count;
}


STDMETHODIMP MJ_GrabberFilter::GetClassID(CLSID* pClassId)
{
    fprintf(stderr, "FIXME: Implement %s\n", __PRETTY_FUNCTION__);
    return E_NOTIMPL; // FIXME
}


// IMediaFilter methods
STDMETHODIMP MJ_GrabberFilter::GetState(
    DWORD dwMilliSecsTimeout,
    FILTER_STATE* state)
{
    fprintf(stderr, "FIXME: Implement %s\n", __PRETTY_FUNCTION__);
    return E_NOTIMPL; // FIXME
}

STDMETHODIMP MJ_GrabberFilter::GetSyncSource(IReferenceClock** pClock)
{
    fprintf(stderr, "FIXME: Implement %s\n", __PRETTY_FUNCTION__);
    return E_NOTIMPL; // FIXME
}

STDMETHODIMP MJ_GrabberFilter::Pause()
{
    fprintf(stderr, "FIXME: Implement %s\n", __PRETTY_FUNCTION__);
    return E_NOTIMPL; // FIXME
}

STDMETHODIMP MJ_GrabberFilter::Run(REFERENCE_TIME tStart)
{
    fprintf(stderr, "FIXME: Implement %s\n", __PRETTY_FUNCTION__);
    return E_NOTIMPL; // FIXME
}

STDMETHODIMP MJ_GrabberFilter::SetSyncSource(IReferenceClock* pClock)
{
    //fprintf(stderr, "FIXME: Implement %s\n", __PRETTY_FUNCTION__);
    //return E_NOTIMPL; // FIXME
    
    return S_OK;
}

STDMETHODIMP MJ_GrabberFilter::Stop()
{
    fprintf(stderr, "FIXME: Implement %s\n", __PRETTY_FUNCTION__);
    return E_NOTIMPL; // FIXME
}

STDMETHODIMP MJ_GrabberFilter::EnumPins(IEnumPins** ppEnum)
{
    if(ppEnum == NULL)
        return E_POINTER;
    
    IEnumPins* enumerator = new MJ_EnumPins(this);
    enumerator->AddRef();
    *ppEnum = enumerator;
    
    return S_OK;
}

STDMETHODIMP MJ_GrabberFilter::FindPin(
    LPCWSTR id,
    IPin** ppPin)
{
    LPWSTR input_pin_id;
    if(input_pin->QueryId(&input_pin_id) != S_OK)
        return E_FAIL;
    
    bool match = (wcscmp(input_pin_id, id) == 0);
    CoTaskMemFree(input_pin_id);
    
    if(match)
    {
        *ppPin = input_pin;
    }
    else
    {
        *ppPin = NULL;
    }
    
    return S_OK;
}

STDMETHODIMP MJ_GrabberFilter::JoinFilterGraph(
    IFilterGraph* pGraph,
    LPCWSTR pName)
{
    graph = pGraph;
    name = pName;
    return S_OK;
}

STDMETHODIMP MJ_GrabberFilter::QueryFilterInfo(FILTER_INFO* pInfo)
{
    if(pInfo == NULL)
        return E_POINTER;
    
    wcscpy_s(pInfo->achName, MAX_FILTER_NAME, name);
    pInfo->pGraph = graph;
    if(graph)
        pInfo->pGraph->AddRef();
    
    return S_OK;
}

