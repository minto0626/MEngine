#pragma once
#include <DirectXMath.h>
#include <cassert>
#include <stdexcept>
#include <sstream>
#include <string>
#include "Debug.h"

class Vector2
{
private:
	DirectX::XMFLOAT2 vec;

public:
	Vector2() : vec(0.0f, 0.0f) {}
	Vector2(float x, float y) : vec(x, y) {}
	explicit Vector2(const DirectX::XMFLOAT2& v) : vec(v) {}

	float GetX() const { return vec.x; }
	float GetY() const { return vec.y; }

	void SetX(float x) { vec.x = x; }
	void SetY(float y) { vec.y = y; }

	Vector2 operator +(const Vector2& other) const
	{
		DirectX::XMVECTOR v1 = DirectX::XMLoadFloat2(&vec);
		DirectX::XMVECTOR v2 = DirectX::XMLoadFloat2(&other.vec);
		DirectX::XMFLOAT2 ret;
		DirectX::XMStoreFloat2(&ret, DirectX::XMVectorAdd(v1, v2));
		return Vector2(ret);
	}
	Vector2 operator -(const Vector2& other) const
	{
		DirectX::XMVECTOR v1 = DirectX::XMLoadFloat2(&vec);
		DirectX::XMVECTOR v2 = DirectX::XMLoadFloat2(&other.vec);
		DirectX::XMFLOAT2 ret;
		DirectX::XMStoreFloat2(&ret, DirectX::XMVectorSubtract(v1, v2));
		return Vector2(ret);
	}
	Vector2 operator *(float scalar) const
	{
		DirectX::XMVECTOR v = DirectX::XMLoadFloat2(&vec);
		DirectX::XMFLOAT2 ret;
		DirectX::XMStoreFloat2(&ret, DirectX::XMVectorScale(v, scalar));
		return Vector2(ret);
	}
	Vector2 operator *(const Vector2& other) const
	{
		DirectX::XMVECTOR v1 = DirectX::XMLoadFloat2(&vec);
		DirectX::XMVECTOR v2 = DirectX::XMLoadFloat2(&other.vec);
		DirectX::XMFLOAT2 ret;
		DirectX::XMStoreFloat2(&ret, DirectX::XMVectorMultiply(v1, v2));
		return Vector2(ret);
	}
	Vector2 operator /(float scalar) const
	{
		if (scalar == 0.0f) { Debug::LogError("0で除算しようとしました。大きさ0のベクトルを返します"); return Zero(); }
		DirectX::XMVECTOR v = DirectX::XMLoadFloat2(&vec);
		DirectX::XMFLOAT2 ret;
		DirectX::XMStoreFloat2(&ret, DirectX::XMVectorScale(v, 1.0f / scalar));
		return Vector2(ret);
	}
	Vector2 operator /(const Vector2& other) const
	{
		DirectX::XMVECTOR v1 = DirectX::XMLoadFloat2(&vec);
		DirectX::XMVECTOR v2 = DirectX::XMLoadFloat2(&other.vec);
		DirectX::XMVECTOR zeroCheck = DirectX::XMVectorEqual(v2, DirectX::XMVectorZero());
		if (DirectX::XMVector2NotEqual(zeroCheck, DirectX::XMVectorZero())) { Debug::LogError("Vector2のいずれかの要素で0除算しようとしました。大きさ0のベクトルを返します"); return Zero(); }
		DirectX::XMFLOAT2 ret;
		DirectX::XMStoreFloat2(&ret, DirectX::XMVectorDivide(v1, v2));
		return Vector2(ret);
	}
	Vector2 operator +=(const Vector2& other)
	{
		DirectX::XMVECTOR v1 = DirectX::XMLoadFloat2(&vec);
		DirectX::XMVECTOR v2 = DirectX::XMLoadFloat2(&other.vec);
		DirectX::XMStoreFloat2(&vec, DirectX::XMVectorAdd(v1, v2));
		return *this;
	}
	Vector2 operator -=(const Vector2& other)
	{
		DirectX::XMVECTOR v1 = DirectX::XMLoadFloat2(&vec);
		DirectX::XMVECTOR v2 = DirectX::XMLoadFloat2(&other.vec);
		DirectX::XMStoreFloat2(&vec, DirectX::XMVectorSubtract(v1, v2));
		return *this;
	}
	Vector2 operator *=(float scalar)
	{
		DirectX::XMVECTOR v = DirectX::XMLoadFloat2(&vec);
		DirectX::XMStoreFloat2(&vec, DirectX::XMVectorScale(v, scalar));
		return *this;
	}
	Vector2 operator *=(const Vector2& other)
	{
		DirectX::XMVECTOR v1 = DirectX::XMLoadFloat2(&vec);
		DirectX::XMVECTOR v2 = DirectX::XMLoadFloat2(&other.vec);
		DirectX::XMStoreFloat2(&vec, DirectX::XMVectorMultiply(v1, v2));
		return *this;
	}
	Vector2 operator /=(float scalar)
	{
		if (scalar == 0.0f) {
			Debug::LogError("0で除算しようとしました。大きさ0のベクトルを返します");
			vec = {0.0f, 0.0f};
		}
		else {
			DirectX::XMVECTOR v = DirectX::XMLoadFloat2(&vec);
			DirectX::XMStoreFloat2(&vec, DirectX::XMVectorScale(v, 1.0f / scalar));
		}
		return *this;
	}
	Vector2 operator /=(const Vector2& other)
	{
		DirectX::XMVECTOR v1 = DirectX::XMLoadFloat2(&vec);
		DirectX::XMVECTOR v2 = DirectX::XMLoadFloat2(&other.vec);
		DirectX::XMVECTOR zeroCheck = DirectX::XMVectorEqual(v2, DirectX::XMVectorZero());
		if (DirectX::XMVector2NotEqual(zeroCheck, DirectX::XMVectorZero())) {
			Debug::LogError("Vector2のいずれかの要素で0除算しようとしました。大きさ0のベクトルを返します");
			vec = {0.0f, 0.0f};
		}
		else {
			DirectX::XMStoreFloat2(&vec, DirectX::XMVectorDivide(v1, v2));
		}
		return *this;
	}

	float Length() const
	{
		DirectX::XMVECTOR v = DirectX::XMLoadFloat2(&vec);
		return DirectX::XMVectorGetX(DirectX::XMVector2Length(v));
	}
	float LengthSquared() const
	{
		DirectX::XMVECTOR v = DirectX::XMLoadFloat2(&vec);
		return DirectX::XMVectorGetX(DirectX::XMVector2LengthSq(v));
	}
	Vector2 Normalized() const
	{
		DirectX::XMVECTOR v = DirectX::XMLoadFloat2(&vec);
		DirectX::XMFLOAT2 ret;
		DirectX::XMStoreFloat2(&ret, DirectX::XMVector2Normalize(v));
		return Vector2(ret);
	}
	float Dot(const Vector2& other) const
	{
		DirectX::XMVECTOR v1 = DirectX::XMLoadFloat2(&vec);
		DirectX::XMVECTOR v2 = DirectX::XMLoadFloat2(&other.vec);
		return DirectX::XMVectorGetX(DirectX::XMVector2Dot(v1, v2));
	}

	std::string ToString() const
	{
		std::stringstream ss;
		ss << "(" << vec.x << ", " << vec.y << ")";
		return ss.str();
	}

	const DirectX::XMFLOAT2& ToXMFLOAT2() const { return vec; }

public:
	static Vector2 Zero() { return Vector2(0.0f, 0.0f); }
	static Vector2 One() { return Vector2(1.0f, 1.0f); }
};

class Vector3
{
private:
	DirectX::XMFLOAT3 vec;

public:
	Vector3() : vec(0.0f, 0.0f, 0.0f) {}
	Vector3(float x, float y, float z) : vec(x, y, z) {}
	explicit Vector3(const DirectX::XMFLOAT3& v) : vec(v) {}

	float GetX() const { return vec.x; }
	float GetY() const { return vec.y; }
	float GetZ() const { return vec.z; }

	void SetX(float x) { vec.x = x; }
	void SetY(float y) { vec.y = y; }
	void SetZ(float z) { vec.y = z; }

	Vector3 operator +(const Vector3& other) const
	{
		DirectX::XMVECTOR v1 = DirectX::XMLoadFloat3(&vec);
		DirectX::XMVECTOR v2 = DirectX::XMLoadFloat3(&other.vec);
		DirectX::XMFLOAT3 ret;
		DirectX::XMStoreFloat3(&ret, DirectX::XMVectorAdd(v1, v2));
		return Vector3(ret);
	}
	Vector3 operator -(const Vector3& other) const
	{
		DirectX::XMVECTOR v1 = DirectX::XMLoadFloat3(&vec);
		DirectX::XMVECTOR v2 = DirectX::XMLoadFloat3(&other.vec);
		DirectX::XMFLOAT3 ret;
		DirectX::XMStoreFloat3(&ret, DirectX::XMVectorSubtract(v1, v2));
		return Vector3(ret);
	}
	Vector3 operator *(const float scalar) const
	{
		DirectX::XMVECTOR v = DirectX::XMLoadFloat3(&vec);
		DirectX::XMFLOAT3 ret;
		DirectX::XMStoreFloat3(&ret, DirectX::XMVectorScale(v, scalar));
		return Vector3(ret);
	}
	Vector3 operator *(const Vector3& other) const
	{
		DirectX::XMVECTOR v1 = DirectX::XMLoadFloat3(&vec);
		DirectX::XMVECTOR v2 = DirectX::XMLoadFloat3(&other.vec);
		DirectX::XMFLOAT3 ret;
		DirectX::XMStoreFloat3(&ret, DirectX::XMVectorMultiply(v1, v2));
		return Vector3(ret);
	}
	Vector3 operator /(const float scalar) const
	{
		if (scalar == 0.0f) { Debug::LogError("0で除算しようとしました。大きさ0のベクトルを返します"); return Zero(); }
		DirectX::XMVECTOR v = DirectX::XMLoadFloat3(&vec);
		DirectX::XMFLOAT3 ret;
		DirectX::XMStoreFloat3(&ret, DirectX::XMVectorScale(v, 1.0f / scalar));
		return Vector3(ret);
	}
	Vector3 operator /(const Vector3& other) const
	{
		DirectX::XMVECTOR v1 = DirectX::XMLoadFloat3(&vec);
		DirectX::XMVECTOR v2 = DirectX::XMLoadFloat3(&other.vec);
		DirectX::XMVECTOR zeroCheck = DirectX::XMVectorEqual(v2, DirectX::XMVectorZero());
		if (DirectX::XMVector3NotEqual(zeroCheck, DirectX::XMVectorZero())) { Debug::LogError("Vector3のいずれかの要素で0除算しようとしました。大きさ0のベクトルを返します"); return Zero(); }
		DirectX::XMFLOAT3 ret;
		DirectX::XMStoreFloat3(&ret, DirectX::XMVectorDivide(v1, v2));
		return Vector3(ret);
	}
	Vector3 operator +=(const Vector3& other)
	{
		DirectX::XMVECTOR v1 = DirectX::XMLoadFloat3(&vec);
		DirectX::XMVECTOR v2 = DirectX::XMLoadFloat3(&other.vec);
		DirectX::XMStoreFloat3(&vec, DirectX::XMVectorAdd(v1, v2));
		return *this;
	}
	Vector3 operator -=(const Vector3& other)
	{
		DirectX::XMVECTOR v1 = DirectX::XMLoadFloat3(&vec);
		DirectX::XMVECTOR v2 = DirectX::XMLoadFloat3(&other.vec);
		DirectX::XMStoreFloat3(&vec, DirectX::XMVectorSubtract(v1, v2));
		return *this;
	}
	Vector3 operator *=(const float scalar)
	{
		DirectX::XMVECTOR v = DirectX::XMLoadFloat3(&vec);
		DirectX::XMStoreFloat3(&vec, DirectX::XMVectorScale(v, scalar));
		return *this;
	}
	Vector3 operator *=(const Vector3& other)
	{
		DirectX::XMVECTOR v1 = DirectX::XMLoadFloat3(&vec);
		DirectX::XMVECTOR v2 = DirectX::XMLoadFloat3(&other.vec);
		DirectX::XMStoreFloat3(&vec, DirectX::XMVectorMultiply(v1, v2));
		return *this;
	}
	Vector3 operator /=(const float scalar)
	{
		if (scalar == 0.0f) {
			Debug::LogError("0で除算しようとしました。大きさ0のベクトルを返します");
			vec = {0.f, 0.0f, 0.0f};
		}
		else {
			DirectX::XMVECTOR v = DirectX::XMLoadFloat3(&vec);
			DirectX::XMStoreFloat3(&vec, DirectX::XMVectorScale(v, 1.0f / scalar));
		}
		return *this;
	}
	Vector3 operator /=(const Vector3& other)
	{
		DirectX::XMVECTOR v1 = XMLoadFloat3(&vec);
		DirectX::XMVECTOR v2 = XMLoadFloat3(&other.vec);
		DirectX::XMVECTOR zeroCheck = DirectX::XMVectorEqual(v2, DirectX::XMVectorZero());
		if (DirectX::XMVector3NotEqual(zeroCheck, DirectX::XMVectorZero())) {
			Debug::LogError("Vector3のいずれかの要素で0除算しようとしました。大きさ0のベクトルを返します");
			vec = {0.0f, 0.0f, 0.0f };
		}
		else {
			DirectX::XMStoreFloat3(&vec, DirectX::XMVectorDivide(v1, v2));
		}
		return *this;
	}

	float Length() const
	{
		DirectX::XMVECTOR v = DirectX::XMLoadFloat3(&vec);
		return DirectX::XMVectorGetX(DirectX::XMVector3Length(v));
	}
	float LengthSquared() const
	{
		DirectX::XMVECTOR v = DirectX::XMLoadFloat3(&vec);
		return DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(v));
	}
	Vector3 Normalized() const
	{
		DirectX::XMVECTOR v = DirectX::XMLoadFloat3(&vec);
		DirectX::XMFLOAT3 ret;
		DirectX::XMStoreFloat3(&ret, DirectX::XMVector3Normalize(v));
		return Vector3(ret);
	}
	void Normalize()
	{
		DirectX::XMVECTOR v = DirectX::XMLoadFloat3(&vec);
		DirectX::XMStoreFloat3(&vec, DirectX::XMVector3Normalize(v));
	}

	float Dot(const Vector3& other) const
	{
		DirectX::XMVECTOR v1 = DirectX::XMLoadFloat3(&vec);
		DirectX::XMVECTOR v2 = DirectX::XMLoadFloat3(&other.vec);
		return DirectX::XMVectorGetX(DirectX::XMVector3Dot(v1, v2));
	}
	Vector3 Cross(const Vector3& other) const
	{
		DirectX::XMVECTOR v1 = DirectX::XMLoadFloat3(&vec);
		DirectX::XMVECTOR v2 = DirectX::XMLoadFloat3(&other.vec);
		DirectX::XMFLOAT3 ret;
		DirectX::XMStoreFloat3(&ret, DirectX::XMVector3Cross(v1, v2));
		return Vector3(ret);
	}

	std::string ToString() const
	{
		std::stringstream ss;
		ss << "(" << GetX() << ", " << GetY() << ", " << GetZ() << ")";
		return ss.str();
	}

	DirectX::XMVECTOR ToXMVECTOR() const
	{
		return DirectX::XMLoadFloat3(&vec);
	}
	Vector3 Transform(const DirectX::XMMATRIX& matrix) const
	{
		DirectX::XMVECTOR transform = DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&vec), matrix);
		DirectX::XMFLOAT3 ret;
		DirectX::XMStoreFloat3(&ret, transform);
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
		DirectX::XMVECTOR v1 = DirectX::XMLoadFloat3(&a.vec);
		DirectX::XMVECTOR v2 = DirectX::XMLoadFloat3(&b.vec);
		return DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMVectorSubtract(v1, v2)));
	}
};

class Vector4
{
protected:
	DirectX::XMFLOAT4 vec;

public:
	Vector4() : vec(0.0f, 0.0f, 0.0f, 0.0f) {}
	Vector4(float x, float y, float z, float w) : vec(x, y, z, w) {}
	explicit Vector4(const DirectX::XMFLOAT4& v) : vec(v) {}

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
		DirectX::XMVECTOR v1 = DirectX::XMLoadFloat4(&vec);
		DirectX::XMVECTOR v2 = DirectX::XMLoadFloat4(&other.vec);
		DirectX::XMFLOAT4 result;
		DirectX::XMStoreFloat4(&result, DirectX::XMVectorAdd(v1, v2));
		return Vector4(result);
	}
	Vector4 operator -(const Vector4& other) const
	{
		DirectX::XMVECTOR v1 = DirectX::XMLoadFloat4(&vec);
		DirectX::XMVECTOR v2 = DirectX::XMLoadFloat4(&other.vec);
		DirectX::XMFLOAT4 result;
		DirectX::XMStoreFloat4(&result, DirectX::XMVectorSubtract(v1, v2));
		return Vector4(result);
	}
	Vector4 operator *(float scalar) const
	{
		DirectX::XMVECTOR v = DirectX::XMLoadFloat4(&vec);
		DirectX::XMFLOAT4 result;
		DirectX::XMStoreFloat4(&result, DirectX::XMVectorScale(v, scalar));
		return Vector4(result);
	}
	Vector4 operator *(const Vector4& other) const
	{
		DirectX::XMVECTOR v1 = DirectX::XMLoadFloat4(&vec);
		DirectX::XMVECTOR v2 = DirectX::XMLoadFloat4(&other.vec);
		DirectX::XMFLOAT4 result;
		DirectX::XMStoreFloat4(&result, DirectX::XMVectorMultiply(v1, v2));
		return Vector4(result);
	}
	Vector4 operator /(float scalar) const
	{
		if (scalar == 0.0f) {
			Debug::LogError("0で除算しようとしました。大きさ0のベクトルを返します");
			return Zero();
		}
		DirectX::XMVECTOR v = DirectX::XMLoadFloat4(&vec);
		DirectX::XMFLOAT4 result;
		DirectX::XMStoreFloat4(&result, DirectX::XMVectorScale(v, 1.0f / scalar));
		return Vector4(result);
	}
	Vector4 operator /(const Vector4& other) const
	{
		DirectX::XMVECTOR v1 = DirectX::XMLoadFloat4(&vec);
		DirectX::XMVECTOR v2 = DirectX::XMLoadFloat4(&other.vec);
		DirectX::XMVECTOR zeroCheck = DirectX::XMVectorEqual(v2, DirectX::XMVectorZero());
		if (DirectX::XMVector4NotEqual(zeroCheck, DirectX::XMVectorZero())) {
			Debug::LogError("Vector4のいずれかの要素で0除算しようとしました。大きさ0のベクトルを返します");
			return Zero();
		}
		DirectX::XMFLOAT4 result;
		DirectX::XMStoreFloat4(&result, DirectX::XMVectorDivide(v1, v2));
		return Vector4(result);
	}
	Vector4& operator +=(const Vector4& other)
	{
		DirectX::XMVECTOR v1 = DirectX::XMLoadFloat4(&vec);
		DirectX::XMVECTOR v2 = DirectX::XMLoadFloat4(&other.vec);
		DirectX::XMStoreFloat4(&vec, DirectX::XMVectorAdd(v1, v2));
		return *this;
	}
	Vector4& operator -=(const Vector4& other)
	{
		DirectX::XMVECTOR v1 = DirectX::XMLoadFloat4(&vec);
		DirectX::XMVECTOR v2 = DirectX::XMLoadFloat4(&other.vec);
		DirectX::XMStoreFloat4(&vec, DirectX::XMVectorSubtract(v1, v2));
		return *this;
	}
	Vector4 operator *=(float scalar)
	{
		DirectX::XMVECTOR v = DirectX::XMLoadFloat4(&vec);
		DirectX::XMStoreFloat4(&vec, DirectX::XMVectorScale(v, scalar));
		return *this;
	}
	Vector4& operator *=(const Vector4& other)
	{
		DirectX::XMVECTOR v1 = DirectX::XMLoadFloat4(&vec);
		DirectX::XMVECTOR v2 = DirectX::XMLoadFloat4(&other.vec);
		DirectX::XMStoreFloat4(&vec, DirectX::XMVectorMultiply(v1, v2));
		return *this;
	}
	Vector4& operator /(float scalar)
	{
		if (scalar == 0.0f) {
			Debug::LogError("0で除算しようとしました。大きさ0のベクトルを返します");
			vec = { 0.0f, 0.0f, 0.0f, 0.0f };
			return *this;
		}
		DirectX::XMVECTOR v = DirectX::XMLoadFloat4(&vec);
		DirectX::XMStoreFloat4(&vec, DirectX::XMVectorScale(v, 1.0f / scalar));
		return *this;
	}
	Vector4& operator /=(const Vector4& other)
	{
		DirectX::XMVECTOR v1 = DirectX::XMLoadFloat4(&vec);
		DirectX::XMVECTOR v2 = DirectX::XMLoadFloat4(&other.vec);
		DirectX::XMVECTOR zeroCheck = DirectX::XMVectorEqual(v2, DirectX::XMVectorZero());
		if (DirectX::XMVector4NotEqual(zeroCheck, DirectX::XMVectorZero())) {
			Debug::LogError("Vector4のいずれかの要素で0除算しようとしました。大きさ0のベクトルを返します");
			vec = { 0.0f, 0.0f, 0.0f, 0.0f };
		}
		else {
			DirectX::XMStoreFloat4(&vec, DirectX::XMVectorDivide(v1, v2));
		}
		return *this;
	}

	float Length() const
	{
		DirectX::XMVECTOR v = DirectX::XMLoadFloat4(&vec);
		return DirectX::XMVectorGetX(DirectX::XMVector4Length(v));
	}
	float LengthSquared() const {
		DirectX::XMVECTOR v = DirectX::XMLoadFloat4(&vec);
		return DirectX::XMVectorGetX(DirectX::XMVector4LengthSq(v));
	}
	Vector4 Normalized() const
	{
		DirectX::XMVECTOR v = DirectX::XMLoadFloat4(&vec);
		DirectX::XMFLOAT4 result;
		DirectX::XMStoreFloat4(&result, DirectX::XMVector4Normalize(v));
		return Vector4(result);
	}

	std::string ToString() const
	{
		std::stringstream ss;
		ss << "(" << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w << ")";
		return ss.str();
	}

	DirectX::XMVECTOR ToXMVECTOR() const
	{
		return DirectX::XMLoadFloat4(&vec);
	}

public:
public:
	static Vector4 Zero() { return Vector4(0.0f, 0.0f, 0.0f, 0.0f); }

};

class Quaternion : public Vector4
{
private:
	static float ClampToZero(float value) { return (std::abs(value) < 1e-6f) ? 0.0f : value; }
	void Clean()
	{
		vec.x = ClampToZero(vec.x);
		vec.y = ClampToZero(vec.y);
		vec.z = ClampToZero(vec.z);
		vec.w = ClampToZero(vec.w);
	}

public:
	Quaternion() : Vector4(0.0f, 0.0f, 0.0f, 1.0f) {}
	Quaternion(float x, float y, float z, float w) : Vector4(x, y, z, w)
	{
		Normalize();
		Clean();
	}
	explicit Quaternion(DirectX::XMVECTOR& v)
	{
		DirectX::XMStoreFloat4(&vec, DirectX::XMQuaternionNormalize(v));
		Clean();
	}

	static Quaternion Identity() { return Quaternion(0.0f, 0.0f, 0.0f, 1.0f); }

	static Quaternion FromAxisAngle(const Vector3& axis, float angleDeg)
	{
		DirectX::XMVECTOR axisVec = axis.ToXMVECTOR();
		// 時計回り回転にするため、角度を反転
		DirectX::XMVECTOR q = DirectX::XMQuaternionRotationAxis(axisVec, DirectX::XMConvertToRadians(-angleDeg));
		return Quaternion(q);
	}

	static Quaternion FromEulerAngles(float pitch, float yaw, float roll)	// 度数法
	{
		DirectX::XMVECTOR q = DirectX::XMQuaternionRotationRollPitchYaw(
			// 時計回り回転にするため、角度を反転
			DirectX::XMConvertToRadians(-pitch),
			DirectX::XMConvertToRadians(-yaw),
			DirectX::XMConvertToRadians(-roll)
		);
		return Quaternion(q);
	}

	Quaternion operator *(const Quaternion& other) const
	{
		DirectX::XMVECTOR q1 = DirectX::XMQuaternionNormalize(ToXMVECTOR());
		DirectX::XMVECTOR q2 = DirectX::XMQuaternionNormalize(other.ToXMVECTOR());
		DirectX::XMVECTOR mul = DirectX::XMQuaternionMultiply(q1, q2);
		Quaternion result(mul);
		result.Clean();
		return result;
	}

	Quaternion operator *=(const Quaternion& other)
	{
		DirectX::XMVECTOR q1 = DirectX::XMQuaternionNormalize(ToXMVECTOR());
		DirectX::XMVECTOR q2 = DirectX::XMQuaternionNormalize(other.ToXMVECTOR());
		DirectX::XMStoreFloat4(&vec, DirectX::XMQuaternionMultiply(q1, q2));
		Clean();
		return *this;
	}

	Quaternion Conjugate() const
	{
		DirectX::XMVECTOR q = ToXMVECTOR();
		DirectX::XMVECTOR con = DirectX::XMQuaternionConjugate(q);
		Quaternion result(con);
		result.Clean();
		return result;
	}

	Quaternion Normalized() const
	{
		DirectX::XMVECTOR q = DirectX::XMQuaternionNormalize(ToXMVECTOR());
		Quaternion result(q);
		result.Clean();
		return result;
	}

	void Normalize()
	{
		DirectX::XMVECTOR q = ToXMVECTOR();
		DirectX::XMStoreFloat4(&vec, DirectX::XMQuaternionNormalize(q));
		Clean();
	}

	Quaternion Inverse() const
	{
		DirectX::XMVECTOR q = ToXMVECTOR();
		DirectX::XMVECTOR inv = DirectX::XMQuaternionInverse(q);
		Quaternion result(inv);
		result.Clean();
		return result;
	}

	Vector3 Rotate(const Vector3& v) const
	{
		DirectX::XMVECTOR q = DirectX::XMQuaternionNormalize(ToXMVECTOR());
		DirectX::XMVECTOR vec = v.ToXMVECTOR();
		DirectX::XMVECTOR result = DirectX::XMVector3Rotate(vec, q);
		DirectX::XMFLOAT3 out;
		DirectX::XMStoreFloat3(&out, result);
		out.x = ClampToZero(out.x);
		out.y = ClampToZero(out.y);
		out.z = ClampToZero(out.z);
		return Vector3(out.x, out.y, out.z);
	}
};