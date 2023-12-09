#include "sandbox/common.hpp"
#include "sandbox/device/gpu_device_api.hpp"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <d3d12x.h>

#include "sandbox/device/gpu_device.hpp"
#include "sandbox/platform/platform.hpp"

const u8 d3d12_num_frames = 3;


struct D3D12Swapchain {
    IDXGISwapChain4*                swapchain{};
    bool                            tearing_supported{};
    DXGI_FORMAT                     swapchain_format{};
    ID3D12DescriptorHeap*           rtv_descriptor_heap{};
    u32                             rtv_descriptor_size{};
    ID3D12Resource*                 back_buffers[d3d12_num_frames]{};
    CD3DX12_CPU_DESCRIPTOR_HANDLE   target_descriptors[d3d12_num_frames]{};
};


struct D3D12Context {
    ID3D12Device2* device = nullptr;
    ID3D12CommandQueue* command_queue = nullptr;
    ID3D12CommandAllocator* command_allocators[d3d12_num_frames]{};
    ID3D12GraphicsCommandList* command_list = nullptr;
    ID3D12DescriptorHeap* srv_descriptor_heap = nullptr;

    ID3D12Fence* fence = nullptr;
    HANDLE fence_event = nullptr;
    u64 fence_value = 0;
    u64 frame_fence_values[d3d12_num_frames] = {};

    D3D_FEATURE_LEVEL minimum_feature_level = D3D_FEATURE_LEVEL_12_0;
} d3d12_content;


inline void throw_if_failed(HRESULT hr) {
    if (FAILED(hr)) {
        SNBX_ASSERT(false, "Error on D3D12");
    }
}


IDXGIAdapter1* d3d12_get_adapter() {
    IDXGIFactory4* factory;
    UINT create_factory_flags = 0;
#if defined(_DEBUG)
    create_factory_flags = DXGI_CREATE_FACTORY_DEBUG;
#endif
    throw_if_failed(CreateDXGIFactory2(create_factory_flags, IID_PPV_ARGS(&factory)));

    IDXGIAdapter1* ret_adapter = nullptr;
    IDXGIAdapter1* dxgi_adapter1;

    //choose device
    std::size_t max_dedicated_video_memory = 0;
    for (UINT i = 0; factory->EnumAdapters1(i, &dxgi_adapter1) != DXGI_ERROR_NOT_FOUND; ++i) {
        DXGI_ADAPTER_DESC1 dxgi_adapter_desc1;
        dxgi_adapter1->GetDesc1(&dxgi_adapter_desc1);

        // Check to see if the adapter can create a D3D12 device without actually
        // creating it. The adapter with the largest dedicated video memory
        // is favored.

        if ((dxgi_adapter_desc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 &&
            SUCCEEDED(D3D12CreateDevice(dxgi_adapter1, d3d12_content.minimum_feature_level, __uuidof(ID3D12Device), nullptr)) &&
            dxgi_adapter_desc1.DedicatedVideoMemory > max_dedicated_video_memory) {
            max_dedicated_video_memory = dxgi_adapter_desc1.DedicatedVideoMemory;
            ret_adapter = dxgi_adapter1;
        }
    }
    factory->Release();
    return ret_adapter;
}


void d3d12_create_device(IDXGIAdapter1* adapter) {
    throw_if_failed(D3D12CreateDevice(adapter, d3d12_content.minimum_feature_level, IID_PPV_ARGS(&d3d12_content.device)));

#if defined(SNBX_DEBUG)
    ID3D12InfoQueue* info_queue;
    d3d12_content.device->QueryInterface(IID_PPV_ARGS(&info_queue));
    if (info_queue) {
        info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
        info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
        info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

        // Suppress whole categories of messages
        //D3D12_MESSAGE_CATEGORY Categories[] = {};

        // Suppress messages based on their severity level
        D3D12_MESSAGE_SEVERITY severities[] =
        {
            D3D12_MESSAGE_SEVERITY_INFO
        };

        // Suppress individual messages by their ID
        D3D12_MESSAGE_ID deny_ids[] = {
            D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
            D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,
            D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,
        };

        D3D12_INFO_QUEUE_FILTER new_filter = {};
        //NewFilter.DenyList.NumCategories = _countof(Categories);
        //NewFilter.DenyList.pCategoryList = Categories;
        new_filter.DenyList.NumSeverities = _countof(severities);
        new_filter.DenyList.pSeverityList = severities;
        new_filter.DenyList.NumIDs = _countof(deny_ids);
        new_filter.DenyList.pIDList = deny_ids;

        throw_if_failed(info_queue->PushStorageFilter(&new_filter));
    }
#endif
}


void d3d12_device_init() {
#if defined(SNBX_DEBUG)
    ID3D12Debug1* dc;
    throw_if_failed(D3D12GetDebugInterface(IID_PPV_ARGS(&dc)));
    throw_if_failed(dc->QueryInterface(IID_PPV_ARGS(&dc)));
    dc->EnableDebugLayer();
    dc->SetEnableGPUBasedValidation(true);
    dc->Release();
    dc = nullptr;
#endif

    IDXGIAdapter1* adapter = d3d12_get_adapter();
    d3d12_create_device(adapter);
    adapter->Release();

    D3D12_COMMAND_QUEUE_DESC command_queue_desc = {};
    command_queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    command_queue_desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    command_queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    command_queue_desc.NodeMask = 0;
    throw_if_failed(d3d12_content.device->CreateCommandQueue(&command_queue_desc, IID_PPV_ARGS(&d3d12_content.command_queue)));

    for (int i = 0; i < d3d12_num_frames; ++i) {
        throw_if_failed(d3d12_content.device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&d3d12_content.command_allocators[i])));
    }

    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    desc.NumDescriptors = 1;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    throw_if_failed(d3d12_content.device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&d3d12_content.srv_descriptor_heap)));

    throw_if_failed(d3d12_content.device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, d3d12_content.command_allocators[0], nullptr, IID_PPV_ARGS(&d3d12_content.command_list)));
    throw_if_failed(d3d12_content.command_list->Close());

    throw_if_failed(d3d12_content.device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&d3d12_content.fence)));
    d3d12_content.fence_event = ::CreateEvent(nullptr, FALSE, FALSE, nullptr);
    SNBX_ASSERT(d3d12_content.fence_event, "Failed to create fence event.");

    spdlog::info("D3D12Device initialized successfully");
}

bool d3d12_is_tearing_supported() {
    IDXGIFactory4* factory;
    UINT create_factory_flags = 0;
#if defined(_DEBUG)
    create_factory_flags = DXGI_CREATE_FACTORY_DEBUG;
#endif
    throw_if_failed(CreateDXGIFactory2(create_factory_flags, IID_PPV_ARGS(&factory)));

    bool allow_tearing = false;
    IDXGIFactory5* factory5;
    factory->QueryInterface(IID_PPV_ARGS(&factory5));
    if (factory5) {
        if (FAILED(factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allow_tearing, sizeof(allow_tearing)))) {
            allow_tearing = false;
        }
        factory5->Release();
    }

    factory->Release();
    return allow_tearing;
}

GPUSwapchain d3d12_create_swapchain(const SwapChainCreation& swapchain_creation, Window* window) {
    D3D12Swapchain* d3d12_swapchain = new D3D12Swapchain{
        .tearing_supported = d3d12_is_tearing_supported(),
        .swapchain_format = DXGI_FORMAT_R8G8B8A8_UNORM
    };

    D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
    descriptorHeapDesc.NumDescriptors = d3d12_num_frames;
    descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    descriptorHeapDesc.NodeMask = 1;
    throw_if_failed(d3d12_content.device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&d3d12_swapchain->rtv_descriptor_heap)));
    d3d12_swapchain->rtv_descriptor_size = d3d12_content.device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    HWND hwnd = (HWND)platform_get_internal_handler(window);
    UVec2 size = platform_window_get_size(window);

    DXGI_SWAP_CHAIN_DESC1 swapchain_desc = {};
    swapchain_desc.Width = size.x;
    swapchain_desc.Height = size.y;
    swapchain_desc.Format = d3d12_swapchain->swapchain_format;
    swapchain_desc.Stereo = FALSE;
    swapchain_desc.SampleDesc = {1, 0};
    swapchain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapchain_desc.BufferCount = d3d12_num_frames;
    swapchain_desc.Scaling = DXGI_SCALING_STRETCH;
    swapchain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapchain_desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    swapchain_desc.Flags = d3d12_swapchain->tearing_supported ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

    IDXGIFactory4* factory;
    UINT create_factory_flags = 0;
#if defined(_DEBUG)
    create_factory_flags = DXGI_CREATE_FACTORY_DEBUG;
#endif
    throw_if_failed(CreateDXGIFactory2(create_factory_flags, IID_PPV_ARGS(&factory)));

    IDXGISwapChain1* swapchain1;
    throw_if_failed(factory->CreateSwapChainForHwnd(d3d12_content.command_queue, hwnd, &swapchain_desc, nullptr, nullptr, &swapchain1));

    throw_if_failed(factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER));
    throw_if_failed(swapchain1->QueryInterface(IID_PPV_ARGS(&d3d12_swapchain->swapchain)));

    i32 rtv_descriptor_size = d3d12_content.device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(d3d12_swapchain->rtv_descriptor_heap->GetCPUDescriptorHandleForHeapStart());

    for (int i = 0; i < d3d12_num_frames; ++i) {
        ID3D12Resource* back_buffer;
        throw_if_failed(d3d12_swapchain->swapchain->GetBuffer(i, IID_PPV_ARGS(&back_buffer)));

        d3d12_content.device->CreateRenderTargetView(back_buffer, nullptr, rtvHandle);
        d3d12_swapchain->back_buffers[i] = back_buffer;
        d3d12_swapchain->target_descriptors[i] = rtvHandle;
        rtvHandle.Offset(rtv_descriptor_size);
    }

    factory->Release();

    return {d3d12_swapchain};
}


void d3d12_destroy_swapchain(const GPUSwapchain& swapchain) {

}


void d3d12_shutdown() {
    d3d12_content.fence->Release();
    d3d12_content.command_list->Release();
    d3d12_content.srv_descriptor_heap->Release();

    for (int i = 0; i < d3d12_num_frames; ++i) {
        d3d12_content.command_allocators[i]->Release();
    }

    d3d12_content.command_queue->Release();
    d3d12_content.device->Release();

    d3d12_content = {};
}

void d3d12_device_register(GPUDeviceAPI& gpu_device_api) {
    gpu_device_api.init = d3d12_device_init;
    gpu_device_api.create_swapchain = d3d12_create_swapchain;
    gpu_device_api.destroy_swapchain = d3d12_destroy_swapchain;
    gpu_device_api.shutdown = d3d12_shutdown;
}
