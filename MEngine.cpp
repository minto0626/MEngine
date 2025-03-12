#include "MEngine.h"

#include <string>
#include <assert.h>

#if _DEBUG
#include <iostream>
#include <sstream>
#endif

using namespace std;
using namespace Microsoft::WRL;

namespace
{
    string wchar_to_string(wchar_t* wstr)
    {
        if (!wstr) { return ""; }

        int size = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, nullptr, 0, nullptr, nullptr);
        if (size == 0) { return ""; }

        string str(size - 1, 0);
        WideCharToMultiByte(CP_UTF8, 0, wstr, -1, &str[0], size, nullptr, nullptr);
        return str;
    }
}

IDXGIFactory6* MEngine::CreateDXGIFactory()
{
    UINT dxgiFactoryFlag = 0;

#if _DEBUG
    ID3D12Debug* debugLayer = nullptr;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugLayer))))
    {
        dxgiFactoryFlag |= DXGI_CREATE_FACTORY_DEBUG;

        debugLayer->EnableDebugLayer();
        debugLayer->Release();
    }
#endif

    IDXGIFactory6* dxgiFactory = nullptr;

#if _DEBUG
    CreateDXGIFactory2(dxgiFactoryFlag, IID_PPV_ARGS(&dxgiFactory));
#else
    CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));
#endif
    return dxgiFactory;
}

bool MEngine::CreateDevice(IDXGIFactory6* dxgiFactory)
{
    // �r�f�I�������̗ʂ���ԑ����A�_�v�^�[��T��
    ComPtr<IDXGIAdapter> adapterTmp;
    ComPtr<IDXGIAdapter> useAdapter;
    SIZE_T maxVideoMemorySize = 0;
    for (int i = 0; dxgiFactory->EnumAdapters(i, &adapterTmp) != DXGI_ERROR_NOT_FOUND; i++)
    {
        DXGI_ADAPTER_DESC desc = {};
        adapterTmp->GetDesc(&desc);

#if _DEBUG
        ostringstream oss;
        string gpuName = wchar_to_string(desc.Description);
        oss << "�A�_�v�^�[" << (i + 1) << " ���O: " << gpuName << endl;
        oss << "  ������ID: " << desc.VendorId << endl;
        oss << "  �f�o�C�X�̓���ID: " << desc.DeviceId << endl;
        oss << "  ��pVRAM: " << desc.DedicatedVideoMemory / (1024 * 1024) << " MB" << endl;
        oss << "  �V�X�e���������Ƃ��Ċ��蓖�ĉ\�ȃ�����: " << desc.DedicatedSystemMemory / (1024 * 1024) << " MB" << endl;
        oss << "  ���L�\�ȃV�X�e��������: " << desc.SharedSystemMemory / (1024 * 1024) << " MB" << endl;
        ::OutputDebugStringA(oss.str().c_str());
        oss.str("");

        ComPtr<IDXGIOutput> outputTmp;
        for (int j = 0; j < adapterTmp->EnumOutputs(j, &outputTmp) != DXGI_ERROR_NOT_FOUND; j++)
        {
            if (outputTmp == nullptr) { break; }

            DXGI_OUTPUT_DESC opDesc = {};
            outputTmp->GetDesc(&opDesc);

            oss << "���j�^�[��: " << wchar_to_string(opDesc.DeviceName) << endl;
            oss << "  ���j�^�[�T�C�Y" << endl;
            oss << "   width: " << opDesc.DesktopCoordinates.right - opDesc.DesktopCoordinates.left << endl;
            oss << "   height: " << opDesc.DesktopCoordinates.bottom - opDesc.DesktopCoordinates.top << endl;
            ::OutputDebugStringA(oss.str().c_str());
            oss.str("");

            UINT numModes = 0;
            outputTmp->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, 0, &numModes, nullptr);
            vector<DXGI_MODE_DESC> modeLists(numModes);
            outputTmp->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, 0, &numModes, modeLists.data());

            oss << "  �T�|�[�g���[�h " << numModes << "��" << endl;
            for (const auto& mode : modeLists)
            {
                // ����->�����ɋߎ�
                UINT refreshRate = (mode.RefreshRate.Numerator + mode.RefreshRate.Denominator - 1) / mode.RefreshRate.Denominator;
                oss << "   " << mode.Width << "x" << mode.Height << " @ " << refreshRate << " Hz" << endl;
            }
            ::OutputDebugStringA(oss.str().c_str());
            oss.str("");
        }
#endif

        if (desc.DedicatedVideoMemory > maxVideoMemorySize)
        {
            useAdapter = adapterTmp;
            maxVideoMemorySize = desc.DedicatedVideoMemory;
        }
    }

    // �t�B�[�`���[���x���̑I��
    D3D_FEATURE_LEVEL levels[] =
    {
        D3D_FEATURE_LEVEL_12_1,
        D3D_FEATURE_LEVEL_12_0,
    };
    for (auto level : levels)
    {
        if (SUCCEEDED(D3D12CreateDevice(useAdapter.Get(), level, IID_PPV_ARGS(_device.ReleaseAndGetAddressOf()))))
        {
            return true;
        }
    }

    return false;
}

bool MEngine::CreateCommandList()
{
    HRESULT ret;

    // �R�}���h�A���P�[�^�̍쐬
    ret = _device->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        IID_PPV_ARGS(_commandAllocator.ReleaseAndGetAddressOf()));
    if (FAILED(ret))
    {
        return false;
    }
    // �R�}���h���X�g�̍쐬
    ret = _device->CreateCommandList(
        0,
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        _commandAllocator.Get(),
        nullptr,
        IID_PPV_ARGS(_commandList.ReleaseAndGetAddressOf()));
    if (FAILED(ret))
    {
        return false;
    }

    return true;
}

bool MEngine::CreateCommandQueue()
{
    // �R�}���h�L���[�̍쐬
    D3D12_COMMAND_QUEUE_DESC desc = {};
    desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE; // �^�C���A�E�g����
    desc.NodeMask = 0;  // �A�_�v�^�[�P��
    desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    desc.Type = _commandList.Get()->GetType();

    HRESULT ret = _device->CreateCommandQueue(
        &desc,
        IID_PPV_ARGS(_commandQueue.ReleaseAndGetAddressOf()));
    if (FAILED(ret))
    {
        return false;
    }

    return true;
}

bool MEngine::CreateSwapchain(HWND hwnd, SIZE& windowSize, IDXGIFactory6* dxgiFactory)
{
    // �X���b�v�`�F�C���̍쐬
    DXGI_SWAP_CHAIN_DESC1 desc = {};
    desc.Width = windowSize.cx;
    desc.Height = windowSize.cy;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.Stereo = false;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.BufferCount = FRAME_BUFFER_COUNT;
    desc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
    desc.Scaling = DXGI_SCALING_STRETCH;
    desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    HRESULT ret = dxgiFactory->CreateSwapChainForHwnd(
        _commandQueue.Get(),
        hwnd,
        &desc,
        nullptr,
        nullptr,
        (IDXGISwapChain1**)_swapchain.ReleaseAndGetAddressOf());
    if (FAILED(ret))
    {
        return false;
    }

    return true;
}

bool MEngine::CreateFinalRenderTarget()
{
    // �����_�[�^�[�Q�b�g�p�̃q�[�v���쐬
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    heapDesc.NodeMask = 0;
    heapDesc.NumDescriptors = FRAME_BUFFER_COUNT;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    HRESULT ret = _device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(_rtvHeap.ReleaseAndGetAddressOf()));
    if (FAILED(ret))
    {
        return false;
    }

    // �X���b�v�`�F�C���̃������ƕR�Â���
    DXGI_SWAP_CHAIN_DESC scDesc = {};
    ret = _swapchain->GetDesc(&scDesc);
    _renderTargets.resize(scDesc.BufferCount);
    auto rtvHandle = _rtvHeap->GetCPUDescriptorHandleForHeapStart();
    for (int i = 0; i < scDesc.BufferCount; i++)
    {
        ret = _swapchain->GetBuffer(i, IID_PPV_ARGS(_renderTargets[i].ReleaseAndGetAddressOf()));
        assert(SUCCEEDED(ret));
        _device->CreateRenderTargetView(_renderTargets[i].Get(), nullptr, rtvHandle);
        rtvHandle.ptr += _rtvDescriptorSize;
    }

    return true;
}

bool MEngine::CreateSynchronizationWithGPUObject()
{
    // �t�F���X�̍쐬
    HRESULT ret = _device->CreateFence(_fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(_fence.ReleaseAndGetAddressOf()));
    if (FAILED(ret))
    {
        return false;
    }

    // GPU�Ɠ��������Ƃ��̃C�x���g�n���h�����쐬
    _fenceEvent = CreateEvent(nullptr, false, false, nullptr);
    if (_fenceEvent == nullptr)
    {
        return false;
    }
    return true;
}

void MEngine::BegineRender()
{
    auto frameIndex = _swapchain->GetCurrentBackBufferIndex();

    ResourceBarrier(
        _renderTargets[frameIndex].Get(),
        D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET);

    // �����_�[�^�[�Q�b�g�̐ݒ�
    auto rtvHandle = _rtvHeap->GetCPUDescriptorHandleForHeapStart();
    rtvHandle.ptr += frameIndex * _rtvDescriptorSize;
    _commandList->OMSetRenderTargets(1, &rtvHandle, false, nullptr);

    // �����_�[�^�[�Q�b�g�̃N���A
    float cc[] = { .0f, .0f, 1.0f, 1.0f };
    _commandList->ClearRenderTargetView(rtvHandle, cc, 0, nullptr);
}

void MEngine::EndRender()
{
    auto frameIndex = _swapchain->GetCurrentBackBufferIndex();

    ResourceBarrier(
        _renderTargets[frameIndex].Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PRESENT);

    _commandList->Close();

    ID3D12CommandList* commandLists[] = { _commandList.Get() };
    _commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

    WaitDraw();

    _commandAllocator->Reset();
    _commandList->Reset(_commandAllocator.Get(), nullptr);

    _swapchain->Present(1, 0);
}

void MEngine::WaitDraw()
{
    _commandQueue->Signal(_fence.Get(), ++_fenceValue);
    
    if (_fence->GetCompletedValue() != _fenceValue)
    {
        _fence->SetEventOnCompletion(_fenceValue, _fenceEvent);
        WaitForSingleObject(_fenceEvent, INFINITE);
    }
}

void MEngine::ResourceBarrier(ID3D12Resource* resource, D3D12_RESOURCE_STATES from, D3D12_RESOURCE_STATES to)
{
    D3D12_RESOURCE_BARRIER desc = {};
    desc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    desc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    desc.Transition.pResource = resource;
    desc.Transition.Subresource = 0;
    desc.Transition.StateBefore = from;
    desc.Transition.StateAfter = to;
    _commandList->ResourceBarrier(1, &desc);
}

MEngine::~MEngine()
{
    WaitDraw();
    CloseHandle(_fenceEvent);
    CoUninitialize();
}

bool MEngine::Init(HWND hwnd, SIZE& windowSize)
{
    HRESULT ret = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(ret))
    {
        return false;
    }

    auto dxgiFactory = CreateDXGIFactory();
    if (!CreateDevice(dxgiFactory))
    {
        assert("�f�o�C�X�̍쐬�Ɏ��s�I");
        return false;
    }
    // RTV�p�q�[�v�̃T�C�Y���擾
    _rtvDescriptorSize = _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    if (!CreateCommandList())
    {
        assert("�R�}���h���X�g�̍쐬�Ɏ��s�I");
        return false;
    }
    if (!CreateCommandQueue())
    {
        assert("�R�}���h�L���[�̍쐬�Ɏ��s!");
        return false;
    }
    if (!CreateSwapchain(hwnd, windowSize, dxgiFactory))
    {
        assert("�X���b�v�`�F�C���̍쐬�Ɏ��s�I");
        return false;
    }
    if (!CreateFinalRenderTarget())
    {
        assert("�ŏI�I�ȕ`���̍쐬�Ɏ��s�I");
        return false;
    }
    if (!CreateSynchronizationWithGPUObject())
    {
        assert("�t�F���X�̍쐬�Ɏ��s�I");
        return false;
    }

    dxgiFactory->Release();

    return true;
}

void MEngine::Update()
{
}

void MEngine::Draw()
{
    BegineRender();
    EndRender();
}
