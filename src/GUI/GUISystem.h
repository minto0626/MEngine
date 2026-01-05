#pragma once
#include <windows.h>
#include "Core/GfxDevice.h"
#include "Core/GfxCommandContext.h"
#include "DescriptorHeap/DescriptorHeap.h"
#include "Math/Vector.h"
#include "Scene/GameObject.h"

class GUISystem
{
private:
    static const std::string ImGuiIniFilePath;

public:
    bool Initialize(HWND hwnd, Graphics::GfxDevice* device, DescriptorHeap* srv_heap, DXGI_FORMAT rtvFormat);
    void Shutdown();
    void NewFrame();
    void Render(Graphics::GfxCommandContext* commandContext, DescriptorHeap* heap);
    void WinProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);

    void DrawHierarchyWindow(Vector2 window_pos, Vector2 window_size, Scene& scene, GameObject*& selectGameObject);
    void DrawSceneViewWindow(Vector2 window_pos, Vector2 window_size, DescriptorHandle sceneSRVHandle);
    void DrawInspectorWindow(Vector2 window_pos, Vector2 window_size, GameObject* gameObject);
};
