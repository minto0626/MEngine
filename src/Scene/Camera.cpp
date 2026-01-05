#include "Camera.h"
#include "GameObject.h"

Camera::Camera(GameObject* owner, int updateOrder)
	: Component(owner, updateOrder)
{
	_transform = owner->GetTransform();
    _lastTransformMatrix = _transform->GetWorldMatrix();
    ForceUpdateMatrices();
}

void Camera::Init(ProjectionType projectionType, uint32_t viewportWidth, uint32_t viewportHeight)
{
	_projectionType = projectionType;
	_viewportWidth = viewportWidth;
	_viewportHeight = viewportHeight;
	_fov = DirectX::XM_PIDIV4;
	_aspectRaito = static_cast<float>(_viewportWidth) / static_cast<float>(_viewportHeight);
	_nearZ = 1.0f;
	_farZ = 100.0f;
	_viewMatrix.SetIdentity();
	_projectionMatrix.SetIdentity();
	_viewProjectionMatrix.SetIdentity();
    _lastTransformMatrix = _transform->GetWorldMatrix();
    ForceUpdateMatrices();
}

Matrix Camera::GetViewMatrix()
{
	// 何かしらのパラメータが変更されていたら再計算される
	Update(0.0f);
	return _viewMatrix;
}

Matrix Camera::GetProjectionMatrix()
{
	// 何かしらのパラメータが変更されていたら再計算される
	Update(0.0f);
	return _projectionMatrix;
}

Matrix Camera::GetViewProjectionMatrix()
{
	// 何かしらのパラメータが変更されていたら再計算される
	Update(0.0f);
	return _viewProjectionMatrix;
}

void Camera::Update(float deltaTime)
{
    // Transform の変化をチェックして、変化があれば行列を再計算する
    auto currentMatrix = _transform->GetWorldMatrix();
    if (currentMatrix == _lastTransformMatrix)
    {
        // 変化なし
        return;
    }
    else
    {
        // 変化あり
        _lastTransformMatrix = currentMatrix;
    }

    ForceUpdateMatrices();
}

void Camera::ForceUpdateMatrices()
{
	if (_projectionType == ProjectionType::Ortho)
	{
		// [2次元用]スクリーン座標に変換する行列
		Vector3 scaleSize(2.0f / _viewportWidth, -2.0f / _viewportHeight, 1.0f);
		Matrix scale = Matrix::Scaling(scaleSize.GetX(), scaleSize.GetY(), 1.0f);
		Matrix offset = Matrix::Translation(-1.0, 1.0f, 0.0f);

		Matrix translation = Matrix::Translation(-_transform->GetPos().GetX(), -_transform->GetPos().GetY(), 0.0f);
		Matrix rotation = Matrix::RotationRollPitchYaw(_transform->GetRot().GetX(), _transform->GetRot().GetY(), _transform->GetRot().GetZ());

		_viewProjectionMatrix = translation * rotation * scale * offset;
	}
	else if (_projectionType == ProjectionType::Perspective)
	{
		// [3次元用]スクリーン座標に変換する行列
		_viewMatrix.MakeLookAt(_transform->GetPos(), _transform->GetPos() + _transform->GetForward().Normalized(), _transform->GetUp());

		_aspectRaito = static_cast<float>(_viewportWidth) / static_cast<float>(_viewportHeight);
		_projectionMatrix.MakeProjectionMatrix(_fov, _aspectRaito, _nearZ, _farZ);

		_viewProjectionMatrix = _viewMatrix * _projectionMatrix;
	}
}
