#include "InputSystem.h"
#include <cassert>

#include "Keyboard.h"
#include "GamePad.h"

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

BOOL InputSystem::EnumDevicesCallback(const DIDEVICEINSTANCE* instance, void* context)
{
	auto* self = static_cast<InputSystem*>(context);
	if (instance->dwDevType & DI8DEVTYPE_JOYSTICK)
	{
		try
		{
			auto device = std::make_unique<GamePad>(self->_directInput, self->_hwnd, *instance);
			self->_devices.push_back(std::move(device));
		}
		catch (const std::exception& e)
		{

		}
	}
	return DIENUM_CONTINUE;
}

void InputSystem::EnumerateDevices()
{
	HRESULT ret = _directInput->EnumDevices(
		DI8DEVCLASS_GAMECTRL,
		EnumDevicesCallback,
		this,
		DIEDFL_ATTACHEDONLY);
	if (FAILED(ret))
	{
		assert(0);
		return;
	}
}

void InputSystem::Init(HINSTANCE hInstance, HWND hwnd)
{
	_hwnd = hwnd;

	HRESULT ret = DirectInput8Create(
		hInstance,
		DIRECTINPUT_VERSION,
		IID_IDirectInput8,
		reinterpret_cast<void**>(_directInput.ReleaseAndGetAddressOf()),
		nullptr);
	if (FAILED(ret))
	{
		assert(0);
		return;
	}

	_devices.push_back(std::make_unique<KeyBoard>(_directInput, hwnd));
	EnumerateDevices();
}

void InputSystem::Update()
{
	for (auto& device : _devices)
	{
		device->Update();
	}
}

bool InputSystem::IsButtonDown(uint32_t deviceType, uint32_t button) const
{
	if (deviceType >= _devices.size()) { return false; }
	return _devices[deviceType]->IsButtonDown(button);
}

bool InputSystem::IsButtonUp(uint32_t deviceType, uint32_t button) const
{
	if (deviceType >= _devices.size()) { return false; }
	return _devices[deviceType]->IsButtonUp(button);
}

float InputSystem::GetAxis(uint32_t deviceType, uint32_t axis) const
{
	if (deviceType >= _devices.size()) { return 0.0f; }
	return _devices[deviceType]->GetAxis(axis);
}

int32_t InputSystem::GetPOV(uint32_t deviceType) const
{
	if (deviceType >= _devices.size()) { return -1; }
	return _devices[deviceType]->GetPOV();
}