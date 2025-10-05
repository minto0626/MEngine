#include "MEngine.h"
#include <string>
#include <assert.h>
#include "Utility/Debug.h"
#include "Utility/GameTime.h"
#include "Sprite/Sprite.h"
#include "Material/Material.h"
#include "Resources/ConstantBuffer.h"
#include "Renderer/SpriteRenderer.h"
#include "Scene/GameObject.h"
#include "Scene/Camera.h"

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

	// シーン生成
	_scene = std::make_unique<Scene>();
	_scene->Init(&graphicsEngine);

    input.Init(hInstancce, hwnd);
    time.Init();

	// ゲームオブジェクト生成

    // 2Dカメラ
    {
        camera2D = _scene->CreateGameObject("Camera2D")->AddComponent<Camera>();
        camera2D->Init(Camera::ProjectionType::Ortho, windowSize.cx, windowSize.cy);
        camera2D->SetPos({ 0.0f, 0.0f, 0.0f });
	}
    // 3Dカメラ
    {
        camera3D = _scene->CreateGameObject("Camera3D")->AddComponent<Camera>();
        camera3D->Init(Camera::ProjectionType::Perspective, windowSize.cx, windowSize.cy);
        camera3D->SetPos({ 0.0f, 0.0f, -5.0f });
        camera3D->SetTarget({ 0.0f, 0.0f, 0.0f });
    }

    // スプライト
    {
	    sample_spriteObject = _scene->CreateGameObject("SampleGameObject_1");
        sample_spriteObject->GetTransform()->SetPos({ 640.0f, 240.0f, 0.0f });
		sample_spriteObject->GetTransform()->SetRot(Quaternion::FromEulerAngles(0.0f, 0.0f, 0.0f));
		sample_spriteObject->GetTransform()->SetScale({ 2.0f, 2.0f, 1.0f });
        auto spriteRenderer = sample_spriteObject->AddComponent<Graphics::SpriteRenderer>();
        Graphics::Sprite* sprite = graphicsEngine.GetSprite("testSprite");
        spriteRenderer->SetSprite(sprite);
        Graphics::Material* material = graphicsEngine.GetMaterial("DefaultMaterial");
	    Texture* texture = graphicsEngine.GetTexture("Assets/texture/free_brachiosaurus.png");
        UINT texIdx = graphicsEngine.GetRootParameterIndex("mainTex", *material);
	    material->SetTexture(texIdx, texture);
	    spriteRenderer->SetMaterial(material);
    }

	// メッシュ
    {
	    sample_meshObject = _scene->CreateGameObject("SampleGameObject_2");
	    sample_meshObject->GetTransform()->SetPos({ 0.0f, -1.0f, 0.0f });
	    auto meshRenderer = sample_meshObject->AddComponent<Graphics::MeshRenderer>();
	    auto mesh = graphicsEngine.GetMesh(L"Assets/3D/samples/teapot/teapot.fbx");
	    meshRenderer->SetMesh(mesh);
		Graphics::Material* material = graphicsEngine.GetMaterial("3DMaterial");
		Texture* texture = graphicsEngine.GetTexture("Assets/3D/samples/teapot/default.png");
		UINT texIdx = graphicsEngine.GetRootParameterIndex("mainTex", *material);
		material->SetTexture(texIdx, texture);
		meshRenderer->SetMaterial(material);
    }

    return true;
}

void MEngine::Update()
{
    input.Update();
    time.Update();

    const float deltaTime = time.GetScaledDeltaTime();

	_scene->Update(deltaTime);

    // deviceType 0:keyboard, 1:gamepad
    //if (!input.IsButtonDown(0, DIK_D)) { return; }
    //if (!input.IsButtonDown(1, 0)) { return; }  // [ps4] 0:□, 1:x, 2:o, 3:△,
    //if (!(std::abs(input.GetAxis(1, 0)) > 0.1f)) { return; }  // [ps4] 0:LX, 1:LY, 2:RX, 3:RY
    //if (!(input.GetAxis(1, 5) > 0.1f)) { return; }    // [ps4] 4:L2, 5:R2 うまくいかない

    Vector3 move;
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
    if (move.Length() > 0.0f)
    {
        move.Normalize();
        float moveSpeed = 300.0f; // 1秒で300px移動
        move *= moveSpeed * deltaTime;
        auto pos = sample_spriteObject->GetTransform()->GetPos();
        pos += move;
        sample_spriteObject->GetTransform()->SetPos(pos);
	}

    move = Vector3::Zero();
    sample_meshObject->GetTransform()->SetRot(
		sample_meshObject->GetTransform()->GetRot() * Quaternion::FromEulerAngles(0.0f, 0.25f, 0.0f));
    if (input.IsButtonDown(0, DIK_LEFTARROW))
    {
		move.SetX(-1);
    }
    if (input.IsButtonDown(0, DIK_RIGHTARROW))
    {
		move.SetX(1);
    }
    if (move.Length() > 0.0f)
    {
        move.Normalize();
        float moveSpeed = 3.0f; // 1秒で3m移動
        move *= moveSpeed * deltaTime;
        auto pos = sample_meshObject->GetTransform()->GetPos();
        pos += move;
        sample_meshObject->GetTransform()->SetPos(pos);
	}

	move = Vector3::Zero();
    if (input.IsButtonDown(0, DIK_UPARROW))
    {
		move.SetY(1);
    }
    if (input.IsButtonDown(0, DIK_DOWNARROW))
    {
		move.SetY(-1);
	}
    if (move.Length() > 0.0f)
    {
        move.Normalize();
        float moveSpeed = 3.0f; // 1秒で3m移動
        move *= moveSpeed * deltaTime;
        auto pos = camera3D->GetGameObject()->GetTransform()->GetPos();
        pos += move;
        camera3D->SetPos(pos);
    }
}

void MEngine::Draw()
{
    graphicsEngine.Render(camera2D, camera3D);
}