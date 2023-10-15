
#include "pch.h"
#include "framework.h"
#include "neaudiofix.h"
#include <cstdio>
#include <string>
#include <mutex>
#include <vector>
#include <algorithm>
#include <Psapi.h>

#include <Audioclient.h>
#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>

#include "ALCdevice.h"

#include "minhook/include/MinHook.h"

#define NeTrace(...) do { \
    printf("[NeAudioFix|%s;%d]: ", __FUNCTION__, __LINE__); \
    printf(__VA_ARGS__); \
    printf("\n"); fflush(stdout); \
} while (false)

#define NeExtern NEAUDIOFIX_EXTERN
#define NeApi NEAUDIOFIX_API
#define NeCall NEAUDIOFIX_CALL

// enum stuff:
NeComPtr<IMMDeviceEnumerator> g_pMMEnumerator;
UINT g_DevicesCount;
std::unique_ptr<NeComPtr<IMMDevice>[]> g_pDevices;

// device stuff:
bool g_bDeviceForced;
NeComPtr<IMMDevice> g_pMMCurrentDevice;
ALCcontext** ppAlutContext;
IAudioRenderClient** ppAudioRenderClient;
IAudioClient** ppAudioClient;
LPHANDLE pEventHandle;

// the table has 8 methods...
constexpr int GetDefaultAudioEndpointIndex = 4;

using GetDefaultAudioEndpoint_t =
HRESULT(STDMETHODCALLTYPE IMMDeviceEnumerator::*)(EDataFlow dataFlow, ERole role, IMMDevice** ppEndpoint);

GetDefaultAudioEndpoint_t GetDefaultAudioEndpointTramp;

bool NeInitOkay = false;

bool isNeCall = false;
bool NeResetDevice() {
    isNeCall = true;
    HRESULT hr = g_pMMEnumerator->GetDefaultAudioEndpoint(
        eRender,
        eConsole,
        g_pMMCurrentDevice.put_typed()
    );
    isNeCall = false;
    return SUCCEEDED(hr);
}

IMMDevice* NeGetDevice() {
    if (!g_pMMCurrentDevice) {
        NeResetDevice();
    }

    return g_pMMCurrentDevice.get();
}

void NeApplyToAL() {
    if (g_pMMCurrentDevice) {
        DWORD dwState = 0;
        HRESULT hr = g_pMMCurrentDevice->GetState(&dwState);

        if ((FAILED(hr) || dwState != DEVICE_STATE_ACTIVE) && !g_bDeviceForced) {
            // current device is down, and we did not force a device, try auto switch!
            NeResetDevice();
            hr = g_pMMCurrentDevice->GetState(&dwState);
            if (FAILED(hr) || dwState != DEVICE_STATE_ACTIVE) {
                // even the default device is DOWN!
                return;
            }
        }
    }
    else {
        // makes no sense to apply a null device to gamemaker context
        return;
    }

    if (ppAlutContext) {
        NeTrace("reapplying playback state");
        auto dev = (*ppAlutContext)->_pDevice;
        dev->stopPlayback();
        dev->closePlayback();
        auto thr = dev->m_pThread;
        DWORD dwStatus = WaitForSingleObject(thr->m_hThread, INFINITE);
        NeTrace("waitthread status res=%lu", dwStatus);
        BOOL bOk = CloseHandle(thr->m_hThread);
        NeTrace("closehandle thr res=%d", bOk);
        LeaveCriticalSection(thr->m_pTermMutex->criticalSection);
        thr->m_hThread = nullptr;
        thr->m_errorCode = 0;
        thr->m_bTerminate = false;
        thr->m_bRunning = false;
        thr->m_bPaused = false;
        thr->m_pFunctionArg = nullptr;
        thr->m_pThreadFunc = nullptr;
        dev->error = 0;
        while ((*ppAudioRenderClient)->Release() != 0);
        (*ppAudioRenderClient) = nullptr;
        while ((*ppAudioClient)->Release() != 0);
        (*ppAudioClient) = nullptr;
        bOk = CloseHandle(*pEventHandle);
        NeTrace("closehandle event res=%d", bOk);
        (*pEventHandle) = nullptr;
        dev->openPlayback("null");
        dev->resetPlayback();
        dev->startPlayback();
    }
}

void NeSetNewDevice(const NeComPtr<IMMDevice>& dev) {
    NeTrace("setting new device...");

    if (!dev) {
        NeTrace("using default device iptr...");
        NeResetDevice();
    }
    else {
        g_pMMCurrentDevice = dev;
    }

    NeApplyToAL();
    NeTrace("set complete");
}

std::string NeUtf16ToUtf8(LPCWSTR u16) {
    std::string s;
    if (u16 == nullptr) {
        return s;
    }

    size_t len = wcslen(u16);
    if (len <= 0) {
        return s;
    }

    int bytes = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, u16, static_cast<int>(len), nullptr, 0, nullptr, nullptr);
    if (bytes <= 0) {
        return s;
    }

    s.resize(bytes);
    WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, u16, static_cast<int>(len), s.data(), bytes, nullptr, nullptr);
    return s;
}

HRESULT
STDMETHODCALLTYPE
NeGetDefaultAudioEndpoint(IMMDeviceEnumerator* This, EDataFlow dataFlow, ERole role, IMMDevice** ppEndpoint) {
    if (!isNeCall && dataFlow == eRender && role == eConsole && ppEndpoint) {
        /* this is probably GameMaker... */
        auto dev = NeGetDevice();
        NeTrace("GameMaker is requesting an audio device ptr, %p", dev);
        *ppEndpoint = dev;
        return S_OK;
    }
    
    return (This->*GetDefaultAudioEndpointTramp)(dataFlow, role, ppEndpoint);
}

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);

constexpr unsigned char bany = '?';
bool predSearch(unsigned char a, unsigned char b) {
    return b == a || b == bany;
}

/* a little search helper: */
MODULEINFO modinfo{};
LPBYTE fastCodeSearch(const std::vector<unsigned char>& contents) {
    DWORD themask{ PAGE_EXECUTE_READ };
    LPBYTE address_low{ reinterpret_cast<LPBYTE>(modinfo.lpBaseOfDll) };
    LPBYTE address_high{ address_low + modinfo.SizeOfImage };
    MEMORY_BASIC_INFORMATION mbi{};

    while (address_low < address_high && VirtualQuery(address_low, &mbi, sizeof(mbi))) {
        if ((mbi.State == MEM_COMMIT) && (mbi.Protect & themask) && !(mbi.Protect & PAGE_GUARD)) {
            LPBYTE mbeg{ reinterpret_cast<LPBYTE>(mbi.BaseAddress) };
            LPBYTE mend{ mbeg + mbi.RegionSize };
            LPBYTE mres{ std::search(mbeg, mend, contents.begin(), contents.end(), predSearch) };

            if (mres != mend) {
                return mres;
            }
        }

        address_low += mbi.RegionSize;
        ZeroMemory(&mbi, sizeof(mbi));
    }

    return nullptr;
}

bool NeAudioFixInit(void) {
    NeTrace("early init...");

    if (g_pMMEnumerator) {
        NeTrace("already initialised!");
        return true;
    }

    BOOL bOk = GetModuleInformation(
        GetCurrentProcess(),
        GetModuleHandleW(nullptr),
        &modinfo,
        sizeof(modinfo)
    );
    if (!bOk) {
        return false;
    }

    // part of alThreadMixerProc
    auto p1 = fastCodeSearch({
        // 48 8b 0d ? ? ? ? 45 33 c0 ba d0 07 00 00 ff 15 c3 ? ? ? 48 8b 0d ? ? ? ? 48 8d 54 24 40 89 6c 24 40 48 8b 01 ff 50 30 85 c0 78 68 8b 1d ? ? ? ? 2b 5c 24 40 74 50 48 8b 0d ? ? ? ?
        0x48, 0x8b, 0x0d,
        bany, bany, bany, bany, // event handle
        0x45, 0x33, 0xc0,
        0xba, 0xd0, 0x07,
        0x00, 0x00,
        0xff, 0x15, bany, // WaitForSingleObject
        bany, bany, bany,
        0x48, 0x8b, 0x0d,
        bany, bany, bany, bany, // IAudioClient
        0x48, 0x8d, 0x54,
        0x24, 0x40,
        0x89, 0x6c, 0x24, 0x40,
        0x48, 0x8b, 0x01,
        0xff, 0x50, 0x30,
        0x85, 0xc0,
        0x78, 0x68,
        0x8b, 0x1d, bany, // BufferSize
        bany, bany, bany,
        0x2b, 0x5c, 0x24, 0x40,
        0x74, 0x50,
        0x48, 0x8b, 0x0d,
        bany, bany, bany, bany // IAudioRenderClient
    });
    if (!p1) {
        NeTrace("p1 search fail");
        return false;
    }
    // part of alutInit
    auto p2 = fastCodeSearch({
        // 48 89 3d ? ? ? ? b0 01 48 8b 7c 24 30 c7 05
        0x48, 0x89, 0x3d,
        bany, bany, bany, bany, // ALCcontext
        0xb0, 0x01, 0x48, 0x8b, 0x7c, 0x24, 0x30, 0xc7, 0x05
    });
    if (!p2) {
        NeTrace("p2 search fail");
        return false;
    }

    LPBYTE base = reinterpret_cast<LPBYTE>(modinfo.lpBaseOfDll);
    pEventHandle = reinterpret_cast<LPHANDLE>(p1 + 7 + *reinterpret_cast<uint32_t*>(p1 + 3));
    ppAudioClient = reinterpret_cast<IAudioClient**>(p1 + 28 + *reinterpret_cast<uint32_t*>(p1 + 24));
    ppAudioRenderClient = reinterpret_cast<IAudioRenderClient**>(p1 + 66 + *reinterpret_cast<uint32_t*>(p1 + 62));
    ppAlutContext = reinterpret_cast<ALCcontext**>(p2 + 7 + *reinterpret_cast<uint32_t*>(p2 + 3));
    
    HRESULT hr = CoInitialize(nullptr);
    if (FAILED(hr)) {
        NeTrace("coinitialize fail hr=%X", hr);
        return false;
    }

    hr = CoCreateInstance(
        CLSID_MMDeviceEnumerator,
        nullptr,
        CLSCTX_ALL,
        IID_IMMDeviceEnumerator,
        g_pMMEnumerator.put()
    );

    if (FAILED(hr)) {
        NeTrace("cocreateinstance immdeviceenumerator failed, hr=%X", hr);
        return false;
    }
    
    if (!NeResetDevice()) {
        NeTrace("failed to obtain the default device");
        return false;
    }

    MH_STATUS mh = MH_Initialize();
    if (mh != MH_OK) {
        NeTrace("MinHook Initialize error %s", MH_StatusToString(mh));
        return false;
    }

    mh = MH_CreateHookVirtual(
        g_pMMEnumerator.get(),
        GetDefaultAudioEndpointIndex,
        &NeGetDefaultAudioEndpoint,
        reinterpret_cast<LPVOID*>(&GetDefaultAudioEndpointTramp)
    );
    if (mh != MH_OK) {
        NeTrace("MinHook CreateHookVirtual error %s", MH_StatusToString(mh));
        return false;
    }

    mh = MH_EnableHook(MH_ALL_HOOKS);
    if (mh != MH_OK) {
        NeTrace("MinHook EnableHook error %s", MH_StatusToString(mh));
        return false;
    }
    
    NeTrace("done??");
    return 1;
}

NeExtern NeApi double NeCall neaudiofix_enum_devices(void) {
    //NeTrace("enum devices");
    if (!NeInitOkay || !g_pMMEnumerator) {
        return -1;
    }

    NeComPtr<IMMDeviceCollection> deviceCollection;
    HRESULT hr = g_pMMEnumerator->EnumAudioEndpoints(
        eRender,
        DEVICE_STATE_ACTIVE,
        deviceCollection.put_typed()
    );

    if (FAILED(hr)) {
        NeTrace("enumaudioendpoints fail hr=%X", hr);
        return -2;
    }

    UINT count = 0;
    hr = deviceCollection->GetCount(
        &count
    );

    if (FAILED(hr)) {
        NeTrace("getcount fail hr=%X", hr);
        return -3;
    }

    g_pDevices = std::make_unique<NeComPtr<IMMDevice>[]>(count);
    for (UINT idx = 0; idx < count; ++idx) {
        hr = deviceCollection->Item(
            idx,
            g_pDevices[idx].put_typed()
        );

        NeTrace("detected device %u %p", idx, g_pDevices[idx].get());

        if (FAILED(hr)) {
            NeTrace("failed to enum device %u hr=%X", idx, hr);
            // still carry on!!!
        }
    }

    g_DevicesCount = count;
    return count;
}

NeExtern NeApi const char* NeCall neaudiofix_enum_get_name(double deviceIndex) {
    //NeTrace("enum get name");

    if (!NeInitOkay || !g_pDevices) {
        return "";
    }

    UINT idx = static_cast<UINT>(deviceIndex);
    if (idx >= g_DevicesCount) {
        return "";
    }

    NeComPtr<IMMDevice> device = g_pDevices[idx];
    NeComPtr<IPropertyStore> propstore;

    HRESULT hr = device->OpenPropertyStore(
        STGM_READ,
        propstore.put_typed()
    );
    if (FAILED(hr)) {
        return "";
    }

    PROPVARIANT val;
    PropVariantInit(&val);
    hr = propstore->GetValue(
        PKEY_Device_FriendlyName,
        &val
    );
    if (FAILED(hr)) {
        PropVariantClear(&val);
        return "";
    }

    static std::string sDeviceName;
    sDeviceName = NeUtf16ToUtf8(val.pwszVal);
    PropVariantClear(&val);
    return sDeviceName.c_str();
}

NeExtern NeApi double NeCall neaudiofix_enum_devices_select(double deviceIndex) {
    NeTrace("enum devices select");
    if (!NeInitOkay) {
        return -3;
    }

    if (deviceIndex < 0) {
        g_bDeviceForced = false;
        NeSetNewDevice(NeComPtr<IMMDevice>());
        return 1;
    }

    if (!g_pDevices) {
        return -1;
    }

    UINT idx = static_cast<UINT>(deviceIndex);
    if (idx >= g_DevicesCount) {
        return -2;
    }

    g_bDeviceForced = true;
    NeSetNewDevice(g_pDevices[idx]);
    return 1;
}

NeExtern NeApi double NeCall neaudiofix_is_present(void) {
    NeTrace("is present");
    return NeInitOkay;
}

NeExtern NeApi double NeCall neaudiofix_periodic(void) {
    if (!NeInitOkay) {
        return 0;
    }

    if (ppAlutContext) {
        HANDLE hThread = (*ppAlutContext)->_pDevice->m_pThread->m_hThread;
        bool bIsRunning = hThread && WaitForSingleObject(hThread, 0) != WAIT_OBJECT_0;

        if (!bIsRunning) {
            NeTrace("playback thread has died, recovering...");
            NeApplyToAL();
        }
    }

    return 1;
}

NeExtern NeApi void NeCall RegisterCallbacks(void* p1, void* p2, void* p3, void* p4) {
    NeTrace("from inside RegisterCallbacks");
    NeInitOkay = NeAudioFixInit();
}

BOOL
APIENTRY
DllMain(HMODULE hModule, DWORD dwReasonForCall, LPVOID lpvReserved) {
    return TRUE;
}
