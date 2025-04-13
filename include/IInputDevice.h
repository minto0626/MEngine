#pragma once
#include <cstdint>

class IInputDevice
{
public:
	virtual ~IInputDevice() = default;
	virtual void Update() = 0;
	virtual bool IsButtonDown(uint32_t button) const = 0;
	virtual bool IsButtonUp(uint32_t button) const = 0;
	virtual float GetAxis(uint32_t axis) const = 0;
	virtual int32_t GetPOV() const = 0;	// POV(ハットスイッチ)の角度
};