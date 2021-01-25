#pragma once

#include <cmath>

class vec2 final
{
public:
	constexpr vec2() = default;

	constexpr vec2(float x, float y) : x(x), y(y) {}

	constexpr explicit vec2(float k) : x {k}, y {k} {}

	constexpr auto operator<=>(const vec2&) const = default;

	constexpr vec2 operator+(const vec2& v) const { return {x + v.x, y + v.y}; }

	constexpr vec2 operator-(const vec2& v) const { return {x - v.x, y - v.y}; }

	// Dot-Product
	constexpr float operator*(const vec2& v) const { return (x * v.x) + (y * v.y); }

	constexpr vec2 operator*(float k) const { return {k * x, k * y}; }
	constexpr vec2 operator/(float k) const { return {x / k, y / k}; }

	constexpr void operator+=(const vec2& v)
	{
		x += v.x;
		y += v.y;
	}

	constexpr void operator-=(const vec2& v)
	{
		x -= v.x;
		y -= v.y;
	}

	constexpr void operator*=(float k)
	{
		x *= k;
		y *= k;
	}

	constexpr void operator/=(float k)
	{
		x /= k;
		y /= k;
	}

	constexpr vec2 operator-() const { return {-x, -y}; }

	constexpr float magnitute2() const { return (x * x) + (y * y); }

	// constexpr float magnitute() const { return __builtin_sqrtf(magnitute2()); }
	constexpr float magnitute() const { return std::sqrt(magnitute2()); }

	constexpr void normalize()
	{
		const float mag = magnitute();
		if(mag == 0.0f)
			return;
		(*this) /= mag;
	}

	friend constexpr vec2 operator*(float k, const vec2& v) { return v * k; }

private:
	float x {0.f};
	float y {0.f};
};
