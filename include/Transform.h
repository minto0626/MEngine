#pragma once
#include "Vector.h"
#include "Matrix.h"
#include <string>

class Transform
{
private:
	Vector3 _pos;
	Quaternion _rot;
	Vector3 _scale;
	Matrix _world;
	bool _isDirty;

	void UpdateMatrix();

public:
	Transform();

	void SetPos(const Vector3& pos) { _pos = pos; _isDirty = true; }
	void SetRot(const Quaternion& rot) { _rot = rot; _isDirty = true; }
	void SetScale(const Vector3& scale) { _scale = scale; _isDirty = true; }

	Vector3 GetPos() const { return _pos; }
	Quaternion GetRot() const { return _rot; }
	Vector3 GetScale() const { return _scale; }
	Matrix GetWorldMatrix() { UpdateMatrix(); return _world; }

	Vector3 TransformPoint(const Vector3& point);
	Vector3 TrasnformDirection(const Vector3& dir);
	std::string ToString()const;

	void* operator new(size_t size);
	void operator delete(void* ptr);
	void* operator new[](size_t size);
	void operator delete[](void* ptr);
};