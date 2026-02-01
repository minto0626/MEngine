#include "MEngine.h"
#include <string>
#include <assert.h>
#include <filesystem>

#include "Utility/Debug.h"
#include "Utility/GameTime.h"
#include "Sprite/Sprite.h"
#include "Material/Material.h"
#include "Resources/ConstantBuffer.h"
#include "Renderer/SpriteRenderer.h"
#include "Scene/GameObject.h"
#include "Scene/Camera.h"
#include "Scene/Light.h"

GUISystem MEngine::guiSystem;

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

    // エンジンの一時ファイルを保存する場所を生成
    if (!std::filesystem::exists(MEngine::CacheDirectory))
    {
        std::filesystem::create_directory(MEngine::CacheDirectory);
    }

    // グラフィックスエンジン初期化
    if (!graphicsEngine.Initialize(hwnd, windowSize))
    {
        return false;
    }

	// シーン生成
	_scene = std::make_unique<Scene>();
	_scene->Init(&graphicsEngine);

    // リソース読み込み
    graphicsEngine.LoadContent();

    input.Init(hInstancce, hwnd);
    time.Init();

	// ゲームオブジェクト生成

    // 2Dカメラ
    {
        camera2D = _scene->CreateGameObject("Camera2D")->AddComponent<Camera>();
        camera2D->Init(Camera::ProjectionType::Ortho, windowSize.cx, windowSize.cy);
        camera2D->GetGameObject()->GetTransform()->SetPos({ 0.0f, 0.0f, 0.0f });
	}
    // 3Dカメラ
    {
        camera3D = _scene->CreateGameObject("Camera3D")->AddComponent<Camera>();
        camera3D->Init(Camera::ProjectionType::Perspective, windowSize.cx, windowSize.cy);
        camera3D->GetGameObject()->GetTransform()->SetPos({ 0.0f, 7.5f, -11.0f });
        camera3D->GetGameObject()->GetTransform()->SetRot(Quaternion::FromEulerAngles(30.0f, 0.0f, 0.0f));
    }
    // ライト
    {
        directionalLight = _scene->CreateGameObject("DirectionalLight")->AddComponent<Light>();
        directionalLight->GetGameObject()->GetTransform()->SetRot(Quaternion::FromEulerAngles(45.0f, -45.0f, 0.0f));
    }

    // スプライト
    {
        auto obj = _scene->CreateGameObject("cat");
        sample_objects.push_back(obj);
        obj->GetTransform()->SetPos({ 1920 - 64, 1080 - 64, 0.0f });
        obj->GetTransform()->SetRot(Quaternion::FromEulerAngles(0.0f, 0.0f, 0.0f));
        obj->GetTransform()->SetScale({ 1.0f, 1.25f, 1.0f });
        auto spriteRenderer = obj->AddComponent<Graphics::SpriteRenderer>();
        Graphics::Sprite* sprite = graphicsEngine.GetSprite("testSprite");
        spriteRenderer->SetSprite(sprite);
        Graphics::Material* material = graphicsEngine.GetMaterial("Assets/texture/cat_sprite_mat.mat");
	    auto texture = graphicsEngine.GetTexture("Assets/texture/free_cat.png");
        UINT texIdx = graphicsEngine.GetRootParameterIndex("mainTex", *material);
	    material->SetTexture(texIdx, texture);
	    spriteRenderer->SetMaterial(material);
    }

	// メッシュ
    {
        auto obj = _scene->CreateGameObject("tea_pot");
        sample_objects.push_back(obj);
        obj->GetTransform()->SetPos({ 2.0f, 0.0f, 0.0f });
	    auto meshRenderer = obj->AddComponent<Graphics::MeshRenderer>();
	    auto mesh = graphicsEngine.GetMesh(L"Assets/3D/samples/teapot/teapot.fbx");
	    meshRenderer->SetMesh(mesh);
		Graphics::Material* material = graphicsEngine.GetMaterial("Assets/3D/samples/teapot/teapot_mat.mat");
        auto texture = graphicsEngine.GetTexture("Assets/3D/samples/teapot/default.png");
		UINT texIdx = graphicsEngine.GetRootParameterIndex("mainTex", *material);
		material->SetTexture(texIdx, texture);
		meshRenderer->SetMaterial(material);
    }
    {
        auto obj = _scene->CreateGameObject("cube");
        sample_objects.push_back(obj);
        obj->GetTransform()->SetPos({ 0.0f, 0.0f, 0.0f });
        obj->GetTransform()->SetRot(Quaternion::FromEulerAngles(0.0f, 45.0f, 0.0f));
        auto meshRenderer = obj->AddComponent<Graphics::MeshRenderer>();
        auto mesh = graphicsEngine.GetMesh(L"Assets/3D/samples/cube/cube.fbx");
        meshRenderer->SetMesh(mesh);
        Graphics::Material* material = graphicsEngine.GetMaterial("Assets/3D/samples/cube/cube_mat.mat");
        auto texture = graphicsEngine.GetTexture("Assets/3D/samples/cube/default.png");
        UINT texIdx = graphicsEngine.GetRootParameterIndex("mainTex", *material);
        material->SetTexture(texIdx, texture);
        meshRenderer->SetMaterial(material);
    }
    {
        auto obj = _scene->CreateGameObject("horse_statue");
        sample_objects.push_back(obj);
        obj->GetTransform()->SetPos({ 0.0f, 0.0f, 0.0f });
        auto meshRenderer = obj->AddComponent<Graphics::MeshRenderer>();
        auto mesh = graphicsEngine.GetMesh(L"Assets/3D/samples/horse/horse_statue_01.fbx");
        meshRenderer->SetMesh(mesh);
        Graphics::Material* material = graphicsEngine.GetMaterial("Assets/3D/samples/horse/horse_mat.mat");
        auto texture = graphicsEngine.GetTexture("Assets/3D/samples/horse/horse_statue_01_diff.jpg");
        UINT texIdx = graphicsEngine.GetRootParameterIndex("mainTex", *material);
        material->SetTexture(texIdx, texture);
        meshRenderer->SetMaterial(material);
    }
    {
        auto obj = _scene->CreateGameObject("rubber_duck");
        sample_objects.push_back(obj);
        obj->GetTransform()->SetPos({ 3.0f, 0.0f, -1.5f });
        obj->GetTransform()->SetRot(Quaternion::FromEulerAngles(0.0f, 30.0f, 0.0f));
        auto meshRenderer = obj->AddComponent<Graphics::MeshRenderer>();
        auto mesh = graphicsEngine.GetMesh(L"Assets/3D/samples/duck_toy/rubber_duck_toy.fbx");
        meshRenderer->SetMesh(mesh);
        Graphics::Material* material = graphicsEngine.GetMaterial("Assets/3D/samples/duck_toy/duck_mat.mat");
        auto texture = graphicsEngine.GetTexture("Assets/3D/samples/duck_toy/rubber_duck_toy_diff.jpg");
        UINT texIdx = graphicsEngine.GetRootParameterIndex("mainTex", *material);
        material->SetTexture(texIdx, texture);
        meshRenderer->SetMaterial(material);
    }
    {
        auto obj = _scene->CreateGameObject("lounge_chair");
        sample_objects.push_back(obj);
        obj->GetTransform()->SetPos({ -5.0f, 0.0f, 1.0f });
        obj->GetTransform()->SetRot(Quaternion::FromEulerAngles(0.0f, -20.0f, 0.0f));
        auto meshRenderer = obj->AddComponent<Graphics::MeshRenderer>();
        auto mesh = graphicsEngine.GetMesh(L"Assets/3D/samples/chair/mid_century_lounge_chair.fbx");
        meshRenderer->SetMesh(mesh);
        Graphics::Material* material = graphicsEngine.GetMaterial("Assets/3D/samples/chair/chair_mat.mat");
        auto texture = graphicsEngine.GetTexture("Assets/3D/samples/chair/mid_century_lounge_chair_diff.jpg");
        UINT texIdx = graphicsEngine.GetRootParameterIndex("mainTex", *material);
        material->SetTexture(texIdx, texture);
        meshRenderer->SetMaterial(material);
    }
    {
        auto obj = _scene->CreateGameObject("checker_floar");
        sample_objects.push_back(obj);
        obj->GetTransform()->SetPos({ 0.0f, 0.0f, 0.0f });
        auto meshRenderer = obj->AddComponent<Graphics::MeshRenderer>();
        auto mesh = graphicsEngine.GetMesh(L"Assets/3D/samples/plane/plane.fbx");
        meshRenderer->SetMesh(mesh);
        Graphics::Material* material = graphicsEngine.GetMaterial("Assets/3D/samples/plane/floor_mat.mat");
        auto texture = graphicsEngine.GetTexture("Assets/3D/samples/plane/checkered_pavement_tiles_diff.jpg");
        UINT texIdx = graphicsEngine.GetRootParameterIndex("mainTex", *material);
        material->SetTexture(texIdx, texture);
        meshRenderer->SetMaterial(material);
    }
    {
        auto obj = _scene->CreateGameObject("roza");
        sample_objects.push_back(obj);
        obj->GetTransform()->SetPos({ -2.0f, 0.0f, 5.0f });
        obj->GetTransform()->SetRot(Quaternion::FromEulerAngles(0.0f, -30.0f, 0.0f));
        auto meshRenderer = obj->AddComponent<Graphics::MeshRenderer>();
        auto mesh = graphicsEngine.GetMesh(L"Assets/3D/samples/sculpture_bust_of_roza_loewenfeld/scene.gltf");
        meshRenderer->SetMesh(mesh);
        Graphics::Material* material = graphicsEngine.GetMaterial("Assets/3D/samples/sculpture_bust_of_roza_loewenfeld/roza_mat.mat");
        auto texture = graphicsEngine.GetTexture("Assets/3D/samples/sculpture_bust_of_roza_loewenfeld/textures/defaultMat_diffuse.jpeg");
        UINT texIdx = graphicsEngine.GetRootParameterIndex("mainTex", *material);
        material->SetTexture(texIdx, texture);
        meshRenderer->SetMaterial(material);
    }
    {
        auto obj = _scene->CreateGameObject("marble_bust_01");
        sample_objects.push_back(obj);
        obj->GetTransform()->SetPos({ 5.0f, 0.0f, 4.0f });
        auto meshRenderer = obj->AddComponent<Graphics::MeshRenderer>();
        auto mesh = graphicsEngine.GetMesh(L"Assets/3D/samples/marble_bust_01/marble_bust_01.fbx");
        meshRenderer->SetMesh(mesh);
        Graphics::Material* material = graphicsEngine.GetMaterial("Assets/3D/samples/marble_bust_01/marble_bust_01_mat.mat");
        auto texture = graphicsEngine.GetTexture("Assets/3D/samples/marble_bust_01/textures/marble_bust_01_diff.jpg");
        UINT texIdx = graphicsEngine.GetRootParameterIndex("mainTex", *material);
        material->SetTexture(texIdx, texture);
        meshRenderer->SetMaterial(material);
    }
    {
        auto obj = _scene->CreateGameObject("potted_plant_04");
        sample_objects.push_back(obj);
        obj->GetTransform()->SetPos({ 5.0f, 0.0f, -3.0f });
        auto meshRenderer = obj->AddComponent<Graphics::MeshRenderer>();
        auto mesh = graphicsEngine.GetMesh(L"Assets/3D/samples/potted_plant_04/potted_plant_04.fbx");
        meshRenderer->SetMesh(mesh);
        Graphics::Material* material = graphicsEngine.GetMaterial("Assets/3D/samples/potted_plant_04/potted_plant_04_mat.mat");
        auto texture = graphicsEngine.GetTexture("Assets/3D/samples/potted_plant_04/textures/potted_plant_04_diff.jpg");
        UINT texIdx = graphicsEngine.GetRootParameterIndex("mainTex", *material);
        material->SetTexture(texIdx, texture);
        meshRenderer->SetMaterial(material);
    }
    {
        auto obj = _scene->CreateGameObject("round_wooden_table_01");
        sample_objects.push_back(obj);
        obj->GetTransform()->SetPos({ -2.0f, 0.0f, -4.0f });
        auto meshRenderer = obj->AddComponent<Graphics::MeshRenderer>();
        auto mesh = graphicsEngine.GetMesh(L"Assets/3D/samples/round_wooden_table_01/round_wooden_table_01.fbx");
        meshRenderer->SetMesh(mesh);
        Graphics::Material* material = graphicsEngine.GetMaterial("Assets/3D/samples/round_wooden_table_01/round_wooden_table_01_mat.mat");
        auto texture = graphicsEngine.GetTexture("Assets/3D/samples/round_wooden_table_01/textures/round_wooden_table_01_diff.jpg");
        UINT texIdx = graphicsEngine.GetRootParameterIndex("mainTex", *material);
        material->SetTexture(texIdx, texture);
        meshRenderer->SetMaterial(material);
    }
    {
        auto obj = _scene->CreateGameObject("root_object");
        sample_objects.push_back(obj);
        obj->GetTransform()->SetPos({ 0.0f, 3.0f, 0.0f });
    }
    {
        auto obj = _scene->CreateGameObject("table_top_root");
        sample_objects.push_back(obj);
        obj->GetTransform()->SetPos({ 0.0f, 1.0f, 0.0f });
    }

    // 親を設定する
    {
        // root_objectにcube, cubeにtea_potをぶら下げる
        auto obj1 = sample_objects[11];
        auto obj2 = sample_objects[1];
        auto obj3 = sample_objects[2];
        obj3->GetTransform()->SetParent(obj1->GetTransform());
        obj2->GetTransform()->SetParent(obj3->GetTransform());

        // round_wooden_table_01にtable_top_root, table_top_rootにhorse_statueをぶら下げる
        auto obj4 = sample_objects[12];
        auto obj5 = sample_objects[10];
        auto obj6 = sample_objects[3];
        obj4->GetTransform()->SetParent(obj5->GetTransform());
        obj6->GetTransform()->SetParent(obj4->GetTransform());
    }

    for (auto& obj : sample_objects)
    {
        Debug::Log("オブジェクトの名前 : " + (obj->GetName()));
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

    sample_objects[1]->GetTransform()->SetRot(
        sample_objects[1]->GetTransform()->GetRot() * Quaternion::FromEulerAngles(0.0f, 0.25f, 0.0f));
    sample_objects[2]->GetTransform()->SetRot(
        sample_objects[2]->GetTransform()->GetRot() * Quaternion::FromEulerAngles(0.3f, 0.3f, 0.3f));
    sample_objects[3]->GetTransform()->SetRot(
        sample_objects[3]->GetTransform()->GetRot() * Quaternion::FromEulerAngles(0.0f, 0.1f, 0.0f));

    Vector3 move;

	move = Vector3::Zero();
    if (input.IsButtonDown(0, DIK_W))
    {
		move.SetZ(1);
    }
    if (input.IsButtonDown(0, DIK_S))
    {
		move.SetZ(-1);
	}
    if (input.IsButtonDown(0, DIK_A))
    {
        move.SetX(-1);
    }
    if (input.IsButtonDown(0, DIK_D))
    {
        move.SetX(1);
    }
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
        camera3D->GetGameObject()->GetTransform()->SetPos(pos);
    }

    if (input.IsButtonDown(0, DIK_L))
    {
        directionalLight->GetGameObject()->GetTransform()->SetRot(
            directionalLight->GetGameObject()->GetTransform()->GetRot() * Quaternion::FromEulerAngles(0.0f, 0.25f, 0.0f));
    }
}

void MEngine::Draw()
{
    _scene->Draw();
}
