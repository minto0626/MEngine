#pragma once
#define DIRECTINPUT_VERSION 0x0800	// バージョンを明示的に指定
#include <dinput.h>
#include <wrl.h>
#include <vector>
#include <memory>

#include "IInputDevice.h"
class Keyboard;
class GamePad;

class InputSystem
{
private:
	template<typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;
	
	ComPtr<IDirectInput8> _directInput;
	std::vector<std::unique_ptr<IInputDevice>> _devices;
	HWND _hwnd = nullptr;

	static BOOL CALLBACK EnumDevicesCallback(const DIDEVICEINSTANCE* instance, void* context);
	void EnumerateDevices();

public:
	void Init(HINSTANCE hInstance, HWND hwnd);
	void Update();
	
	bool IsButtonDown(uint32_t deviceType, uint32_t button) const;
	bool IsButtonUp(uint32_t deviceType, uint32_t button) const;
	float GetAxis(uint32_t deviceType, uint32_t axis) const;
	int32_t GetPOV(uint32_t deviceType) const;
};