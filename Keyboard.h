#pragma once
#define DIRECTINPUT_VERSION 0x0800	// バージョンを明示的に指定
#include <dinput.h>
#include <wrl.h>
#include <array>
#include "IInputDevice.h"

class KeyBoard : public IInputDevice
{
private:
	template<typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	ComPtr<IDirectInputDevice8> _device;
	std::array<uint8_t, 256> _state = {};

	void Init(ComPtr<IDirectInput8> directInput, HWND hwnd);

public:
	KeyBoard(ComPtr<IDirectInput8> directInput, HWND hwnd);
	~KeyBoard();

	void Update();

	bool IsButtonDown(uint32_t button) const override;
	bool IsButtonUp(uint32_t button) const override;
	float GetAxis(uint32_t axis) const override;
	int32_t GetPOV() const override;
};