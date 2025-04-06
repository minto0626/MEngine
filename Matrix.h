#pragma once
#include "Vector.h"

class Matrix
{
private:
    XMFLOAT4X4 mat;

public:
    Matrix() : mat(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1) {}
    explicit Matrix(const XMFLOAT4X4& m) : mat(m) {}
    Matrix(float m00, float m01, float m02, float m03,
        float m10, float m11, float m12, float m13,
        float m20, float m21, float m22, float m23,
        float m30, float m31, float m32, float m33)
        : mat(m00, m01, m02, m03, m10, m11, m12, m13,
            m20, m21, m22, m23, m30, m31, m32, m33) {}

    Matrix operator *(const Matrix& other) const
    {
        XMMATRIX m1 = XMLoadFloat4x4(&mat);
        XMMATRIX m2 = XMLoadFloat4x4(&other.mat);
        XMFLOAT4X4 result;
        XMStoreFloat4x4(&result, XMMatrixMultiply(m1, m2));
        return Matrix(result);
    }
    Matrix& operator *=(const Matrix& other)
    {
        XMMATRIX m1 = XMLoadFloat4x4(&mat);
        XMMATRIX m2 = XMLoadFloat4x4(&other.mat);
        XMStoreFloat4x4(&mat, XMMatrixMultiply(m1, m2));
        return *this;
    }

    void SetIdentity()
    {
        mat = XMFLOAT4X4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
    }
    void SetFromXMMatrix(const XMMATRIX& m)
    {
        XMStoreFloat4x4(&mat, m);
    }

    Matrix Transpose() const
    {
        XMMATRIX m = XMLoadFloat4x4(&mat);
        XMFLOAT4X4 result;
        XMStoreFloat4x4(&result, XMMatrixTranspose(m));
        return Matrix(result);
    }

    Vector3 TransformPoint(const Vector3& point) const
    {
        XMMATRIX m = XMLoadFloat4x4(&mat);
        XMVECTOR v = point.ToXMVECTOR();
        XMFLOAT3 result;
        XMStoreFloat3(&result, XMVector3Transform(v, m));
        return Vector3(result.x, result.y, result.z);
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

    XMMATRIX ToXMMatrix() const { return XMLoadFloat4x4(&mat); }

public:
    static Matrix Scaling(float sx, float sy, float sz)
    {
        Matrix m;
        m.SetFromXMMatrix(XMMatrixScaling(sx, sy, sz));
        return m;
    }

    static Matrix RotationRollPitchYaw(float pitch, float yaw, float roll) // “x”–@
    {
        Matrix m;
        m.SetFromXMMatrix(XMMatrixRotationRollPitchYaw(
            XMConvertToRadians(pitch),
            XMConvertToRadians(yaw),
            XMConvertToRadians(roll)
        ));
        return m;
    }

    static Matrix Translation(float tx, float ty, float tz)
    {
        Matrix m;
        m.SetFromXMMatrix(XMMatrixTranslation(tx, ty, tz));
        return m;
    }

};