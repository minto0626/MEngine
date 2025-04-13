#include "Camera.h"

void Camera::Init(uint32_t viewportWidth, uint32_t viewportHeight)
{
	_viewportWidth = viewportWidth;
	_viewportHeight = viewportHeight;
}

void Camera::SetPos(Vector3 pos)
{
	_transform.SetPos(pos);
}

void Camera::SetRot(Vector3 rot)
{
	_transform.SetRot(rot);
}

Matrix Camera::GetViewMatrix() const
{
	// [2次元用]スクリーン座標に変換する行列
	Vector3 scaleSize(2.0f / _viewportWidth, -2.0f / _viewportHeight, 1.0f);
	Matrix scale = Matrix::Scaling(scaleSize.GetX(), scaleSize.GetY(), 1.0f);
	Matrix offset = Matrix::Translation(-1.0, 1.0f, 0.0f);

	Matrix translation = Matrix::Translation(-_transform.GetPos().GetX(), -_transform.GetPos().GetY(), 0.0f);
	Matrix rotation = Matrix::RotationRollPitchYaw(_transform.GetRot().GetX(), _transform.GetRot().GetY(), _transform.GetRot().GetZ());

	return translation * rotation * scale * offset;
}