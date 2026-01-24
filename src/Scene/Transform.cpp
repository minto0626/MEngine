#include "Transform.h"
#include "GameObject.h"

#include <sstream>

using namespace DirectX;

static_assert(alignof(Transform) >= 16, "Transform must be 16-byte aligned!");

Transform::Transform(GameObject* owner, int updateOrder) :
	Component(owner, updateOrder),
	_pos(0.0f, 0.0f, 0.0f),
	_rot(0.0f, 0.0f, 0.0f, 1.0),
	_scale(1.0f, 1.0f, 1.0f),
	_world(),
	_isDirty(false)
{
}

void Transform::UpdateMatrix()
{
	if (!_isDirty) { return; }

	Matrix scaleMatrix = Matrix::Scaling(_scale.GetX(), _scale.GetY(), _scale.GetZ());
	Matrix rotMatrix = Matrix::RotationQuaternion(_rot);
	Matrix transMatrix = Matrix::Translation(_pos.GetX(), _pos.GetY(), _pos.GetZ());
	_world = scaleMatrix * rotMatrix * transMatrix;

	_isDirty = false;
}

Vector3 Transform::GetForward() const
{
	return _rot.Rotate(Vector3(0.0f, 0.0f, 1.0f));
}

Vector3 Transform::GetRight() const
{
	return _rot.Rotate(Vector3(1.0f, 0.0f, 0.0f));
}

Vector3 Transform::GetUp() const
{
	return _rot.Rotate(Vector3(0.0f, 1.0f, 0.0f));
}

Vector3 Transform::TransformPoint(const Vector3& point)
{
	UpdateMatrix();
	return _world.TransformPoint(point);
}

Vector3 Transform::TrasnformDirection(const Vector3& dir)
{
	UpdateMatrix();
	XMVECTOR v = dir.ToXMVECTOR();
	XMFLOAT3 ret;
	XMStoreFloat3(&ret, XMVector3TransformNormal(v, _world.ToXMMatrix()));
	return Vector3(ret);
}

std::string Transform::ToString() const
{
	std::stringstream ss;
	ss << "pos: " << _pos.ToString() << "\n";
	ss << "rot: " << _rot.ToString() << "(degrees)\n";
	ss << "scale: " << _scale.ToString() << "\n";
	return ss.str();
}
