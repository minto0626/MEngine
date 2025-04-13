#include "Keyboard.h"
#include <cassert>

KeyBoard::KeyBoard(ComPtr<IDirectInput8> directInput, HWND hwnd)
{
	Init(directInput, hwnd);
}

KeyBoard::~KeyBoard()
{
	if (_device)
	{
		_device->Unacquire();
	}
}

void KeyBoard::Init(ComPtr<IDirectInput8> directInput, HWND hwnd)
{
	HRESULT ret = directInput->CreateDevice(
		GUID_SysKeyboard,
		_device.ReleaseAndGetAddressOf(),
		nullptr);
	if (FAILED(ret))
	{
		assert(0);
		return;
	}

	ret = _device->SetDataFormat(&c_dfDIKeyboard);
	if (FAILED(ret))
	{
		assert(0);
		return;
	}

	ret = _device->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	if (FAILED(ret))
	{
		assert(0);
		return;
	}

	ret = _device->Acquire();
	//if (FAILED(ret))
	//{
	//	assert(0);
	//	return;
	//}
}

void KeyBoard::Update()
{
	HRESULT ret = _device->GetDeviceState(sizeof(_state), _state.data());
	if (FAILED(ret))
	{
		ret = _device->Acquire();
		if (SUCCEEDED(ret))
		{
			_device->GetDeviceState(sizeof(_state), _state.data());
		}
	}
}

bool KeyBoard::IsButtonDown(uint32_t button) const
{
	return (_state[button] & 0x80) != 0;
}

bool KeyBoard::IsButtonUp(uint32_t button) const
{
	return (_state[button] & 0x80) == 0;
}

float KeyBoard::GetAxis(uint32_t axis) const
{
	return 0.0f;
}

int32_t KeyBoard::GetPOV() const
{
	return -1;
}