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
    // ビデオメモリの量が一番多いアダプターを探す
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
        oss << "アダプター" << (i + 1) << " 名前: " << gpuName << endl;
        oss << "  製造元ID: " << desc.VendorId << endl;
        oss << "  デバイスの特定ID: " << desc.DeviceId << endl;
        oss << "  専用VRAM: " << desc.DedicatedVideoMemory / (1024 * 1024) << " MB" << endl;
        oss << "  システムメモリとして割り当て可能なメモリ: " << desc.DedicatedSystemMemory / (1024 * 1024) << " MB" << endl;
        oss << "  共有可能なシステムメモリ: " << desc.SharedSystemMemory / (1024 * 1024) << " MB" << endl;
        ::OutputDebugStringA(oss.str().c_str());
        oss.str("");

        ComPtr<IDXGIOutput> outputTmp;
        for (int j = 0; j < adapterTmp->EnumOutputs(j, &outputTmp) != DXGI_ERROR_NOT_FOUND; j++)
        {
            if (outputTmp == nullptr) { break; }

            DXGI_OUTPUT_DESC opDesc = {};
            outputTmp->GetDesc(&opDesc);

            oss << "モニター名: " << wchar_to_string(opDesc.DeviceName) << endl;
            oss << "  モニターサイズ" << endl;
            oss << "   width: " << opDesc.DesktopCoordinates.right - opDesc.DesktopCoordinates.left << endl;
            oss << "   height: " << opDesc.DesktopCoordinates.bottom - opDesc.DesktopCoordinates.top << endl;
            ::OutputDebugStringA(oss.str().c_str());
            oss.str("");

            UINT numModes = 0;
            outputTmp->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, 0, &numModes, nullptr);
            vector<DXGI_MODE_DESC> modeLists(numModes);
            outputTmp->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, 0, &numModes, modeLists.data());

            oss << "  サポートモード " << numModes << "個" << endl;
            for (const auto& mode : modeLists)
            {
                // 分数->整数に近似
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

    // フィーチャーレベルの選定
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

    // コマンドアロケータの作成
    ret = _device->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        IID_PPV_ARGS(_commandAllocator.ReleaseAndGetAddressOf()));
    if (FAILED(ret))
    {
        return false;
    }
    // コマンドリストの作成
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
    // コマンドキューの作成
    D3D12_COMMAND_QUEUE_DESC desc = {};
    desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE; // タイムアウト無し
    desc.NodeMask = 0;  // アダプター１つ
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
    // スワップチェインの作成
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
    // レンダーターゲット用のヒープを作成
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

    // スワップチェインのメモリと紐づける
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
    // フェンスの作成
    HRESULT ret = _device->CreateFence(_fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(_fence.ReleaseAndGetAddressOf()));
    if (FAILED(ret))
    {
        return false;
    }

    // GPUと同期を取るときのイベントハンドルを作成
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

    // フルスクリーンの影響で、バリアが既にレンダーターゲット状態になっている可能性がある
    if (_renderTargetStates[frameIndex] != D3D12_RESOURCE_STATE_RENDER_TARGET)
    {
        ResourceBarrier(
            _renderTargets[frameIndex].Get(),
            D3D12_RESOURCE_STATE_PRESENT,
            D3D12_RESOURCE_STATE_RENDER_TARGET);
    }
    _renderTargetStates[frameIndex] = D3D12_RESOURCE_STATE_RENDER_TARGET;

    // ビューポートとシザーレクトを設定
    auto viewport = CD3DX12_VIEWPORT(_renderTargets[frameIndex].Get());
    DXGI_SWAP_CHAIN_DESC1 desc = {};
    auto result = _swapchain->GetDesc1(&desc);
    auto scissorRect = CD3DX12_RECT(0, 0, desc.Width, desc.Height);
    _commandList->RSSetViewports(1, &viewport);
    _commandList->RSSetScissorRects(1, &scissorRect);

    // レンダーターゲットの設定
    auto rtvHandle = _rtvHeap->GetCPUDescriptorHandleForHeapStart();
    rtvHandle.ptr += frameIndex * _rtvDescriptorSize;
    _commandList->OMSetRenderTargets(1, &rtvHandle, false, nullptr);

    // レンダーターゲットのクリア
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
        if (ret == DXGI_ERROR_INVALID_CALL) // エラー#117
        {
            ::OutputDebugStringA("Present処理に失敗。回復を試みます...\n");

            // フルスクリーン時にアプリケーション外部からウィンドウ化されてしまった場合（例えばスクショを撮ろうとしたとき）に
            // フルスクリーンを維持するように試みる
            BOOL isFullScreen;
            _swapchain->GetFullscreenState(&isFullScreen, nullptr);
            if (!isFullScreen)
            {
                ::OutputDebugStringA("フルスクリーン状態が解除れているため、もとに戻します\n");

                ComPtr<IDXGIAdapter> adapter;
                ret = _dxgiFactory->EnumAdapters(0, &adapter);
                if (FAILED(ret)) {
                    ::OutputDebugStringA("アダプタが見つからない\n");
                    return;
                }
                ComPtr<IDXGIOutput> output;
                ret = adapter->EnumOutputs(0, &output);
                if (FAILED(ret)) {
                    ::OutputDebugStringA("モニターが見つからない\n");
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

                // フルスクリーンに戻す
                ret = _swapchain->SetFullscreenState(true, output.Get());
                if (FAILED(ret)) {
                    ::OutputDebugStringA("フルスクリーン化に失敗\n");
                    return;
                }

                ret = _swapchain->ResizeBuffers(
                    FRAME_BUFFER_COUNT,
                    closestMode.Width,
                    closestMode.Height,
                    closestMode.Format,
                    DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
                if (FAILED(ret)){
                    string error = "バッファのリサイズに失敗。" + to_string(ret) + "\n";
                    ::OutputDebugStringA(error.c_str());
                    return;
                }

                // リソースを再初期化
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
                // フルスクリーンなのにエラーなら、単純なリサイズ
                DXGI_SWAP_CHAIN_DESC swapDesc;
                _swapchain->GetDesc(&swapDesc);
                ret = _swapchain->ResizeBuffers(
                    swapDesc.BufferCount,
                    swapDesc.BufferDesc.Width,
                    swapDesc.BufferDesc.Height,
                    swapDesc.BufferDesc.Format,
                    swapDesc.Flags);
                if (FAILED(ret)) {
                    ::OutputDebugStringA("バッファのリサイズに失敗\n");
                }

                // リソースを再初期化
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

                ::OutputDebugStringA("スワップチェィンのモードでフルスクリーン状態を維持\n");
            }

            // 再試行
            ret = _swapchain->Present(1, 0);
            if (FAILED(ret))
            {
                string error = "Presentをリトライしても失敗。" + to_string(ret) + "\n";
                ::OutputDebugStringA(error.c_str());
            }
        }
        else
        {
            string error = "予期せぬエラーでPresentに失敗。" + to_string(ret) + "\n";
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

    // アプリケーション終了時に、フルスクリーン状態ならウィンドウにしてからスワップチェィンを解放しなければならない
    BOOL isFullScreen;
    _swapchain->GetFullscreenState(&isFullScreen, nullptr);
    if (isFullScreen)
    {
        auto ret = _swapchain->SetFullscreenState(false, nullptr);
        if (FAILED(ret))
        {
            ::OutputDebugStringA("アプリケーション終了時、ウィンドウモードへのスイッチに失敗");
        }
    }

    CloseHandle(_fenceEvent);
    CoUninitialize();
}

bool MEngine::Init(HWND hwnd, SIZE& windowSize)
{
    _hwnd = hwnd;

    // Windowsのスケーリング設定を無効
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_UNAWARE);

    HRESULT ret = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(ret))
    {
        return false;
    }

    auto dxgiFactory = CreateDXGIFactory();
    if (!CreateDevice(dxgiFactory))
    {
        assert("デバイスの作成に失敗！");
        return false;
    }
    // RTV用ヒープのサイズを取得
    _rtvDescriptorSize = _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    if (!CreateCommandList())
    {
        assert("コマンドリストの作成に失敗！");
        return false;
    }
    if (!CreateCommandQueue())
    {
        assert("コマンドキューの作成に失敗!");
        return false;
    }
    if (!CreateSwapchain(hwnd, windowSize, dxgiFactory))
    {
        assert("スワップチェインの作成に失敗！");
        return false;
    }
    if (!CreateFinalRenderTarget())
    {
        assert("最終的な描画先の作成に失敗！");
        return false;
    }
    if (!CreateSynchronizationWithGPUObject())
    {
        assert("フェンスの作成に失敗！");
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
        // フルスクリーンに切り替える

        // GPUが現在のバッファ使用を終えるのを待つ
        WaitDraw();

        // バックバッファの参照を解放
        for (auto& buffer : _renderTargets)
        {
            buffer.Reset();
        }
        _renderTargets.clear();
        _swapchain.Reset();

        ComPtr<IDXGIAdapter> adapter;
        ret = _dxgiFactory->EnumAdapters(0, &adapter);
        if (FAILED(ret)) {
            ::OutputDebugStringA("アダプタが見つからない\n");
            return;
        }
        ComPtr<IDXGIOutput> output;
        ret = adapter->EnumOutputs(0, &output);
        if (FAILED(ret)) {
            ::OutputDebugStringA("モニターが見つからない\n");
            return;
        }
        DXGI_OUTPUT_DESC outputDesc = {};
        output->GetDesc(&outputDesc);
        UINT width = outputDesc.DesktopCoordinates.right - outputDesc.DesktopCoordinates.left;
        UINT height = outputDesc.DesktopCoordinates.bottom - outputDesc.DesktopCoordinates.top;
        string op = "setting fullscreen to " + to_string(width) + "x" + to_string(height) + "\n";
        OutputDebugStringA(op.c_str());

        // モニターのモードを取得
        DXGI_MODE_DESC modeDesc = {};
        modeDesc.Width = width;
        modeDesc.Height = height;
        modeDesc.RefreshRate.Numerator = 144;   // モニターが対応している最高リフレッシュレート
        modeDesc.RefreshRate.Denominator = 1;
        modeDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        // モニターがサポートするモードに調整
        DXGI_MODE_DESC closestMode;
        output->FindClosestMatchingMode(&modeDesc, &closestMode, nullptr);

        // スワップチェィン再作成
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
        // フルスクリーンに対応
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
            ::OutputDebugStringA("スワップチェィンの再作成に失敗\n");
            return;
        }

        // フルスクリーン状態を設定
        ret = _swapchain->SetFullscreenState(true, output.Get());
        if (FAILED(ret)) {
            ::OutputDebugStringA("フルスクリーン化に失敗\n");
            return;
        }

        // フルスクリーンの解像度に合わせて、バッファをリサイズ
        // memo: フリップモデル（DXGI_SWAP_EFFECT_FLIP_DISCARD）の要件を満たすため、フルスクリーン切り替え後にバッファを更新する必要がある
        ret = _swapchain->ResizeBuffers(
            FRAME_BUFFER_COUNT,
            closestMode.Width,
            closestMode.Height,
            closestMode.Format,
            DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
        if (FAILED(ret)) {
            ::OutputDebugStringA("スワップチェインのバッファリサイズに失敗\n");
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

        // 新しいバックバッファを取得してRTVを再生成
        _renderTargets.resize(FRAME_BUFFER_COUNT);
        auto rtvHandle = _rtvHeap->GetCPUDescriptorHandleForHeapStart();
        for (int i = 0; i <_renderTargets.size(); i++)
        {
            _swapchain->GetBuffer(i, IID_PPV_ARGS(_renderTargets[i].ReleaseAndGetAddressOf()));
            _device->CreateRenderTargetView(_renderTargets[i].Get(), nullptr, rtvHandle);
            rtvHandle.ptr += _rtvDescriptorSize;
        }

        // リソースの状態を遷移
        // コマンドリストを閉じます。これをしないとコマンドアロケータのリセットで失敗します
        // (おそらくコマンドリストを作成した時、デフォルトでオープン状態になっているからだと思われる)
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

        // ウィンドウスタイルを調整（ボーダー非表示）
        SetWindowLong(_hwnd, GWL_STYLE, WS_POPUP);
        SetWindowPos(_hwnd, HWND_TOP, 0, 0, width, height, SWP_FRAMECHANGED);
        ShowWindow(_hwnd, SW_SHOW);
        UpdateWindow(_hwnd);

        string ws = "フルスクリーンにスイッチ: " + to_string(width) + "x" + to_string(height) + "\n";
        ::OutputDebugStringA(ws.c_str());
    }
    else
    {
        // ウィンドウモードに戻す

        // GPUの処理待機
        WaitDraw();

        // バックバッファ解放
        for (auto& buffer : _renderTargets)
        {
            buffer.Reset();
        }
        _renderTargets.clear();

        // ウィンドウモードに戻す
        auto ret = _swapchain->SetFullscreenState(false, nullptr);
        if (FAILED(ret))
        {
            ::OutputDebugStringA("ウィンドウ化に失敗\n");
            return;
        }

        // バッファをウィンドウサイズにリサイズ
        // memo: フリップモデル（DXGI_SWAP_EFFECT_FLIP_DISCARD）の要件を満たすため、フルスクリーン切り替え後にバッファを更新する必要がある
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
            ::OutputDebugStringA("スワップチェインのバッファリサイズに失敗\n");
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

        // RTVを再生成
        _renderTargets.resize(FRAME_BUFFER_COUNT);
        auto rtvHandle = _rtvHeap->GetCPUDescriptorHandleForHeapStart();
        for (int i = 0; i < _renderTargets.size(); i++)
        {
            _swapchain->GetBuffer(i, IID_PPV_ARGS(_renderTargets[i].ReleaseAndGetAddressOf()));
            _device->CreateRenderTargetView(_renderTargets[i].Get(), nullptr, rtvHandle);
            rtvHandle.ptr += _rtvDescriptorSize;
        }

        // リソースの状態を遷移
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

        // ウィンドウスタイルを元に戻す
        SetWindowLong(_hwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
        SetWindowPos(_hwnd, HWND_TOP, 200, 200, width, height, SWP_FRAMECHANGED);
        ShowWindow(_hwnd, SW_SHOW);
        UpdateWindow(_hwnd);

        string ws = "ウィンドウモードにスイッチ: " + to_string(width) + "x" + to_string(height) + "\n";
        ::OutputDebugStringA(ws.c_str());
    }
}