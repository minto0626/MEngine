#include "MEngine.h"
#include "d3dx12.h"
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

#if _DEBUG
    CreateDXGIFactory2(dxgiFactoryFlag, IID_PPV_ARGS(_dxgiFactory.ReleaseAndGetAddressOf()));
#else
    CreateDXGIFactory1(IID_PPV_ARGS(_dxgiFactory.ReleaseAndGetAddressOf()));
#endif
    return _dxgiFactory.Get();
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
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
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

    // �t���X�N���[���̉e���ŁA�o���A�����Ƀ����_�[�^�[�Q�b�g��ԂɂȂ��Ă���\��������
    if (_renderTargetStates[frameIndex] != D3D12_RESOURCE_STATE_RENDER_TARGET)
    {
        ResourceBarrier(
            _renderTargets[frameIndex].Get(),
            D3D12_RESOURCE_STATE_PRESENT,
            D3D12_RESOURCE_STATE_RENDER_TARGET);
    }
    _renderTargetStates[frameIndex] = D3D12_RESOURCE_STATE_RENDER_TARGET;

    // �r���[�|�[�g�ƃV�U�[���N�g��ݒ�
    auto viewport = CD3DX12_VIEWPORT(_renderTargets[frameIndex].Get());
    DXGI_SWAP_CHAIN_DESC1 desc = {};
    auto result = _swapchain->GetDesc1(&desc);
    auto scissorRect = CD3DX12_RECT(0, 0, desc.Width, desc.Height);
    _commandList->RSSetViewports(1, &viewport);
    _commandList->RSSetScissorRects(1, &scissorRect);

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
    _renderTargetStates[frameIndex] = D3D12_RESOURCE_STATE_PRESENT;

    _commandList->Close();

    ID3D12CommandList* commandLists[] = { _commandList.Get() };
    _commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

    WaitDraw();

    _commandAllocator->Reset();
    _commandList->Reset(_commandAllocator.Get(), nullptr);

    PresentFrame();
}

void MEngine::PresentFrame()
{
    auto ret = _swapchain->Present(1, 0);
    if (FAILED(ret))
    {
        if (ret == DXGI_ERROR_INVALID_CALL) // �G���[#117
        {
            ::OutputDebugStringA("Present�����Ɏ��s�B�񕜂����݂܂�...\n");

            // �t���X�N���[�����ɃA�v���P�[�V�����O������E�B���h�E������Ă��܂����ꍇ�i�Ⴆ�΃X�N�V�����B�낤�Ƃ����Ƃ��j��
            // �t���X�N���[�����ێ�����悤�Ɏ��݂�
            BOOL isFullScreen;
            _swapchain->GetFullscreenState(&isFullScreen, nullptr);
            if (!isFullScreen)
            {
                ::OutputDebugStringA("�t���X�N���[����Ԃ�������Ă��邽�߁A���Ƃɖ߂��܂�\n");

                ComPtr<IDXGIAdapter> adapter;
                ret = _dxgiFactory->EnumAdapters(0, &adapter);
                if (FAILED(ret)) {
                    ::OutputDebugStringA("�A�_�v�^��������Ȃ�\n");
                    return;
                }
                ComPtr<IDXGIOutput> output;
                ret = adapter->EnumOutputs(0, &output);
                if (FAILED(ret)) {
                    ::OutputDebugStringA("���j�^�[��������Ȃ�\n");
                    return;
                }

                DXGI_OUTPUT_DESC outputDesc = {};
                output->GetDesc(&outputDesc);
                UINT width = outputDesc.DesktopCoordinates.right - outputDesc.DesktopCoordinates.left;
                UINT height = outputDesc.DesktopCoordinates.bottom - outputDesc.DesktopCoordinates.top;

                DXGI_MODE_DESC modeDesc = {};
                modeDesc.Width = width;
                modeDesc.Height = height;
                modeDesc.RefreshRate.Numerator = 144;
                modeDesc.RefreshRate.Denominator = 1;
                modeDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                DXGI_MODE_DESC closestMode;
                output->FindClosestMatchingMode(&modeDesc, &closestMode, nullptr);

                // �t���X�N���[���ɖ߂�
                ret = _swapchain->SetFullscreenState(true, output.Get());
                if (FAILED(ret)) {
                    ::OutputDebugStringA("�t���X�N���[�����Ɏ��s\n");
                    return;
                }

                ret = _swapchain->ResizeBuffers(
                    FRAME_BUFFER_COUNT,
                    closestMode.Width,
                    closestMode.Height,
                    closestMode.Format,
                    DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
                if (FAILED(ret)){
                    string error = "�o�b�t�@�̃��T�C�Y�Ɏ��s�B" + to_string(ret) + "\n";
                    ::OutputDebugStringA(error.c_str());
                    return;
                }

                // ���\�[�X���ď�����
                for (auto& buffer : _renderTargets)
                {
                    buffer.Reset();
                }
                _renderTargets.clear();
                _renderTargets.resize(FRAME_BUFFER_COUNT);
                auto rtvHandle = _rtvHeap->GetCPUDescriptorHandleForHeapStart();
                for (int i = 0; i < _renderTargets.size(); i++)
                {
                    _swapchain->GetBuffer(i, IID_PPV_ARGS(_renderTargets[i].ReleaseAndGetAddressOf()));
                    _device->CreateRenderTargetView(_renderTargets[i].Get(), nullptr, rtvHandle);
                    rtvHandle.ptr += _rtvDescriptorSize;
                }

                string log = "Restored fullscreen: " + to_string(closestMode.Width) + "x" + to_string(closestMode.Height);
                log += " @ " + to_string(closestMode.RefreshRate.Numerator) + "/" + to_string(closestMode.RefreshRate.Denominator) + "Hz\n";
                ::OutputDebugStringA(log.c_str());
            }
            else
            {
                // �t���X�N���[���Ȃ̂ɃG���[�Ȃ�A�P���ȃ��T�C�Y
                DXGI_SWAP_CHAIN_DESC swapDesc;
                _swapchain->GetDesc(&swapDesc);
                ret = _swapchain->ResizeBuffers(
                    swapDesc.BufferCount,
                    swapDesc.BufferDesc.Width,
                    swapDesc.BufferDesc.Height,
                    swapDesc.BufferDesc.Format,
                    swapDesc.Flags);
                if (FAILED(ret)) {
                    ::OutputDebugStringA("�o�b�t�@�̃��T�C�Y�Ɏ��s\n");
                }

                // ���\�[�X���ď�����
                for (auto& buffer : _renderTargets)
                {
                    buffer.Reset();
                }
                _renderTargets.clear();
                _renderTargets.resize(FRAME_BUFFER_COUNT);
                auto rtvHandle = _rtvHeap->GetCPUDescriptorHandleForHeapStart();
                for (int i = 0; i < _renderTargets.size(); i++)
                {
                    _swapchain->GetBuffer(i, IID_PPV_ARGS(_renderTargets[i].ReleaseAndGetAddressOf()));
                    _device->CreateRenderTargetView(_renderTargets[i].Get(), nullptr, rtvHandle);
                    rtvHandle.ptr += _rtvDescriptorSize;
                }

                ::OutputDebugStringA("�X���b�v�`�F�B���̃��[�h�Ńt���X�N���[����Ԃ��ێ�\n");
            }

            // �Ď��s
            ret = _swapchain->Present(1, 0);
            if (FAILED(ret))
            {
                string error = "Present�����g���C���Ă����s�B" + to_string(ret) + "\n";
                ::OutputDebugStringA(error.c_str());
            }
        }
        else
        {
            string error = "�\�����ʃG���[��Present�Ɏ��s�B" + to_string(ret) + "\n";
            ::OutputDebugStringA(error.c_str());
        }
    }
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
    auto desc = CD3DX12_RESOURCE_BARRIER::Transition(resource, from, to);
    _commandList->ResourceBarrier(1, &desc);
}

MEngine::~MEngine()
{
    WaitDraw();

    // �A�v���P�[�V�����I�����ɁA�t���X�N���[����ԂȂ�E�B���h�E�ɂ��Ă���X���b�v�`�F�B����������Ȃ���΂Ȃ�Ȃ�
    BOOL isFullScreen;
    _swapchain->GetFullscreenState(&isFullScreen, nullptr);
    if (isFullScreen)
    {
        auto ret = _swapchain->SetFullscreenState(false, nullptr);
        if (FAILED(ret))
        {
            ::OutputDebugStringA("�A�v���P�[�V�����I�����A�E�B���h�E���[�h�ւ̃X�C�b�`�Ɏ��s");
        }
    }

    CloseHandle(_fenceEvent);
    CoUninitialize();
}

bool MEngine::Init(HWND hwnd, SIZE& windowSize)
{
    _hwnd = hwnd;

    // Windows�̃X�P�[�����O�ݒ�𖳌�
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_UNAWARE);

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

    //ToggleFullScreen();
    //ToggleFullScreen();

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

void MEngine::ToggleFullScreen()
{
    BOOL isFullScreen;
    _swapchain->GetFullscreenState(&isFullScreen, nullptr);

    if (!isFullScreen)
    {
        HRESULT ret;
        // �t���X�N���[���ɐ؂�ւ���

        // GPU�����݂̃o�b�t�@�g�p���I����̂�҂�
        WaitDraw();

        // �o�b�N�o�b�t�@�̎Q�Ƃ����
        for (auto& buffer : _renderTargets)
        {
            buffer.Reset();
        }
        _renderTargets.clear();
        _swapchain.Reset();

        ComPtr<IDXGIAdapter> adapter;
        ret = _dxgiFactory->EnumAdapters(0, &adapter);
        if (FAILED(ret)) {
            ::OutputDebugStringA("�A�_�v�^��������Ȃ�\n");
            return;
        }
        ComPtr<IDXGIOutput> output;
        ret = adapter->EnumOutputs(0, &output);
        if (FAILED(ret)) {
            ::OutputDebugStringA("���j�^�[��������Ȃ�\n");
            return;
        }
        DXGI_OUTPUT_DESC outputDesc = {};
        output->GetDesc(&outputDesc);
        UINT width = outputDesc.DesktopCoordinates.right - outputDesc.DesktopCoordinates.left;
        UINT height = outputDesc.DesktopCoordinates.bottom - outputDesc.DesktopCoordinates.top;
        string op = "setting fullscreen to " + to_string(width) + "x" + to_string(height) + "\n";
        OutputDebugStringA(op.c_str());

        // ���j�^�[�̃��[�h���擾
        DXGI_MODE_DESC modeDesc = {};
        modeDesc.Width = width;
        modeDesc.Height = height;
        modeDesc.RefreshRate.Numerator = 144;   // ���j�^�[���Ή����Ă���ō����t���b�V�����[�g
        modeDesc.RefreshRate.Denominator = 1;
        modeDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        // ���j�^�[���T�|�[�g���郂�[�h�ɒ���
        DXGI_MODE_DESC closestMode;
        output->FindClosestMatchingMode(&modeDesc, &closestMode, nullptr);

        // �X���b�v�`�F�B���č쐬
        DXGI_SWAP_CHAIN_DESC1 scDesc = {};
        scDesc.Width = closestMode.Width;
        scDesc.Height = closestMode.Height;
        scDesc.Format = closestMode.Format;
        scDesc.SampleDesc.Count = 1;
        scDesc.SampleDesc.Quality = 0;
        scDesc.BufferCount = FRAME_BUFFER_COUNT;
        scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        scDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
        // �t���X�N���[���ɑΉ�
        DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsDesc{};
        fsDesc.RefreshRate.Numerator = closestMode.RefreshRate.Numerator;
        fsDesc.RefreshRate.Denominator = closestMode.RefreshRate.Denominator;
        fsDesc.Windowed = false;
        ret = _dxgiFactory->CreateSwapChainForHwnd(
            _commandQueue.Get(),
            _hwnd,
            &scDesc,
            &fsDesc,
            nullptr,
            (IDXGISwapChain1**)_swapchain.ReleaseAndGetAddressOf());
        if (FAILED(ret)) {
            ::OutputDebugStringA("�X���b�v�`�F�B���̍č쐬�Ɏ��s\n");
            return;
        }

        // �t���X�N���[����Ԃ�ݒ�
        ret = _swapchain->SetFullscreenState(true, output.Get());
        if (FAILED(ret)) {
            ::OutputDebugStringA("�t���X�N���[�����Ɏ��s\n");
            return;
        }

        // �t���X�N���[���̉𑜓x�ɍ��킹�āA�o�b�t�@�����T�C�Y
        // memo: �t���b�v���f���iDXGI_SWAP_EFFECT_FLIP_DISCARD�j�̗v���𖞂������߁A�t���X�N���[���؂�ւ���Ƀo�b�t�@���X�V����K�v������
        ret = _swapchain->ResizeBuffers(
            FRAME_BUFFER_COUNT,
            closestMode.Width,
            closestMode.Height,
            closestMode.Format,
            DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
        if (FAILED(ret)) {
            ::OutputDebugStringA("�X���b�v�`�F�C���̃o�b�t�@���T�C�Y�Ɏ��s\n");
            return;
        }

        DXGI_SWAP_CHAIN_DESC swapchainDesc;
        _swapchain->GetDesc(&swapchainDesc);
        DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullScreenDesc;
        _swapchain->GetFullscreenDesc(&fullScreenDesc);
        string sw = "swpchain resolution: " + to_string(swapchainDesc.BufferDesc.Width) + "x" + to_string(swapchainDesc.BufferDesc.Height);
        sw += " @ " + to_string(swapchainDesc.BufferDesc.RefreshRate.Numerator) + "/" + to_string(swapchainDesc.BufferDesc.RefreshRate.Denominator) + "Hz\n";
        sw += "fullscreen resolution: ";
        sw += fullScreenDesc.Windowed ? "Windowed" : "FullScreen";
        sw += " @ " + to_string(fullScreenDesc.RefreshRate.Numerator) + "/" + to_string(fullScreenDesc.RefreshRate.Denominator) + "Hz\n";
        OutputDebugStringA(sw.c_str());

        // �V�����o�b�N�o�b�t�@���擾����RTV���Đ���
        _renderTargets.resize(FRAME_BUFFER_COUNT);
        auto rtvHandle = _rtvHeap->GetCPUDescriptorHandleForHeapStart();
        for (int i = 0; i <_renderTargets.size(); i++)
        {
            _swapchain->GetBuffer(i, IID_PPV_ARGS(_renderTargets[i].ReleaseAndGetAddressOf()));
            _device->CreateRenderTargetView(_renderTargets[i].Get(), nullptr, rtvHandle);
            rtvHandle.ptr += _rtvDescriptorSize;
        }

        // ���\�[�X�̏�Ԃ�J��
        // �R�}���h���X�g����܂��B��������Ȃ��ƃR�}���h�A���P�[�^�̃��Z�b�g�Ŏ��s���܂�
        // (�����炭�R�}���h���X�g���쐬�������A�f�t�H���g�ŃI�[�v����ԂɂȂ��Ă��邩�炾�Ǝv����)
        _commandList->Close();
        _commandAllocator->Reset();
        _commandList->Reset(_commandAllocator.Get(), nullptr);
        auto frameIndex = _swapchain->GetCurrentBackBufferIndex();
        ResourceBarrier(
            _renderTargets[frameIndex].Get(),
            D3D12_RESOURCE_STATE_COMMON,
            D3D12_RESOURCE_STATE_RENDER_TARGET);
        _renderTargetStates[frameIndex] = D3D12_RESOURCE_STATE_RENDER_TARGET;
        _commandList->Close();
        ID3D12CommandList* commandLists[] = {_commandList.Get()};
        _commandQueue->ExecuteCommandLists(1, commandLists);
        WaitDraw();
        _commandAllocator->Reset();
        _commandList->Reset(_commandAllocator.Get(), nullptr);

        // �E�B���h�E�X�^�C���𒲐��i�{�[�_�[��\���j
        SetWindowLong(_hwnd, GWL_STYLE, WS_POPUP);
        SetWindowPos(_hwnd, HWND_TOP, 0, 0, width, height, SWP_FRAMECHANGED);
        ShowWindow(_hwnd, SW_SHOW);
        UpdateWindow(_hwnd);

        string ws = "�t���X�N���[���ɃX�C�b�`: " + to_string(width) + "x" + to_string(height) + "\n";
        ::OutputDebugStringA(ws.c_str());
    }
    else
    {
        // �E�B���h�E���[�h�ɖ߂�

        // GPU�̏����ҋ@
        WaitDraw();

        // �o�b�N�o�b�t�@���
        for (auto& buffer : _renderTargets)
        {
            buffer.Reset();
        }
        _renderTargets.clear();

        // �E�B���h�E���[�h�ɖ߂�
        auto ret = _swapchain->SetFullscreenState(false, nullptr);
        if (FAILED(ret))
        {
            ::OutputDebugStringA("�E�B���h�E���Ɏ��s\n");
            return;
        }

        // �o�b�t�@���E�B���h�E�T�C�Y�Ƀ��T�C�Y
        // memo: �t���b�v���f���iDXGI_SWAP_EFFECT_FLIP_DISCARD�j�̗v���𖞂������߁A�t���X�N���[���؂�ւ���Ƀo�b�t�@���X�V����K�v������
        UINT width = 1280;
        UINT height = 720;
        ret = _swapchain->ResizeBuffers(
            FRAME_BUFFER_COUNT,
            width,
            height,
            DXGI_FORMAT_R8G8B8A8_UNORM,
            DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
        if (FAILED(ret))
        {
            ::OutputDebugStringA("�X���b�v�`�F�C���̃o�b�t�@���T�C�Y�Ɏ��s\n");
            return;
        }

        DXGI_SWAP_CHAIN_DESC swapchainDesc;
        _swapchain->GetDesc(&swapchainDesc);
        DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullScreenDesc;
        _swapchain->GetFullscreenDesc(&fullScreenDesc);
        string sw = "swpchain resolution: " + to_string(swapchainDesc.BufferDesc.Width) + "x" + to_string(swapchainDesc.BufferDesc.Height);
        sw += " @ " + to_string(swapchainDesc.BufferDesc.RefreshRate.Numerator) + "/" + to_string(swapchainDesc.BufferDesc.RefreshRate.Denominator) + "Hz\n";
        sw += "fullscreen resolution: ";
        sw += fullScreenDesc.Windowed ? "Windowed" : "FullScreen";
        sw += " @ " + to_string(fullScreenDesc.RefreshRate.Numerator) + "/" + to_string(fullScreenDesc.RefreshRate.Denominator) + "Hz\n";
        OutputDebugStringA(sw.c_str());

        // RTV���Đ���
        _renderTargets.resize(FRAME_BUFFER_COUNT);
        auto rtvHandle = _rtvHeap->GetCPUDescriptorHandleForHeapStart();
        for (int i = 0; i < _renderTargets.size(); i++)
        {
            _swapchain->GetBuffer(i, IID_PPV_ARGS(_renderTargets[i].ReleaseAndGetAddressOf()));
            _device->CreateRenderTargetView(_renderTargets[i].Get(), nullptr, rtvHandle);
            rtvHandle.ptr += _rtvDescriptorSize;
        }

        // ���\�[�X�̏�Ԃ�J��
        _commandList->Close();
        _commandAllocator->Reset();
        _commandList->Reset(_commandAllocator.Get(), nullptr);
        auto frameIndex = _swapchain->GetCurrentBackBufferIndex();
        ResourceBarrier(
            _renderTargets[frameIndex].Get(),
            D3D12_RESOURCE_STATE_COMMON,
            D3D12_RESOURCE_STATE_RENDER_TARGET);
        _renderTargetStates[frameIndex] = D3D12_RESOURCE_STATE_RENDER_TARGET;
        _commandList->Close();
        ID3D12CommandList* commandLists[] = { _commandList.Get() };
        _commandQueue->ExecuteCommandLists(1, commandLists);
        WaitDraw();
        _commandAllocator->Reset();
        _commandList->Reset(_commandAllocator.Get(), nullptr);

        // �E�B���h�E�X�^�C�������ɖ߂�
        SetWindowLong(_hwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
        SetWindowPos(_hwnd, HWND_TOP, 200, 200, width, height, SWP_FRAMECHANGED);
        ShowWindow(_hwnd, SW_SHOW);
        UpdateWindow(_hwnd);

        string ws = "�E�B���h�E���[�h�ɃX�C�b�`: " + to_string(width) + "x" + to_string(height) + "\n";
        ::OutputDebugStringA(ws.c_str());
    }
}