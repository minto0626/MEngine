#include "Transform.h"
#include <sstream>

using namespace DirectX;

Transform::Transform() :
	_pos(0.0f, 0.0f, 0.0f),
	_rot(0.0f, 0.0f, 0.0f),
	_scale(1.0f, 1.0f, 1.0f),
	_world(),
	_isDirty(false)
{
}

void Transform::UpdateMatrix()
{
	if (!_isDirty) { return; }

	Matrix scaleMatrix = Matrix::Scaling(_scale.GetX(), _scale.GetY(), _scale.GetZ());
	Matrix rotMatrix = Matrix::RotationRollPitchYaw(_rot.GetX(), _rot.GetY(), _rot.GetZ());
	Matrix transMatrix = Matrix::Translation(_pos.GetX(), _pos.GetY(), _pos.GetZ());
	_world = scaleMatrix * rotMatrix * transMatrix;

	_isDirty = false;
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