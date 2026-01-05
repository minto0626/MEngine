#include "GUI/GUISystem.h"
#include "MEngine.h"
#include "Core/GfxCommandContext.h"
#include "Scene/Transform.h"
#include "Renderer/MeshRenderer.h"
#include "Library/imgui/imgui.h"
#include "Library/imgui/imgui_impl_dx12.h"
#include "Library/imgui/imgui_impl_win32.h"

const std::string GUISystem::ImGuiIniFilePath = MEngine::CacheDirectory + "imgui.ini";
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

bool GUISystem::Initialize(HWND hwnd, Graphics::GfxDevice* device, DescriptorHeap* srv_heap, DXGI_FORMAT rtvFormat)
{
    IMGUI_CHECKVERSION();

    if (ImGui::CreateContext() == nullptr)
    {
        assert(0 && "ImGui コンテキストの作成に失敗！");
        return false;
    }

    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // キーボード操作を有効化
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // ゲームパッド操作を有効化

    ImGui::StyleColorsClassic();

    ImGuiStyle& style = ImGui::GetStyle();

    // 各背景の透明度を固定
    style.Colors[ImGuiCol_WindowBg] = ImColor(0.1f, 0.1f, 0.1f, 1.0f);
    style.Colors[ImGuiCol_TitleBgActive].w = 1.0f;
    style.Colors[ImGuiCol_TitleBg].w = 1.0f;

    auto result = ImGui_ImplWin32_Init(hwnd);

    auto* heap = srv_heap->GetHeap();
    auto handle = srv_heap->Allocate();
    result = ImGui_ImplDX12_Init(
        device->Get(),
        3,
        rtvFormat,
        heap,
        handle.cpuHandle,
        handle.gpuHandle);

    // todo: ソースコードのエンコーディングがUTF-8でない限り、日本語が文字化けする。
    //       utf8オプション付きでコンパイルする必要があるが、それだけだとVisual Studioのエディタ上で文字化けする。
    // 日本語フォントを追加
    ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\meiryo.ttc", 16.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());

    // もしビルドされていなければ強制実行
    if (!io.Fonts->IsBuilt())
    {
        Debug::Log("ImGui フォントのビルドを強制実行します。");
        if (!io.Fonts->Build())
        {
            assert(0 && "ImGui フォントのビルドに失敗！");
        }
    }

    // .iniの保存場所を変更（カレントディレクトリ直下に保存されるのを防ぐため）
    io.IniFilename = GUISystem::ImGuiIniFilePath.c_str();

    return true;
}

void GUISystem::Shutdown()
{
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

void GUISystem::NewFrame()
{
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void GUISystem::Render(Graphics::GfxCommandContext* commandContext, DescriptorHeap* heap)
{
    ImGui::Render();

    ID3D12DescriptorHeap* const heaps[] = { heap->GetHeap() };
    commandContext->GetCommandList()->SetDescriptorHeaps(1, heaps);
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandContext->GetCommandList());
}

void GUISystem::WinProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
    ImGui_ImplWin32_WndProcHandler(hWnd, msg, wp, lp);
}

void GUISystem::DrawHierarchyWindow(Vector2 window_pos, Vector2 window_size, Scene& scene, GameObject*& selectGameObject)
{
    ImGui::SetNextWindowPos(ImVec2(window_pos.GetX(), window_pos.GetY()), ImGuiCond_::ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(window_size.GetX(), window_size.GetY()), ImGuiCond_::ImGuiCond_Always);
    ImGui::Begin("Hierarchy");
    std::function<void(GameObject*)> drawNode = [&](GameObject* gameObject)
    {
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
        if (gameObject == selectGameObject)
        {
            flags |= ImGuiTreeNodeFlags_Selected;
        }
        // todo: 子オブジェクト対応
        bool hasChildren = false/*gameObject.GetChildren().size() > 0*/;
        if (!hasChildren)
        {
            flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
        }
        bool nodeOpen = ImGui::TreeNodeEx(gameObject->GetName().c_str(), flags);
        if (ImGui::IsItemClicked())
        {
            selectGameObject = gameObject;
        }
        if (nodeOpen && hasChildren)
        {
            // todo: 子オブジェクト対応
            //for (auto& child : gameObject.GetChildren())
            //{
            //    drawNode(*child);
            //}
            ImGui::TreePop();
        }
    };

    for (auto& rootGameObject : scene.GetAllGameObjects())
    {
        drawNode(rootGameObject.get());
    }

    ImGui::End();
}

void GUISystem::DrawSceneViewWindow(Vector2 window_pos, Vector2 window_size, DescriptorHandle sceneSRVHandle)
{
    ImGui::SetNextWindowPos(ImVec2(window_pos.GetX(), window_pos.GetY()), ImGuiCond_::ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(window_size.GetX(), window_size.GetY()), ImGuiCond_::ImGuiCond_Once);
    ImGui::Begin("Scene View", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Image((ImTextureID)sceneSRVHandle.gpuHandle.ptr, ImVec2(window_size.GetX(), window_size.GetY()));
    ImGui::End();
}

void GUISystem::DrawInspectorWindow(Vector2 window_pos, Vector2 window_size, GameObject* gameObject)
{
    ImGui::SetNextWindowPos(ImVec2(window_pos.GetX(), window_pos.GetY()), ImGuiCond_::ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(window_size.GetX(), window_size.GetY()), ImGuiCond_::ImGuiCond_Always);
    ImGui::Begin("Inspector");

    if (gameObject != nullptr)
    {
        ImGui::InputText("Name", (char*)gameObject->GetName().c_str(), 256);

        ImGui::SetNextItemOpen(true, ImGuiCond_::ImGuiCond_Once);
        if (ImGui::TreeNode("Transform"))
        {
            auto& transform = *gameObject->GetTransform();
            Vector3 pos = transform.GetPos();
            Vector3 rot = Quaternion::ToEulerAngles(transform.GetRot());
            Vector3 scale = transform.GetScale();
            float positionArray[] = { pos.GetX(), pos.GetY(), pos.GetZ() };
            float rotationArray[] = { rot.GetX(), rot.GetY(), rot.GetZ() };
            float scaleArray[] = { scale.GetX(), scale.GetY(), scale.GetZ() };
            if (ImGui::DragFloat3("Position", positionArray, 0.01f))
            {
                pos.SetX(positionArray[0]);
                pos.SetY(positionArray[1]);
                pos.SetZ(positionArray[2]);
                transform.SetPos(pos);
            }
            // todo: ジンバルロックの影響で表示がおかしくなる。Transform側で回避策を用意する（オイラー角も保持しておくなど）
            if (ImGui::DragFloat3("Rotation", rotationArray, 0.01f))
            {
                rot.SetX(rotationArray[0]);
                rot.SetY(rotationArray[1]);
                rot.SetZ(rotationArray[2]);
                transform.SetRot(Quaternion::FromEulerAngles(rot.GetX(), rot.GetY(), rot.GetZ()));
            }
            if (ImGui::DragFloat3("Scale", scaleArray, 0.01f))
            {
                scale.SetX(scaleArray[0]);
                scale.SetY(scaleArray[1]);
                scale.SetZ(scaleArray[2]);
                transform.SetScale(scale);
            }

            ImGui::TreePop();
        }
    }

    ImGui::End();
}
