#pragma once
#include "Vector.h"

class Matrix
{
private:
    DirectX::XMFLOAT4X4 mat;

public:
    Matrix() : mat(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1) {}
    explicit Matrix(const DirectX::XMFLOAT4X4& m) : mat(m) {}
    Matrix(float m00, float m01, float m02, float m03,
        float m10, float m11, float m12, float m13,
        float m20, float m21, float m22, float m23,
        float m30, float m31, float m32, float m33)
        : mat(m00, m01, m02, m03, m10, m11, m12, m13,
            m20, m21, m22, m23, m30, m31, m32, m33) {}

    bool Equals(const Matrix& other) const
    {
        for (int row = 0; row < 4; ++row)
        {
            for (int col = 0; col < 4; ++col)
            {
                if (this->GetElement(row, col) != other.GetElement(row, col))
                {
                    return false;
                }
            }
        }
        return true;
    }

    bool operator ==(const Matrix& other) const { return Equals(other); }
    bool operator !=(const Matrix& other) const { return !Equals(other); }

    Matrix operator *(const Matrix& other) const
    {
        DirectX::XMMATRIX m1 = DirectX::XMLoadFloat4x4(&mat);
        DirectX::XMMATRIX m2 = DirectX::XMLoadFloat4x4(&other.mat);
        DirectX::XMFLOAT4X4 result;
        DirectX::XMStoreFloat4x4(&result, DirectX::XMMatrixMultiply(m1, m2));
        return Matrix(result);
    }
    Matrix& operator *=(const Matrix& other)
    {
        DirectX::XMMATRIX m1 = DirectX::XMLoadFloat4x4(&mat);
        DirectX::XMMATRIX m2 = DirectX::XMLoadFloat4x4(&other.mat);
        DirectX::XMStoreFloat4x4(&mat, DirectX::XMMatrixMultiply(m1, m2));
        return *this;
    }

    void SetIdentity()
    {
        mat = DirectX::XMFLOAT4X4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
    }
    void SetFromXMMatrix(const DirectX::XMMATRIX& m)
    {
        XMStoreFloat4x4(&mat, m);
    }

    Matrix Transpose() const
    {
        DirectX::XMMATRIX m = DirectX::XMLoadFloat4x4(&mat);
        DirectX::XMFLOAT4X4 result;
        DirectX::XMStoreFloat4x4(&result, DirectX::XMMatrixTranspose(m));
        return Matrix(result);
    }

    Matrix Inverse() const
    {
        DirectX::XMMATRIX m = DirectX::XMLoadFloat4x4(&mat);
        DirectX::XMVECTOR det;
        DirectX::XMMATRIX inv = DirectX::XMMatrixInverse(&det, m);

        float detVal = DirectX::XMVectorGetX(det);
        if (std::fabsf(detVal) < 1e-6f)
        {
            // 非可逆行列の場合、単位行列を返す
            return Matrix::Identity();
        }

        DirectX::XMFLOAT4X4 result;
        DirectX::XMStoreFloat4x4(&result, inv);
        return Matrix(result);
    }

    bool Decomose(Vector3& outPosition, Quaternion& outRotation, Vector3& outScale) const
    {
        DirectX::XMVECTOR scale, rotation, translation;
        DirectX::XMMATRIX m = DirectX::XMLoadFloat4x4(&mat);
        bool result = DirectX::XMMatrixDecompose(&scale, &rotation, &translation, m);
        if (!result)
        {
            outPosition = Vector3::Zero();
            outRotation = Quaternion::Identity();
            outScale = Vector3::One();
            return false;
        }

        DirectX::XMFLOAT3 scaleF3, translationF3;
        DirectX::XMStoreFloat3(&scaleF3, scale);
        DirectX::XMStoreFloat3(&translationF3, translation);

        outPosition = Vector3(translationF3);
        outRotation = Quaternion(rotation);
        outScale = Vector3(scaleF3);

        return true;
    }

    Vector3 TransformPoint(const Vector3& point) const
    {
        DirectX::XMMATRIX m = DirectX::XMLoadFloat4x4(&mat);
        DirectX::XMVECTOR v = point.ToXMVECTOR();
        DirectX::XMFLOAT3 result;
        DirectX::XMStoreFloat3(&result, DirectX::XMVector3Transform(v, m));
        return Vector3(result.x, result.y, result.z);
    }

    void MakeLookAt(const Vector3& pos, const Vector3& target, const Vector3& up)
    {
        DirectX::XMStoreFloat4x4(
            &mat,
            DirectX::XMMatrixLookAtLH(pos.ToXMVECTOR(), target.ToXMVECTOR(), up.ToXMVECTOR())
        );
    }

    void MakeOrthographicMatrix(float width, float height, float nearZ, float farZ)
    {
        DirectX::XMStoreFloat4x4(
            &mat,
            DirectX::XMMatrixOrthographicLH(width, height, nearZ, farZ)
        );
    }

    void MakeProjectionMatrix(float fov, float aspectRaito, float nearZ, float farZ)
    {
        DirectX::XMStoreFloat4x4(
            &mat,
            DirectX::XMMatrixPerspectiveFovLH(fov, aspectRaito, nearZ, farZ)
        );
    }

    float GetElement(int row, int col) const { return mat.m[row][col]; }
    void SetElement(int row, int col, float value) { mat.m[row][col] = value; }

    std::string ToString() const
    {
        std::stringstream ss;
        for (int i = 0; i < 4; ++i) {
            ss << "(" << mat.m[i][0] << ", " << mat.m[i][1] << ", "
                << mat.m[i][2] << ", " << mat.m[i][3] << ")\n";
        }
        return ss.str();
    }

    DirectX::XMMATRIX ToXMMatrix() const { return DirectX::XMLoadFloat4x4(&mat); }

public:
    static Matrix Identity()
    {
        Matrix m;
        m.SetIdentity();
        return m;
    }

    static Matrix Scaling(float sx, float sy, float sz)
    {
        Matrix m;
        m.SetFromXMMatrix(DirectX::XMMatrixScaling(sx, sy, sz));
        return m;
    }

    static Matrix RotationRollPitchYaw(float pitch, float yaw, float roll) // 度数法
    {
        Matrix m;
        m.SetFromXMMatrix(DirectX::XMMatrixRotationRollPitchYaw(
            DirectX::XMConvertToRadians(pitch),
            DirectX::XMConvertToRadians(yaw),
            DirectX::XMConvertToRadians(roll)
        ));
        return m;
    }

    static Matrix Translation(float tx, float ty, float tz)
    {
        Matrix m;
        m.SetFromXMMatrix(DirectX::XMMatrixTranslation(tx, ty, tz));
        return m;
    }

    static Matrix RotationQuaternion(const Quaternion& q)
    {
        Matrix m;
        m.SetFromXMMatrix(DirectX::XMMatrixRotationQuaternion(q.ToXMVECTOR()));
        return m;
    }

};
