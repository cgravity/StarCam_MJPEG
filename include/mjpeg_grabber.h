#pragma once

#include <dshow.h>

class MJ_GrabberFilter;

// IEnumMediaTypes is an interface used to access the list of media types
// supported by a pin. In this case, we need to indicate that the MJ_InputPin
// can support receiving JPEG frames.
//
// IEnumMediaTypes has the following inheritance tree:
//
// IUnknown
//   IEnumMediaTypes

class MJ_PinMediaTypes : public IEnumMediaTypes
{
  public:
    MJ_PinMediaTypes() : ref_count(0), index(0) {}
    virtual ~MJ_PinMediaTypes() {}
    
    // IUnknown methods
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID* ppvObj);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();
    
    // IEnumMediaTypes methods
    STDMETHODIMP Clone(IEnumMediaTypes** ppEnum);
    STDMETHODIMP Next(
        ULONG cMediaTypes,
        AM_MEDIA_TYPE** ppMediaTypes,
        ULONG* pcFetched);
    STDMETHODIMP Reset();
    STDMETHODIMP Skip(ULONG cMediaTypes);
  
  private:
    ULONG ref_count;
    ULONG index;
};

#if 0
// Allocates frame buffers. Required to support IMemInputPin interface
// on MJ_InputPin class.
//
// IUnknown
//   IMemAllocator

class MJ_Allocator : public IMemAllocator
{
  public:
    MJ_Allocator();
    
    // IUnknown methods
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID* ppvObj);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();
    
    // IMemAllocator interface
    STDMETHODIMP Commit();
    STDMETHODIMP Decommit();
    STDMETHODIMP GetBuffer(
        IMediaSample** ppBuffer,
        REFERENCE_TIME *pStartTime,
        REFERENCE_TIME *pEndTime,
        DWORD dwFlags);    
    STDMETHODIMP GetProperties(ALLOCATOR_PROPERTIES* pProps);
    STDMETHODIMP ReleaseBuffer(IMediaSample* pBuffer);
    STDMETHODIMP SetProperties(
        ALLOCATOR_PROPERTIES* pRequest,
        ALLOCATOR_PROPERTIES* pActual);
        
  private:
    ALLOCATOR_PROPERTIES properties;
    bool properties_set;
    bool commited;
    
    ULONG ref_count;  
};
#endif


// Pins on a DirectShow filter must implement the IPin interface.
// The IMemInputPin interface allows for the push model to be used to
// communicate with the pin.
//
// IPin has the following inheritance tree:
//
// IUnknown
//   IPin


class MJ_InputPin : public IPin, public IMemInputPin
{
  public:
    explicit MJ_InputPin(MJ_GrabberFilter* f) : ref_count(0), filter(f),
        other_end(NULL) {}
    virtual ~MJ_InputPin() {}
    
    // IUnknown methods
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID* ppvObj);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();
    
    // IPin methods
    STDMETHODIMP BeginFlush();
    STDMETHODIMP Connect(IPin* pReceivePin, const AM_MEDIA_TYPE* pmt);
    STDMETHODIMP ConnectedTo(IPin** ppPin);
    STDMETHODIMP ConnectionMediaType(AM_MEDIA_TYPE* pmt);
    STDMETHODIMP Disconnect();
    STDMETHODIMP EndFlush();
    STDMETHODIMP EndOfStream();
    STDMETHODIMP EnumMediaTypes(IEnumMediaTypes** ppEnum);
    STDMETHODIMP NewSegment(
        REFERENCE_TIME tStart,
        REFERENCE_TIME tEnd,
        double dRate);
    STDMETHODIMP QueryAccept(const AM_MEDIA_TYPE* pmt);
    STDMETHODIMP QueryDirection(PIN_DIRECTION* pPinDir);
    STDMETHODIMP QueryId(LPWSTR* id);
    STDMETHODIMP QueryInternalConnections(
        IPin** apPin,
        ULONG* nPin) { return E_NOTIMPL; }
    STDMETHODIMP QueryPinInfo(PIN_INFO* pInfo);
    STDMETHODIMP ReceiveConnection(
        IPin* pConnector,
        const AM_MEDIA_TYPE* pmt);
    
    // IMemInputPin methods
    STDMETHODIMP GetAllocator(IMemAllocator** ppAllocator);
    STDMETHODIMP GetAllocatorRequirements(ALLOCATOR_PROPERTIES *pProps);
    STDMETHODIMP NotifyAllocator(IMemAllocator* pAllocator, BOOL bReadOnly);
    STDMETHODIMP Receive(IMediaSample* pSample);
    STDMETHODIMP ReceiveCanBlock();
    STDMETHODIMP ReceiveMultiple(IMediaSample** pSamples, long nSamples,
        long *nSamplesProcessed);
    
  private:
    ULONG ref_count;
    MJ_GrabberFilter* filter;
    IPin* other_end;
    //MJ_Allocator* allocator;
};

// Pin Enumerator for the DirectShow filter.
// 
// IUnknown
//   IEnumPins
class MJ_EnumPins : public IEnumPins
{
  public:
    explicit MJ_EnumPins(MJ_GrabberFilter* pFilter);
    virtual ~MJ_EnumPins() {}
    
    // IUnknown methods
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID* ppvObj);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();
    
    // IEnumPins methods
    STDMETHODIMP Clone(IEnumPins** ppEnum);
    STDMETHODIMP Next(
        ULONG cPins,
        IPin** ppPins,
        ULONG* pcFetched);
    STDMETHODIMP Reset();
    STDMETHODIMP Skip(ULONG cPins);
    
  private:
    ULONG ref_count;
    MJ_GrabberFilter* filter;
    ULONG index;
};


// All DirectShow filters must implement the IBaseFilter interface.
//
// IBaseFilter has the following inheritance tree:
//
// IUnknown
//   IPersist
//     IMediaFilter
//       IBaseFilter

class MJ_GrabberFilter : public IBaseFilter
{
    friend class MJ_EnumPins;
    
  public:
    MJ_GrabberFilter();
    virtual ~MJ_GrabberFilter() {}
    
    // IUnknown methods
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID* ppvObj);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();
    
    // IPersist methods
    STDMETHODIMP GetClassID(CLSID* pClassId);
    
    // IMediaFilter methods
    STDMETHODIMP GetState(
        DWORD dwMilliSecsTimeout,
        FILTER_STATE* state);
    STDMETHODIMP GetSyncSource(IReferenceClock** pClock);
    STDMETHODIMP Pause();
    STDMETHODIMP Run(REFERENCE_TIME tStart);
    STDMETHODIMP SetSyncSource(IReferenceClock* pClock);
    STDMETHODIMP Stop();
    
    // IBaseFilter methods
    STDMETHODIMP EnumPins(IEnumPins** ppEnum);
    STDMETHODIMP FindPin(
        LPCWSTR id,
        IPin** ppPin);
    STDMETHODIMP JoinFilterGraph(
        IFilterGraph* pGraph,
        LPCWSTR pName);
    STDMETHODIMP QueryFilterInfo(FILTER_INFO* pInfo);
    STDMETHODIMP QueryVendorInfo(LPWSTR* pVenderInfo) {
        return E_NOTIMPL;
    }
    
  private:
    ULONG ref_count;
    MJ_InputPin* input_pin;
    IFilterGraph* graph;
    LPCWSTR name;
};

// Reference URLs from MSDN:
//
// IUnknown - https://msdn.microsoft.com/en-us/library/windows/desktop/ms682521(v=vs.85).aspx
// IEnumMediaTypes - https://msdn.microsoft.com/en-us/library/windows/desktop/dd376600(v=vs.85).aspx
// IEnumPins - https://msdn.microsoft.com/en-us/library/windows/desktop/dd376610(v=vs.85).aspx
// IBaseFilter - https://msdn.microsoft.com/en-us/library/windows/desktop/dd389526(v=vs.85).aspx
// IMediaFilter - https://msdn.microsoft.com/en-us/library/windows/desktop/dd406916(v=vs.85).aspx
// IPersist - https://msdn.microsoft.com/en-us/library/windows/desktop/ms688695(v=vs.85).aspx
// IPin - https://msdn.microsoft.com/en-us/library/windows/desktop/dd390397(v=vs.85).aspx
// IFilterGraph - https://msdn.microsoft.com/en-us/library/windows/desktop/dd389989(v=vs.85).aspx


