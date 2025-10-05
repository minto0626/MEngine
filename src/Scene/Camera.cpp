#include "Camera.h"
#include "GameObject.h"

Camera::Camera(GameObject* owner, int updateOrder)
	: Component(owner, updateOrder)
{
	_transform = owner->GetTransform();
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
	_isDirty = false;
}

void Camera::SetPos(Vector3 pos)
{
	_transform->SetPos(pos);
	_isDirty = true;
}

void Camera::SetTarget(Vector3 target)
{
	_target = target;
	_isDirty = true;
}

void Camera::SetRot(Quaternion rot)
{
	_transform->SetRot(rot);
	_isDirty = true;
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
	if (!_isDirty) { return; }

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
		_viewMatrix.MakeLookAt(_transform->GetPos(), _target, _transform->GetUp());

		_aspectRaito = static_cast<float>(_viewportWidth) / static_cast<float>(_viewportHeight);
		_projectionMatrix.MakeProjectionMatrix(_fov, _aspectRaito, _nearZ, _farZ);

		_viewProjectionMatrix = _viewMatrix * _projectionMatrix;
	}

	_isDirty = false;
}