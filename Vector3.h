#pragma once
#include <DirectXMath.h>
#include <cassert>
#include <stdexcept>
#include <sstream>
#include <string>

using namespace DirectX;

class Vector3
{
public:
	Vector3() : vec(XMVectorZero()) {}
	Vector3(float x, float y, float z) : vec(XMVectorSet(x, y, z, 0.0f)) {}
	explicit Vector3(XMVECTOR v) : vec(v) {}

	float GetX() const { return XMVectorGetX(vec); }
	float GetY() const { return XMVectorGetY(vec); }
	float GetZ() const { return XMVectorGetZ(vec); }

	void SetX(float x) { vec = XMVectorSetX(vec, x); }
	void SetY(float y) { vec = XMVectorSetY(vec, y); }
	void SetZ(float z) { vec = XMVectorSetZ(vec, z); }

	Vector3 operator +(const Vector3& other) const { return Vector3(XMVectorAdd(vec, other.vec)); }
	Vector3 operator -(const Vector3& other) const { return Vector3(XMVectorSubtract(vec, other.vec)); }
	Vector3 operator *(const float scalar) const { return Vector3(XMVectorScale(vec, scalar)); }
	Vector3 operator *(const Vector3& other) const { return Vector3(XMVectorMultiply(vec, other.vec)); }
	Vector3 operator /(const float scalar) const
	{
		if (scalar == 0.0f) { ::OutputDebugStringA("0で除算しようとしました。大きさ0のベクトルを返します\n"); return Zero(); }
		return Vector3(XMVectorScale(vec, 1.0f / scalar));
	}
	Vector3 operator /(const Vector3& other) const
	{
		XMVECTOR zeroCheck = XMVectorEqual(other.vec, XMVectorZero());
		if (XMVector4Equal(zeroCheck, XMVectorTrueInt())) { ::OutputDebugStringA("Vector3のいずれかの要素で0除算しようとしました。大きさ0のベクトルを返します\n"); return Zero(); }
		return Vector3(XMVectorDivide(vec, other.vec));
	}
	Vector3 operator +=(const Vector3& other) { vec = XMVectorAdd(vec, other.vec); return *this; }
	Vector3 operator -=(const Vector3& other) { vec = XMVectorSubtract(vec, other.vec); return *this; }
	Vector3 operator *=(const float scalar) { vec = XMVectorScale(vec, scalar); return *this; }
	Vector3 operator *=(const Vector3& other) { vec = XMVectorMultiply(vec, other.vec); return *this; }
	Vector3 operator /=(const float scalar)
	{
		if (scalar == 0.0f) {
			::OutputDebugStringA("0で除算しようとしました。大きさ0のベクトルを返します\n"); vec = XMVectorZero();
		}
		else {
			vec = XMVectorScale(vec, 1.0f / scalar);
		}
		return *this;
	}
	Vector3 operator /=(const Vector3& other)
	{
		XMVECTOR zeroCheck = XMVectorEqual(other.vec, XMVectorZero());
		if (XMVector4Equal(zeroCheck, XMVectorTrueInt())) {
			::OutputDebugStringA("Vector3のいずれかの要素で0除算しようとしました。大きさ0のベクトルを返します\n"); vec = XMVectorZero();
		}
		else {
			vec = XMVectorDivide(vec, other.vec);
		}
		return *this;
	}

	float Dot(const Vector3& other) const { return XMVectorGetX(XMVector3Dot(vec, other.vec)); }
	Vector3 Cross(const Vector3& other) const { return Vector3(XMVector3Cross(vec, other.vec)); }
	float Length() const { return XMVectorGetX(XMVector3Length(vec)); }
	float LengthSquared() const { return XMVectorGetX(XMVector3LengthSq(vec)); }
	Vector3 Normalized() const { return Vector3(XMVector3Normalize(vec)); }

	std::string ToString() const
	{
		std::stringstream ss;
		ss << "(" << GetX() << ", " << GetY() << ", " << GetZ() << ")";
		return ss.str();
	}

	XMVECTOR ToXMVECTOR() const { return vec; }
	Vector3 Transform(const XMMATRIX& matrix) const { Vector3(XMVector3Transform(vec, matrix)); }

	// 内部に持っているXMVECTORが16アライメントなので、newするときは16バイト境界に確保する
	void* operator new(size_t size)
	{
		void* ptr = _aligned_malloc(size, 16);
		if (ptr == nullptr) { throw std::bad_alloc(); }
		assert((reinterpret_cast<uintptr_t>(ptr) % 16) == 0);	// アライメントを確認
		return ptr;
	}

	void operator delete(void* ptr)
	{
		if (ptr != nullptr) _aligned_free(ptr);
	}

	void* operator new[](size_t size)
	{
		void* ptr = _aligned_malloc(size, 16);
		if (ptr == nullptr) { throw std::bad_alloc(); }
		assert((reinterpret_cast<uintptr_t>(ptr) % 16) == 0);
		return ptr;
	}

	void operator delete[](void* ptr)
	{
		if (ptr != nullptr) _aligned_free(ptr);
	}

public:

	static Vector3 Zero() { return Vector3(0.0f, 0.0f, 0.0f); }
	static Vector3 One() { return Vector3(1.0f, 1.0f, 1.0f); }

	static float Angle(const Vector3& a, const Vector3& b)
	{
		float dot = a.Dot(b);
		float lengthProduct = a.Length() * b.Length();
		if (lengthProduct == 0.0f) { return 0.0f; }	// ゼロ除算防止

		float cosTheta = dot / lengthProduct;
		cosTheta = (std::max)(-1.0f, (std::min)(1.0f, cosTheta));
		return std::acos(cosTheta);
	}

	static float Distance(const Vector3& a, const Vector3& b)
	{
		return XMVectorGetX(XMVector3Length(XMVectorSubtract(a.vec, b.vec)));
	}

private:
	XMVECTOR vec;
};