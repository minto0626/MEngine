#pragma once
#include "Transform.h"

class Camera
{
private:
	Transform _transform;
	Matrix _viewMatrix;
	uint32_t _viewportWidth;
	uint32_t _viewportHeight;

public:
	void Init(uint32_t viewportWidth, uint32_t viewportHeight);
	void SetPos(Vector3 pos);
	void SetRot(Vector3 rot);

	Matrix GetViewMatrix() const;
};