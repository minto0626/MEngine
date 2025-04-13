#pragma once
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <wrl.h>
#include "IInputDevice.h"

class GamePad : public IInputDevice
{
private:
	template<typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	ComPtr<IDirectInputDevice8> _device;
	DIJOYSTATE2 _state = {};

	static float NormalizeAxis(LONG value, LONG min, LONG max);
	static float NormalizeTrigger(LONG value, LONG min, LONG max);
	void Init(ComPtr<IDirectInput8> directInput, HWND hwnd, const DIDEVICEINSTANCE& deviceInstance);

public:
	GamePad(ComPtr<IDirectInput8> directInput, HWND hwnd, const DIDEVICEINSTANCE& deviceInstance);
	~GamePad();

	void Update() override;
	bool IsButtonDown(uint32_t button) const override;
	bool IsButtonUp(uint32_t button) const override;
	float GetAxis(uint32_t axis) const override;
	int32_t GetPOV() const override;
};