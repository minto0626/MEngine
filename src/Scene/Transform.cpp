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

void Transform::OnDestroy()
{
    // 親から外す
    SetParent(nullptr);
    // 子供の親を外す
    for (auto* child : _children)
    {
        if (child->_owner != nullptr)
        {
            child->_owner->SetState(GameObject::State::Dead);
        }
    }
    _children.clear();
}

void Transform::MarkDirty()
{
    _isDirty = true;
    MarkChildrenDirty();
}

void Transform::MarkChildrenDirty()
{
    for (auto* child : _children)
    {
        child->_isDirty = true;
        child->MarkChildrenDirty();
    }
}

void Transform::UpdateWorldMatrix()
{
    // 変更がなければ何もしない
    if (!_isDirty)
    {
        return;
    }

    Matrix localMatrix =
        Matrix::Scaling(_scale.GetX(), _scale.GetY(), _scale.GetZ()) *
        Matrix::RotationQuaternion(_rot) *
        Matrix::Translation(_pos.GetX(), _pos.GetY(), _pos.GetZ());

    // 親がいる場合は親のワールド行列を掛ける
    _world = _parent != nullptr ? localMatrix * _parent->GetWorldMatrix() : localMatrix;

    _isDirty = false;

    // 子供の行列も更新
    for (auto* child : _children)
    {
        child->UpdateWorldMatrix();
    }
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

void Transform::SetParent(Transform* parent, TransformSpace space)
{
    if (parent == this)
    {
        // 自分自身を親に設定しようとした場合は無視
        return;
    }
    if (_parent == parent)
    {
        // すでに同じ親が設定されている場合は無視
        return;
    }

    // すでに親がいる場合は、現在の親から外す
    if (_parent != nullptr)
    {
        auto& siblings = _parent->_children;
        if (siblings.size() > 0)
        {
            siblings.erase(std::remove(siblings.begin(), siblings.end(), this), siblings.end());
        }
    }

    // 新しい親を設定
    _parent = parent;

    // 新しい親の子リストに追加
    if (_parent != nullptr)
    {
        _parent->_children.push_back(this);
    }

    if (space == TransformSpace::KeepWorld)
    {
        Matrix parentWorld = parent != nullptr ? parent->GetWorldMatrix() : Matrix::Identity();
        Matrix worldMatrix = GetWorldMatrix();
        Matrix localWorld = worldMatrix * parentWorld.Inverse();

        // ローカル変換を抽出
        auto ret = localWorld.Decomose(_pos, _rot, _scale);
    }

    MarkDirty();
    UpdateWorldMatrix();
}

Vector3 Transform::TransformPoint(const Vector3& point)
{
	UpdateWorldMatrix();
	return _world.TransformPoint(point);
}

Vector3 Transform::TrasnformDirection(const Vector3& dir)
{
	UpdateWorldMatrix();
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
