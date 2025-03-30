#include "Transform.h"
#include <sstream>

Transform::Transform() :
	_pos(0.0f, 0.0f, 0.0f),
	_rot(0.0f, 0.0f, 0.0f),
	_scale(1.0f, 1.0f, 1.0f),
	_world(XMMatrixIdentity()),
	_isDirty(false)
{
}

void Transform::UpdateMatrix()
{
	if (!_isDirty) { return; }

	XMMATRIX scaleMatrix = XMMatrixScaling(_scale.GetX(), _scale.GetY(), _scale.GetZ());
	XMMATRIX rotMatrix = XMMatrixRotationRollPitchYaw(
		XMConvertToRadians(_rot.GetX()),
		XMConvertToRadians(_rot.GetY()),
		XMConvertToRadians(_rot.GetZ()));
	XMMATRIX transMatrix = XMMatrixTranslation(_pos.GetX(), _pos.GetY(), _pos.GetZ());
	_world = scaleMatrix * rotMatrix * transMatrix;

	_isDirty = false;
}

Vector3 Transform::TransformPoint(const Vector3& point)
{
	XMVECTOR v = point.ToXMVECTOR();
	XMFLOAT3 ret;
	XMStoreFloat3(&ret, XMVector3Transform(v, GetWorldMatrix()));
	return Vector3(ret);
}

Vector3 Transform::TrasnformDirection(const Vector3& dir)
{
	XMVECTOR v = dir.ToXMVECTOR();
	XMFLOAT3 ret;
	XMStoreFloat3(&ret, XMVector3TransformNormal(v, GetWorldMatrix()));
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

void* Transform::operator new(size_t size)
{
	void* ptr = _aligned_malloc(size, 16);
	if (ptr == nullptr) throw std::bad_alloc();
	return ptr;
}

void Transform::operator delete(void* ptr)
{
	if (ptr != nullptr) _aligned_free(ptr);
}

void* Transform::operator new[](size_t size)
{
	void* ptr = _aligned_malloc(size, 16);
	if (ptr == nullptr) throw std::bad_alloc();
	return ptr;
}

void Transform::operator delete[](void* ptr)
{
	if (ptr != nullptr) _aligned_free(ptr);
}