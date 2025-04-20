#pragma once
#include <DirectXMath.h>
#include <algorithm>

class Color
{
public:
	float r, g, b, a;

	constexpr Color(float r = 0.0f, float g = 0.0f, float b = 0.0f, float a = 1.0f) noexcept
		:r(std::clamp(r, 0.0f, 1.0f)),
		 g(std::clamp(g, 0.0f, 1.0f)),
		 b(std::clamp(b, 0.0f, 1.0f)),
		 a(std::clamp(a, 0.0f, 1.0f))
	{ }

	constexpr Color operator +(const Color& other) const noexcept
	{
		return Color(
			std::clamp(r + other.r, 0.0f, 1.0f),
			std::clamp(g + other.g, 0.0f, 1.0f),
			std::clamp(b + other.b, 0.0f, 1.0f),
			std::clamp(a + other.a, 0.0f, 1.0f)
		);
	}
	constexpr Color operator *(float scalar) const noexcept
	{
		return Color(
			std::clamp(r * scalar, 0.0f, 1.0f),
			std::clamp(g * scalar, 0.0f, 1.0f),
			std::clamp(b * scalar, 0.0f, 1.0f),
			std::clamp(a * scalar, 0.0f, 1.0f)
		);
	}
	constexpr Color operator *(const Color& other) const noexcept
	{
		return Color(
			std::clamp(r * other.r, 0.0f, 1.0f),
			std::clamp(g * other.g, 0.0f, 1.0f),
			std::clamp(b * other.b, 0.0f, 1.0f),
			std::clamp(a * other.a, 0.0f, 1.0f)
		);
	}

	[[nodiscard]] DirectX::XMFLOAT4 ToXMFLOAT4() const noexcept
	{
		return DirectX::XMFLOAT4(r, g, b, a);
	}

public:
	static constexpr Color Lerp(const Color& a, const Color& b, float t) noexcept
	{
		t = std::clamp(t, 0.0f, 1.0f);
		return Color(
			a.r + (b.r - a.r) * t,
			a.g + (b.g - a.g) * t,
			a.b + (b.b - a.b) * t,
			a.a + (b.a - a.a) * t
		);
	}

	static constexpr Color FromHex(uint32_t hex, float alpha = 1.0f) noexcept
	{
		return Color(
			((hex >> 16) & 0xFF) / 255.0f,
			((hex >> 8) & 0xFF) / 255.0f,
			(hex & 0xFF) / 255.0f,
			std::clamp(alpha, 0.0f, 1.0f)
		);
	}

	static constexpr Color Black() noexcept { return Color(0.0f, 0.0f, 0.0f, 1.0f); }
	static constexpr Color White() noexcept { return Color(1.0f, 1.0f, 1.0f, 1.0f); }
	static constexpr Color Red() noexcept { return Color(1.0f, 0.0f, 0.0f, 1.0f); }
	static constexpr Color Green() noexcept { return Color(0.0f, 1.0f, 0.0f, 1.0f); }
	static constexpr Color Blue() noexcept { return Color(0.0f, 0.0f, 1.0f, 1.0f); }
};