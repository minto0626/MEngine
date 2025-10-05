#pragma once
#include "Transform.h"
#include "Component.h"

#include <memory>

class Camera : public Component
{
public:
	enum ProjectionType
	{
		Ortho,
		Perspective,
	};

private:
	class Transform* _transform;
	Vector3 _target;
	float _fov;
	float _aspectRaito;
	float _nearZ;
	float _farZ;
	Matrix _viewMatrix;
	Matrix _projectionMatrix;
	Matrix _viewProjectionMatrix;
	uint32_t _viewportWidth;
	uint32_t _viewportHeight;
	ProjectionType _projectionType;
	bool _isDirty;

public:
	Camera(class GameObject* owner, int updateOrder = 100);

	void Init(ProjectionType projectionType, uint32_t viewportWidth, uint32_t viewportHeight);
	void SetPos(Vector3 pos);
	void SetTarget(Vector3 target);
	void SetRot(Quaternion rot);
	Matrix GetViewMatrix();
	Matrix GetProjectionMatrix();
	Matrix GetViewProjectionMatrix();

	void Update(float deltaTime) override;
};