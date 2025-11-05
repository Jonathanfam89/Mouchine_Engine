
/* Start Header ************************************************************************/
/*!
\file       MathLib.h
\author     Lee Hwee Min [100% - 67 lines]
\par        l.hweemin@digipen.edu
\date       October, 2, 2025
Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#ifndef MATH_LIB_H
#define MATH_LIB_H

#include <cmath>
#include <cstdint>

struct vec3;
struct vec4;
//----------------- vec2 -----------------//
struct vec2
{
	float x, y;
	vec2();
	vec2(float px, float py);
	vec2(float s) : x(s), y(s) {};

	vec2 operator+(vec2 const &v) const noexcept;
	vec2 operator-(vec2 const &v) const noexcept;
	vec2 operator*(float s) const noexcept;
	vec2 operator/(float s) const noexcept;

	vec2 &operator+=(vec2 const &v) noexcept;
	vec2 &operator-=(vec2 const &v) noexcept;
	vec2 &operator*=(float s) noexcept;
	vec2 &operator/=(float s) noexcept;

	float dot(vec2 const &v) const noexcept;
	float cross(vec2 const &v) const noexcept;
	float length() const noexcept;
	vec2 normalized() const noexcept;
	float angle() const noexcept;
};

//----------------- vec3 -----------------//
struct vec3
{
	float x, y, z;
	vec3();
	vec3(float px, float py, float pz);
	vec3(float s) : x(s), y(s), z(s) {};
	vec3(vec4 const& v);


	vec3 operator+(vec3 const &v) const noexcept;
	vec3 operator-(vec3 const &v) const noexcept;
	vec3 operator*(float s) const noexcept;
	vec3 operator/(float s) const noexcept;

	vec3 &operator+=(vec3 const &v) noexcept;
	vec3 &operator-=(vec3 const &v) noexcept;
	vec3 &operator*=(float s) noexcept;
	vec3 &operator/=(float s) noexcept;

	vec3 operator-() const noexcept
	{
		return vec3(-x, -y, -z);
	}

	float dot(vec3 const &v) const noexcept;
	vec3 cross(vec3 const &v) const noexcept;
	float length() const noexcept;
	vec3 normalized() const noexcept;
};

//----------------- vec4 -----------------//
struct vec4
{
	float x, y, z, w;
	vec4();
	vec4(float px, float py, float pz, float pw);
	vec4(float s) : x(s), y(s), z(s), w(s) {};
	vec4(vec3 const& v, float pw);

	vec4 operator+(vec4 const &v) const noexcept;
	vec4 operator-(vec4 const &v) const noexcept;
	vec4 operator*(float s) const noexcept;
	vec4 operator/(float s) const noexcept;
};

//----------------- mat3 -----------------//
struct mat3
{
	float m[9];

	mat3();
	static mat3 translate(vec2 const &v);
	static mat3 rotate(float radians);
	static mat3 scale(vec2 const &v);

	mat3 operator*(mat3 const &o) const;
};

//----------------- mat4 -----------------//
struct mat4
{
	float m[16];

	mat4();
	static mat4 translate(vec3 const &v);
	static mat4 scale(vec3 const &v);
	static mat4 rotateX(float radians);
	static mat4 rotateY(float radians);
	static mat4 rotateZ(float radians);

	mat4 operator*(mat4 const &o) const;


	static mat4 ortho(float left, float right, float bottom, float top, float near = -1.0f, float far = 1.0f)
	{
		mat4 result;
		for (int i = 0; i < 16; i++)
			result.m[i] = 0.0f; // zero initialize

		result.m[0] = 2.0f / (right - left);
		result.m[5] = 2.0f / (top - bottom);
		result.m[10] = -2.0f / (far - near);
		result.m[12] = -(right + left) / (right - left);
		result.m[13] = -(top + bottom) / (top - bottom);
		result.m[14] = -(far + near) / (far - near);
		result.m[15] = 1.0f;

		return result;
	}

	static mat4 rotate(float radians, vec3 axis)
	{
		axis = axis.normalized();
		float c = cos(radians);
		float s = sin(radians);
		float one_c = 1.0f - c;

		mat4 result;
		result.m[0] = c + axis.x * axis.x * one_c;
		result.m[1] = axis.x * axis.y * one_c - axis.z * s;
		result.m[2] = axis.x * axis.z * one_c + axis.y * s;
		result.m[3] = 0.0f;

		result.m[4] = axis.y * axis.x * one_c + axis.z * s;
		result.m[5] = c + axis.y * axis.y * one_c;
		result.m[6] = axis.y * axis.z * one_c - axis.x * s;
		result.m[7] = 0.0f;

		result.m[8] = axis.z * axis.x * one_c - axis.y * s;
		result.m[9] = axis.z * axis.y * one_c + axis.x * s;
		result.m[10] = c + axis.z * axis.z * one_c;
		result.m[11] = 0.0f;

		result.m[12] = result.m[13] = result.m[14] = 0.0f;
		result.m[15] = 1.0f;

		return result;
	}
};

vec4 operator*(mat4 const& m, vec4 const& v);

inline vec3::vec3(vec4 const& v) : x(v.x), y(v.y), z(v.z) {}
inline vec4::vec4(vec3 const& v, float pw) : x(v.x), y(v.y), z(v.z), w(pw) {}

bool SquareToMouseCheck(float, float, float, float);
bool CircleToCircelCheck(float, float, float, float);

#endif // MATH_LIB