#include "MEngine.h"
#include <string>
#include <assert.h>
#include "Debug.h"
#include "Random.h"

MEngine::~MEngine()
{
    graphicsEngine.UnInitialize();

    CoUninitialize();
}

bool MEngine::Init(HWND hwnd, HINSTANCE hInstancce, SIZE& windowSize)
{
    _hwnd = hwnd;

    HRESULT ret = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(ret))
    {
        return false;
    }

    // グラフィックスエンジン初期化
    if (!graphicsEngine.Initialize(hwnd, windowSize))
    {
        return false;
    }

    // リソース読み込み
    graphicsEngine.LoadContent();

    input.Init(hInstancce, hwnd);
    time.Init();

    // 乱数生成テスト
    //Random::InitState(1001);
    int randInt = Random::Range(0, 100);
    float randf = Random::Range(0.0f, 1.0f);
    Debug::Log("rand(100) int: " + std::to_string(randInt));
    Debug::Log("rand(1.0) float: " + std::to_string(randf));

    return true;
}

void MEngine::Update()
{
    input.Update();
    time.Update();
    const float deltaTime = time.GetScaledDeltaTime();

    // deviceType 0:keyboard, 1:gamepad
    //if (!input.IsButtonDown(0, DIK_D)) { return; }
    //if (!input.IsButtonDown(1, 0)) { return; }  // [ps4] 0:□, 1:x, 2:o, 3:△,
    //if (!(std::abs(input.GetAxis(1, 0)) > 0.1f)) { return; }  // [ps4] 0:LX, 1:LY, 2:RX, 3:RY
    //if (!(input.GetAxis(1, 5) > 0.1f)) { return; }    // [ps4] 4:L2, 5:R2 うまくいかない

    Vector3 move;
    if (std::abs(input.GetAxis(1, 0)) > 0.3f)
    {
        move.SetX(input.GetAxis(1, 0));
    }
    if (std::abs(input.GetAxis(1, 1)) > 0.3f)
    {
        move.SetY(input.GetAxis(1, 1));
    }
    if (input.IsButtonDown(0, DIK_D))
    {
        move.SetX(1);
    }
    if (input.IsButtonDown(0, DIK_A))
    {
        move.SetX(-1);
    }
    if (input.IsButtonDown(0, DIK_S))
    {
        move.SetY(1);
    }
    if (input.IsButtonDown(0, DIK_W))
    {
        move.SetY(-1);
    }

    if (input.IsTriggered(0, DIK_SPACE) ||
        input.IsTriggered(1, 1))
    {
        Debug::Log("僕はエイです。");
    }

    float angle = 0.0f;
    if (input.IsButtonDown(0, DIK_Z))
    {
        angle = -1.0f;
    }
    else if (input.IsButtonDown(0, DIK_X))
    {
        angle = 1.0f;
    }
}

void MEngine::Draw()
{
    graphicsEngine.Render();
}