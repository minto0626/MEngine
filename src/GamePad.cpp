#include "GamePad.h"
#include <cassert>

float GamePad::NormalizeAxis(LONG value, LONG min, LONG max)
{
	if (value == 0)
	{
		return 0.0f;
	}
	float normalized = static_cast<float>(value - min) / (max - min);	// 0.0 ~ 1.0
	return normalized * 2.0f - 1.0f;	// -1.0 ~ 1.0
}

float GamePad::NormalizeTrigger(LONG value, LONG min, LONG max)
{
	if (value == 0)
	{
		return 0.0f;
	}
	return static_cast<float>(value - min) / (max - min);	// 0.0 ~ 1.0
}

GamePad::GamePad(ComPtr<IDirectInput8> directInput, HWND hwnd, const DIDEVICEINSTANCE& deviceInstance)
{
	Init(directInput, hwnd, deviceInstance);
}

GamePad::~GamePad()
{
	if (_device)
	{
		_device->Unacquire();
	}
}

void GamePad::Init(ComPtr<IDirectInput8> directInput, HWND hwnd, const DIDEVICEINSTANCE& deviceInstance)
{
	HRESULT ret = directInput->CreateDevice(
		deviceInstance.guidInstance,
		_device.ReleaseAndGetAddressOf(),
		nullptr);
	if (FAILED(ret))
	{
		assert(0);
		return;
	}

	ret = _device->SetDataFormat(&c_dfDIJoystick2);
	if (FAILED(ret))
	{
		assert(0);
		return;
	}

	ret = _device->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE);
	if (FAILED(ret))
	{
		assert(0);
		return;
	}

	DIPROPRANGE range;
	range.diph.dwSize = sizeof(DIPROPRANGE);
	range.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	range.diph.dwHow = DIPH_DEVICE;
	range.diph.dwObj = 0;
	range.lMin = -32768;
	range.lMax = 32767;
	ret = _device->SetProperty(DIPROP_RANGE, &range.diph);
	if (FAILED(ret))
	{
		assert(0);
		return;
	}

	ret = _device->Acquire();
	if (FAILED(ret))
	{
		assert(0);
		return;
	}
}

void GamePad::Update()
{
	HRESULT ret = _device->Poll();
	if (FAILED(ret))
	{
		ret = _device->Acquire();
		if (FAILED(ret))
		{
			return;
		}

		_device->Poll();
	}

	DIJOYSTATE2 state;
	ret = _device->GetDeviceState(sizeof(DIJOYSTATE2), &state);
	if (SUCCEEDED(ret))
	{
		_state = state;
	}
}

bool GamePad::IsButtonDown(uint32_t button) const
{
	return (_state.rgbButtons[button] & 0x80) != 0;
}

bool GamePad::IsButtonUp(uint32_t button) const
{
	return (_state.rgbButtons[button] & 0x80) == 0;
}

float GamePad::GetAxis(uint32_t axis) const
{
	switch (axis)
	{
	case 0:	// X軸（左スティック横）
		return NormalizeAxis(_state.lX, -32768, 32767);
	case 1: // Y軸（左スティック縦）
		return NormalizeAxis(_state.lY, -32768, 32767);
	case 2: // Z軸（右スティック横）
		return NormalizeAxis(_state.lZ, -32768, 32767);
	case 3: // RZ軸（右スティック縦）
		return NormalizeAxis(_state.lRz, -32768, 32767);
	case 4:	// L2 トリガー
		return NormalizeTrigger(_state.rglSlider[0], 0, 32767);
	case 5:	// R2 トリガー
		return NormalizeTrigger(_state.rglSlider[1], 0, 32767);
	default:
		return 0.0f;
	}
}

int32_t GamePad::GetPOV() const
{
	return _state.rgdwPOV[0];
}