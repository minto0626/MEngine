#pragma once
#include "Math/Vector.h"
#include "Math/Matrix.h"
#include "Component.h"

#include <string>
#include <vector>

class alignas(16) Transform : public Component
{
private:
	Vector3 _pos;
	Quaternion _rot;
	Vector3 _scale;
	Matrix _world;
	bool _isDirty;

    Transform* _parent = nullptr;
    std::vector<Transform*> _children;

    void MarkDirty();
    void MarkChildrenDirty();
	void UpdateWorldMatrix();

public:
    enum class TransformSpace
    {
        // 親の変換を維持する
        KeepWorld,
        // ローカル変換を維持する
        KeepLocal,
    };

public:
	Transform(class GameObject* owner, int updateOrder = 100);
    void OnDestroy() override;

    void SetPos(const Vector3& pos) { _pos = pos; MarkDirty(); }
	void SetRot(const Quaternion& rot) { _rot = rot; MarkDirty(); }
	void SetScale(const Vector3& scale) { _scale = scale; MarkDirty(); }

	Vector3 GetPos() const { return _pos; }
	Quaternion GetRot() const { return _rot; }
	Vector3 GetScale() const { return _scale; }
	Matrix GetWorldMatrix() { UpdateWorldMatrix(); return _world; }

	Vector3 GetForward() const;
	Vector3 GetRight() const;
	Vector3 GetUp() const;

    void SetParent(Transform* parent, TransformSpace space = TransformSpace::KeepWorld);

	Vector3 TransformPoint(const Vector3& point);
	Vector3 TrasnformDirection(const Vector3& dir);
	std::string ToString() const;

};
