/* Start Header ************************************************************************/
/*!
\file       MathLib.cpp
\author     Lee Hwee Min [100% - 124 lines]
\par        l.hweemin@digipen.edu
\date       October, 2, 2025
Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "MathLib.h"

//----------------- vec2 -----------------//
vec2::vec2() : x(0), y(0) {}
vec2::vec2(float px, float py) : x(px), y(py) {}

vec2 vec2::operator+(vec2 const &v) const noexcept { return vec2(x + v.x, y + v.y); }
vec2 vec2::operator-(vec2 const &v) const noexcept { return vec2(x - v.x, y - v.y); }
vec2 vec2::operator*(float s) const noexcept { return vec2(x * s, y * s); }
vec2 vec2::operator/(float s) const noexcept { return vec2(x / s, y / s); }

vec2 &vec2::operator+=(vec2 const &v) noexcept
{
    x += v.x;
    y += v.y;
    return *this;
}
vec2 &vec2::operator-=(vec2 const &v) noexcept
{
    x -= v.x;
    y -= v.y;
    return *this;
}
vec2 &vec2::operator*=(float s) noexcept
{
    x *= s;
    y *= s;
    return *this;
}
vec2 &vec2::operator/=(float s) noexcept
{
    x /= s;
    y /= s;
    return *this;
}

float vec2::dot(vec2 const &v) const noexcept { return x * v.x + y * v.y; }
float vec2::cross(vec2 const &v) const noexcept { return x * v.y - y * v.x; }
float vec2::length() const noexcept { return std::sqrt(x * x + y * y); }
vec2 vec2::normalized() const noexcept
{
    float len = length();
    return (len > 0) ? (*this) / len : vec2(0, 0);
}
float vec2::angle() const noexcept { return std::atan2(y, x); }

//----------------- vec3 -----------------//
vec3::vec3() : x(0), y(0), z(0) {}
vec3::vec3(float px, float py, float pz) : x(px), y(py), z(pz) {}

vec3 vec3::operator+(vec3 const &v) const noexcept { return vec3(x + v.x, y + v.y, z + v.z); }
vec3 vec3::operator-(vec3 const &v) const noexcept { return vec3(x - v.x, y - v.y, z - v.z); }
vec3 vec3::operator*(float s) const noexcept { return vec3(x * s, y * s, z * s); }
vec3 vec3::operator/(float s) const noexcept { return vec3(x / s, y / s, z / s); }

vec3 &vec3::operator+=(vec3 const &v) noexcept
{
    x += v.x;
    y += v.y;
    z += v.z;
    return *this;
}
vec3 &vec3::operator-=(vec3 const &v) noexcept
{
    x -= v.x;
    y -= v.y;
    z -= v.z;
    return *this;
}
vec3 &vec3::operator*=(float s) noexcept
{
    x *= s;
    y *= s;
    z *= s;
    return *this;
}
vec3 &vec3::operator/=(float s) noexcept
{
    x /= s;
    y /= s;
    z /= s;
    return *this;
}

float vec3::dot(vec3 const &v) const noexcept { return x * v.x + y * v.y + z * v.z; }
vec3 vec3::cross(vec3 const &v) const noexcept
{
    return vec3(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
}
float vec3::length() const noexcept { return std::sqrt(x * x + y * y + z * z); }
vec3 vec3::normalized() const noexcept
{
    float len = length();
    return (len > 0) ? (*this) / len : vec3(0, 0, 0);
}

//----------------- vec4 -----------------//
vec4::vec4() : x(0), y(0), z(0), w(0) {}
vec4::vec4(float px, float py, float pz, float pw) : x(px), y(py), z(pz), w(pw) {}

vec4 vec4::operator+(vec4 const &v) const noexcept { return vec4(x + v.x, y + v.y, z + v.z, w + v.w); }
vec4 vec4::operator-(vec4 const &v) const noexcept { return vec4(x - v.x, y - v.y, z - v.z, w - v.w); }
vec4 vec4::operator*(float s) const noexcept { return vec4(x * s, y * s, z * s, w * s); }
vec4 vec4::operator/(float s) const noexcept { return vec4(x / s, y / s, z / s, w / s); }


//----------------- mat3 -----------------//
mat3::mat3()
{
    for (int i = 0; i < 9; i++)
    {
        m[i] = (i % 4 == 0) ? 1.f : 0.f;
    }
}

mat3 mat3::translate(vec2 const &v)
{
    mat3 r;
    r.m[6] = v.x;
    r.m[7] = v.y;
    return r;
}

mat3 mat3::rotate(float radians)
{
    mat3 r;
    float c = std::cos(radians);
    float s = std::sin(radians);

    r.m[0] = c;
    r.m[3] = s;
    r.m[1] = s;
    r.m[4] = c;
    return r;
}

mat3 mat3::scale(vec2 const &v)
{
    mat3 r;
    r.m[0] = v.x;
    r.m[4] = v.y;
    return r;
}

mat3 mat3::operator*(mat3 const &o) const
{
    mat3 r;
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            r.m[i * 3 + j] = m[0 * 3 + j] * o.m[i * 3 + 0] + m[1 * 3 + j] * o.m[i * 3 + 1] + m[2 * 3 + j] * o.m[i * 3 + 2];
        }
    }
    return r;
}

//----------------- mat4 -----------------//
mat4::mat4()
{
    for (int i = 0; i < 16; ++i)
    {
        m[i] = (i % 5 == 0) ? 1.0f : 0.0f;
    }
}

mat4 mat4::translate(vec3 const &v)
{
    mat4 r;
    r.m[12] = v.x;
    r.m[13] = v.y;
    r.m[14] = v.z;
    return r;
}

mat4 mat4::scale(vec3 const &v)
{
    mat4 r;
    r.m[0] = v.x;
    r.m[5] = v.y;
    r.m[10] = v.z;
    return r;
}

mat4 mat4::rotateX(float rad)
{
    mat4 r;
    float c = std::cos(rad), s = std::sin(rad);
    r.m[5] = c;
    r.m[9] = -s;
    r.m[6] = s;
    r.m[10] = c;
    return r;
}

mat4 mat4::rotateY(float rad)
{
    mat4 r;
    float c = std::cos(rad), s = std::sin(rad);
    r.m[0] = c;
    r.m[8] = s;
    r.m[2] = -s;
    r.m[10] = c;
    return r;
}

mat4 mat4::rotateZ(float rad)
{
    mat4 r;
    float c = std::cos(rad), s = std::sin(rad);
    r.m[0] = c;
    r.m[4] = -s;
    r.m[1] = s;
    r.m[5] = c;
    return r;
}

mat4 mat4::operator*(mat4 const &o) const
{
    mat4 r;
    for (int col = 0; col < 4; ++col)
    {
        for (int row = 0; row < 4; ++row)
        {
            r.m[col * 4 + row] = 0.0f;
            for (int k = 0; k < 4; ++k)
            {
                r.m[col * 4 + row] += m[k * 4 + row] * o.m[col * 4 + k];
            }
        }
    }
    return r;
}

vec4 operator*(mat4 const& m, vec4 const& v)
{
    vec4 r;
    r.x = m.m[0] * v.x + m.m[4] * v.y + m.m[8] * v.z + m.m[12] * v.w;
    r.y = m.m[1] * v.x + m.m[5] * v.y + m.m[9] * v.z + m.m[13] * v.w;
    r.z = m.m[2] * v.x + m.m[6] * v.y + m.m[10] * v.z + m.m[14] * v.w;
    r.w = m.m[3] * v.x + m.m[7] * v.y + m.m[11] * v.z + m.m[15] * v.w;
    return r;
}


bool IsAreaClicked(float buttonX, float buttonY, float buttonWidth, float buttonHeight, float mouseX, float mouseY)
{
   
    float halfWidth = std::abs(buttonWidth) / 2.0f;
    float halfHeight = std::abs(buttonHeight) / 2.0f;

    float left = buttonX - halfWidth;
    float right = buttonX + halfWidth;
    float top = buttonY + halfHeight;
    float bottom = buttonY - halfHeight;

    return (mouseX >= left && mouseX <= right &&
        mouseY >= bottom && mouseY <= top);
}

void DecomposeMatrix(mat4 const& m, float& x, float& y, float& w, float& h, float& rot)
{
    // Extract translation
    x = m.m[12];
    y = m.m[13];

    // Extract scale
    w = sqrt(m.m[0] * m.m[0] + m.m[1] * m.m[1]);
    h = sqrt(m.m[4] * m.m[4] + m.m[5] * m.m[5]);

    // Create a matrix with rotation and scale
    mat3 rotScaleMat;
    rotScaleMat.m[0] = m.m[0];
    rotScaleMat.m[1] = m.m[1];
    rotScaleMat.m[3] = m.m[4];
    rotScaleMat.m[4] = m.m[5];

    // Remove scale from the matrix to get pure rotation
    if (w > 0) {
        rotScaleMat.m[0] /= w;
        rotScaleMat.m[1] /= w;
    }
    if (h > 0) {
        rotScaleMat.m[3] /= h;
        rotScaleMat.m[4] /= h;
    }

    // Extract rotation from the pure rotation matrix
    rot = atan2(rotScaleMat.m[1], rotScaleMat.m[0]);

    // Check for reflection (negative scale)
    if (m.m[0] * m.m[5] - m.m[4] * m.m[1] < 0) {
        // This indicates a flip. We'll represent this as a 180-degree rotation
        // and a positive scale, as C# might not handle negative scales well.
        w = -w;
        h = -h;
        rot += 3.14159265359f; // Add 180 degrees (pi radians)
    }
}