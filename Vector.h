#pragma once
#include <DirectXMath.h>
#include <cassert>
#include <stdexcept>
#include <sstream>
#include <string>
#include "Debug.h"

using namespace DirectX;

class Vector2
{
private:
	XMFLOAT2 vec;

public:
	Vector2() : vec(0.0f, 0.0f) {}
	Vector2(float x, float y) : vec(x, y) {}
	explicit Vector2(const XMFLOAT2& v) : vec(v) {}

	float GetX() const { return vec.x; }
	float GetY() const { return vec.y; }

	void SetX(float x) { vec.x = x; }
	void SetY(float y) { vec.y = y; }

	Vector2 operator +(const Vector2& other) const
	{
		XMVECTOR v1 = XMLoadFloat2(&vec);
		XMVECTOR v2 = XMLoadFloat2(&other.vec);
		XMFLOAT2 ret;
		XMStoreFloat2(&ret, XMVectorAdd(v1, v2));
		return Vector2(ret);
	}
	Vector2 operator -(const Vector2& other) const
	{
		XMVECTOR v1 = XMLoadFloat2(&vec);
		XMVECTOR v2 = XMLoadFloat2(&other.vec);
		XMFLOAT2 ret;
		XMStoreFloat2(&ret, XMVectorSubtract(v1, v2));
		return Vector2(ret);
	}
	Vector2 operator *(float scalar) const
	{
		XMVECTOR v = XMLoadFloat2(&vec);
		XMFLOAT2 ret;
		XMStoreFloat2(&ret, XMVectorScale(v, scalar));
		return Vector2(ret);
	}
	Vector2 operator *(const Vector2& other) const
	{
		XMVECTOR v1 = XMLoadFloat2(&vec);
		XMVECTOR v2 = XMLoadFloat2(&other.vec);
		XMFLOAT2 ret;
		XMStoreFloat2(&ret, XMVectorMultiply(v1, v2));
		return Vector2(ret);
	}
	Vector2 operator /(float scalar) const
	{
		if (scalar == 0.0f) { Debug::LogError("0で除算しようとしました。大きさ0のベクトルを返します"); return Zero(); }
		XMVECTOR v = XMLoadFloat2(&vec);
		XMFLOAT2 ret;
		XMStoreFloat2(&ret, XMVectorScale(v, 1.0f / scalar));
		return Vector2(ret);
	}
	Vector2 operator /(const Vector2& other) const
	{
		XMVECTOR v1 = XMLoadFloat2(&vec);
		XMVECTOR v2 = XMLoadFloat2(&other.vec);
		XMVECTOR zeroCheck = XMVectorEqual(v2, XMVectorZero());
		if (XMVector2NotEqual(zeroCheck, XMVectorZero())) { Debug::LogError("Vector2のいずれかの要素で0除算しようとしました。大きさ0のベクトルを返します"); return Zero(); }
		XMFLOAT2 ret;
		XMStoreFloat2(&ret, XMVectorDivide(v1, v2));
		return Vector2(ret);
	}
	Vector2 operator +=(const Vector2& other)
	{
		XMVECTOR v1 = XMLoadFloat2(&vec);
		XMVECTOR v2 = XMLoadFloat2(&other.vec);
		XMStoreFloat2(&vec, XMVectorAdd(v1, v2));
		return *this;
	}
	Vector2 operator -=(const Vector2& other)
	{
		XMVECTOR v1 = XMLoadFloat2(&vec);
		XMVECTOR v2 = XMLoadFloat2(&other.vec);
		XMStoreFloat2(&vec, XMVectorSubtract(v1, v2));
		return *this;
	}
	Vector2 operator *=(float scalar)
	{
		XMVECTOR v = XMLoadFloat2(&vec);
		XMStoreFloat2(&vec, XMVectorScale(v, scalar));
		return *this;
	}
	Vector2 operator *=(const Vector2& other)
	{
		XMVECTOR v1 = XMLoadFloat2(&vec);
		XMVECTOR v2 = XMLoadFloat2(&other.vec);
		XMStoreFloat2(&vec, XMVectorMultiply(v1, v2));
		return *this;
	}
	Vector2 operator /=(float scalar)
	{
		if (scalar == 0.0f) {
			Debug::LogError("0で除算しようとしました。大きさ0のベクトルを返します");
			vec = {0.0f, 0.0f};
		}
		else {
			XMVECTOR v = XMLoadFloat2(&vec);
			XMStoreFloat2(&vec, XMVectorScale(v, 1.0f / scalar));
		}
		return *this;
	}
	Vector2 operator /=(const Vector2& other)
	{
		XMVECTOR v1 = XMLoadFloat2(&vec);
		XMVECTOR v2 = XMLoadFloat2(&other.vec);
		XMVECTOR zeroCheck = XMVectorEqual(v2, XMVectorZero());
		if (XMVector2NotEqual(zeroCheck, XMVectorZero())) {
			Debug::LogError("Vector2のいずれかの要素で0除算しようとしました。大きさ0のベクトルを返します");
			vec = {0.0f, 0.0f};
		}
		else {
			XMStoreFloat2(&vec, XMVectorDivide(v1, v2));
		}
		return *this;
	}

	float Length() const
	{
		XMVECTOR v = XMLoadFloat2(&vec);
		return XMVectorGetX(XMVector2Length(v));
	}
	float LengthSquared() const
	{
		XMVECTOR v = XMLoadFloat2(&vec);
		return XMVectorGetX(XMVector2LengthSq(v));
	}
	Vector2 Normalized() const
	{
		XMVECTOR v = XMLoadFloat2(&vec);
		XMFLOAT2 ret;
		XMStoreFloat2(&ret, XMVector2Normalize(v));
		return Vector2(ret);
	}
	float Dot(const Vector2& other) const
	{
		XMVECTOR v1 = XMLoadFloat2(&vec);
		XMVECTOR v2 = XMLoadFloat2(&other.vec);
		return XMVectorGetX(XMVector2Dot(v1, v2));
	}

	std::string ToString() const
	{
		std::stringstream ss;
		ss << "(" << vec.x << ", " << vec.y << ")";
		return ss.str();
	}

	const XMFLOAT2& ToXMFLOAT2() const { return vec; }

public:
	static Vector2 Zero() { return Vector2(0.0f, 0.0f); }
	static Vector2 One() { return Vector2(1.0f, 1.0f); }
};

class Vector3
{
private:
	XMFLOAT3 vec;

public:
	Vector3() : vec(0.0f, 0.0f, 0.0f) {}
	Vector3(float x, float y, float z) : vec(x, y, z) {}
	explicit Vector3(const XMFLOAT3& v) : vec(v) {}

	float GetX() const { return vec.x; }
	float GetY() const { return vec.y; }
	float GetZ() const { return vec.z; }

	void SetX(float x) { vec.x = x; }
	void SetY(float y) { vec.y = y; }
	void SetZ(float z) { vec.y = z; }

	Vector3 operator +(const Vector3& other) const
	{
		XMVECTOR v1 = XMLoadFloat3(&vec);
		XMVECTOR v2 = XMLoadFloat3(&other.vec);
		XMFLOAT3 ret;
		XMStoreFloat3(&ret, XMVectorAdd(v1, v2));
		return Vector3(ret);
	}
	Vector3 operator -(const Vector3& other) const
	{
		XMVECTOR v1 = XMLoadFloat3(&vec);
		XMVECTOR v2 = XMLoadFloat3(&other.vec);
		XMFLOAT3 ret;
		XMStoreFloat3(&ret, XMVectorSubtract(v1, v2));
		return Vector3(ret);
	}
	Vector3 operator *(const float scalar) const
	{
		XMVECTOR v = XMLoadFloat3(&vec);
		XMFLOAT3 ret;
		XMStoreFloat3(&ret, XMVectorScale(v, scalar));
		return Vector3(ret);
	}
	Vector3 operator *(const Vector3& other) const
	{
		XMVECTOR v1 = XMLoadFloat3(&vec);
		XMVECTOR v2 = XMLoadFloat3(&other.vec);
		XMFLOAT3 ret;
		XMStoreFloat3(&ret, XMVectorMultiply(v1, v2));
		return Vector3(ret);
	}
	Vector3 operator /(const float scalar) const
	{
		if (scalar == 0.0f) { Debug::LogError("0で除算しようとしました。大きさ0のベクトルを返します"); return Zero(); }
		XMVECTOR v = XMLoadFloat3(&vec);
		XMFLOAT3 ret;
		XMStoreFloat3(&ret, XMVectorScale(v, 1.0f / scalar));
		return Vector3(ret);
	}
	Vector3 operator /(const Vector3& other) const
	{
		XMVECTOR v1 = XMLoadFloat3(&vec);
		XMVECTOR v2 = XMLoadFloat3(&other.vec);
		XMVECTOR zeroCheck = XMVectorEqual(v2, XMVectorZero());
		if (XMVector3NotEqual(zeroCheck, XMVectorZero())) { Debug::LogError("Vector3のいずれかの要素で0除算しようとしました。大きさ0のベクトルを返します"); return Zero(); }
		XMFLOAT3 ret;
		XMStoreFloat3(&ret, XMVectorDivide(v1, v2));
		return Vector3(ret);
	}
	Vector3 operator +=(const Vector3& other)
	{
		XMVECTOR v1 = XMLoadFloat3(&vec);
		XMVECTOR v2 = XMLoadFloat3(&other.vec);
		XMStoreFloat3(&vec, XMVectorAdd(v1, v2));
		return *this;
	}
	Vector3 operator -=(const Vector3& other)
	{
		XMVECTOR v1 = XMLoadFloat3(&vec);
		XMVECTOR v2 = XMLoadFloat3(&other.vec);
		XMStoreFloat3(&vec, XMVectorSubtract(v1, v2));
		return *this;
	}
	Vector3 operator *=(const float scalar)
	{
		XMVECTOR v = XMLoadFloat3(&vec);
		XMStoreFloat3(&vec, XMVectorScale(v, scalar));
		return *this;
	}
	Vector3 operator *=(const Vector3& other)
	{
		XMVECTOR v1 = XMLoadFloat3(&vec);
		XMVECTOR v2 = XMLoadFloat3(&other.vec);
		XMStoreFloat3(&vec, XMVectorMultiply(v1, v2));
		return *this;
	}
	Vector3 operator /=(const float scalar)
	{
		if (scalar == 0.0f) {
			Debug::LogError("0で除算しようとしました。大きさ0のベクトルを返します");
			vec = {0.f, 0.0f, 0.0f};
		}
		else {
			XMVECTOR v = XMLoadFloat3(&vec);
			XMStoreFloat3(&vec, XMVectorScale(v, 1.0f / scalar));
		}
		return *this;
	}
	Vector3 operator /=(const Vector3& other)
	{
		XMVECTOR v1 = XMLoadFloat3(&vec);
		XMVECTOR v2 = XMLoadFloat3(&other.vec);
		XMVECTOR zeroCheck = XMVectorEqual(v2, XMVectorZero());
		if (XMVector3NotEqual(zeroCheck, XMVectorZero())) {
			Debug::LogError("Vector3のいずれかの要素で0除算しようとしました。大きさ0のベクトルを返します");
			vec = {0.0f, 0.0f, 0.0f };
		}
		else {
			XMStoreFloat3(&vec, XMVectorDivide(v1, v2));
		}
		return *this;
	}

	float Length() const
	{
		XMVECTOR v = XMLoadFloat3(&vec);
		return XMVectorGetX(XMVector3Length(v));
	}
	float LengthSquared() const
	{
		XMVECTOR v = XMLoadFloat3(&vec);
		return XMVectorGetX(XMVector3LengthSq(v));
	}
	Vector3 Normalized() const
	{
		XMVECTOR v = XMLoadFloat3(&vec);
		XMFLOAT3 ret;
		XMStoreFloat3(&ret, XMVector3Normalize(v));
		return Vector3(ret);
	}
	void Normalize()
	{
		XMVECTOR v = XMLoadFloat3(&vec);
		XMStoreFloat3(&vec, XMVector3Normalize(v));
	}

	float Dot(const Vector3& other) const
	{
		XMVECTOR v1 = XMLoadFloat3(&vec);
		XMVECTOR v2 = XMLoadFloat3(&other.vec);
		return XMVectorGetX(XMVector3Dot(v1, v2));
	}
	Vector3 Cross(const Vector3& other) const
	{
		XMVECTOR v1 = XMLoadFloat3(&vec);
		XMVECTOR v2 = XMLoadFloat3(&other.vec);
		XMFLOAT3 ret;
		XMStoreFloat3(&ret, XMVector3Cross(v1, v2));
		return Vector3(ret);
	}

	std::string ToString() const
	{
		std::stringstream ss;
		ss << "(" << GetX() << ", " << GetY() << ", " << GetZ() << ")";
		return ss.str();
	}

	XMVECTOR ToXMVECTOR() const
	{
		return XMLoadFloat3(&vec);
	}
	Vector3 Transform(const XMMATRIX& matrix) const
	{
		XMVECTOR transform = XMVector3Transform(XMLoadFloat3(&vec), matrix);
		XMFLOAT3 ret;
		XMStoreFloat3(&ret, transform);
		return Vector3(ret);
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
		XMVECTOR v1 = XMLoadFloat3(&a.vec);
		XMVECTOR v2 = XMLoadFloat3(&b.vec);
		return XMVectorGetX(XMVector3Length(XMVectorSubtract(v1, v2)));
	}
};

class Vector4
{
private:
	XMFLOAT4 vec;

public:
	Vector4() : vec(0.0f, 0.0f, 0.0f, 0.0f) {}
	Vector4(float x, float y, float z, float w) : vec(x, y, z, w) {}
	explicit Vector4(const XMFLOAT4& v) : vec(v) {}

	float GetX() const { return vec.x; }
	float GetY() const { return vec.y; }
	float GetZ() const { return vec.z; }
	float GetW() const { return vec.w; }

	void SetX(float x) { vec.x = x; }
	void SetY(float y) { vec.y = y; }
	void SetZ(float z) { vec.z = z; }
	void SetW(float w) { vec.w = w; }

	Vector4 operator +(const Vector4& other) const
	{
		XMVECTOR v1 = XMLoadFloat4(&vec);
		XMVECTOR v2 = XMLoadFloat4(&other.vec);
		XMFLOAT4 result;
		XMStoreFloat4(&result, XMVectorAdd(v1, v2));
		return Vector4(result);
	}
	Vector4 operator -(const Vector4& other) const
	{
		XMVECTOR v1 = XMLoadFloat4(&vec);
		XMVECTOR v2 = XMLoadFloat4(&other.vec);
		XMFLOAT4 result;
		XMStoreFloat4(&result, XMVectorSubtract(v1, v2));
		return Vector4(result);
	}
	Vector4 operator *(float scalar) const
	{
		XMVECTOR v = XMLoadFloat4(&vec);
		XMFLOAT4 result;
		XMStoreFloat4(&result, XMVectorScale(v, scalar));
		return Vector4(result);
	}
	Vector4 operator *(const Vector4& other) const
	{
		XMVECTOR v1 = XMLoadFloat4(&vec);
		XMVECTOR v2 = XMLoadFloat4(&other.vec);
		XMFLOAT4 result;
		XMStoreFloat4(&result, XMVectorMultiply(v1, v2));
		return Vector4(result);
	}
	Vector4 operator /(float scalar) const
	{
		if (scalar == 0.0f) {
			Debug::LogError("0で除算しようとしました。大きさ0のベクトルを返します");
			return Zero();
		}
		XMVECTOR v = XMLoadFloat4(&vec);
		XMFLOAT4 result;
		XMStoreFloat4(&result, XMVectorScale(v, 1.0f / scalar));
		return Vector4(result);
	}
	Vector4 operator /(const Vector4& other) const
	{
		XMVECTOR v1 = XMLoadFloat4(&vec);
		XMVECTOR v2 = XMLoadFloat4(&other.vec);
		XMVECTOR zeroCheck = XMVectorEqual(v2, XMVectorZero());
		if (XMVector4NotEqual(zeroCheck, XMVectorZero())) {
			Debug::LogError("Vector4のいずれかの要素で0除算しようとしました。大きさ0のベクトルを返します");
			return Zero();
		}
		XMFLOAT4 result;
		XMStoreFloat4(&result, XMVectorDivide(v1, v2));
		return Vector4(result);
	}
	Vector4& operator +=(const Vector4& other)
	{
		XMVECTOR v1 = XMLoadFloat4(&vec);
		XMVECTOR v2 = XMLoadFloat4(&other.vec);
		XMStoreFloat4(&vec, XMVectorAdd(v1, v2));
		return *this;
	}
	Vector4& operator -=(const Vector4& other)
	{
		XMVECTOR v1 = XMLoadFloat4(&vec);
		XMVECTOR v2 = XMLoadFloat4(&other.vec);
		XMStoreFloat4(&vec, XMVectorSubtract(v1, v2));
		return *this;
	}
	Vector4 operator *=(float scalar)
	{
		XMVECTOR v = XMLoadFloat4(&vec);
		XMStoreFloat4(&vec, XMVectorScale(v, scalar));
		return *this;
	}
	Vector4& operator *=(const Vector4& other)
	{
		XMVECTOR v1 = XMLoadFloat4(&vec);
		XMVECTOR v2 = XMLoadFloat4(&other.vec);
		XMStoreFloat4(&vec, XMVectorMultiply(v1, v2));
		return *this;
	}
	Vector4& operator /(float scalar)
	{
		if (scalar == 0.0f) {
			Debug::LogError("0で除算しようとしました。大きさ0のベクトルを返します");
			vec = { 0.0f, 0.0f, 0.0f, 0.0f };
			return *this;
		}
		XMVECTOR v = XMLoadFloat4(&vec);
		XMStoreFloat4(&vec, XMVectorScale(v, 1.0f / scalar));
		return *this;
	}
	Vector4& operator /=(const Vector4& other)
	{
		XMVECTOR v1 = XMLoadFloat4(&vec);
		XMVECTOR v2 = XMLoadFloat4(&other.vec);
		XMVECTOR zeroCheck = XMVectorEqual(v2, XMVectorZero());
		if (XMVector4NotEqual(zeroCheck, XMVectorZero())) {
			Debug::LogError("Vector4のいずれかの要素で0除算しようとしました。大きさ0のベクトルを返します");
			vec = { 0.0f, 0.0f, 0.0f, 0.0f };
		}
		else {
			XMStoreFloat4(&vec, XMVectorDivide(v1, v2));
		}
		return *this;
	}

	float Length() const
	{
		XMVECTOR v = XMLoadFloat4(&vec);
		return XMVectorGetX(XMVector4Length(v));
	}
	float LengthSquared() const {
		XMVECTOR v = XMLoadFloat4(&vec);
		return XMVectorGetX(XMVector4LengthSq(v));
	}
	Vector4 Normalized() const
	{
		XMVECTOR v = XMLoadFloat4(&vec);
		XMFLOAT4 result;
		XMStoreFloat4(&result, XMVector4Normalize(v));
		return Vector4(result);
	}

	std::string ToString() const
	{
		std::stringstream ss;
		ss << "(" << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w << ")";
		return ss.str();
	}

	XMVECTOR ToXMVECTOR() const
	{
		return XMLoadFloat4(&vec);
	}

public:
public:
	static Vector4 Zero() { return Vector4(0.0f, 0.0f, 0.0f, 0.0f); }

};