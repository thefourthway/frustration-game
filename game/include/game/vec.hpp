#pragma once

#include <raylib.h>
#include <cmath>

struct vec2 {
    float x, y;

    vec2() : x(0), y(0) {}
    vec2(float x, float y) : x(x), y(y) {}

    operator ::Vector2() const { return Vector2{ x, y }; }

    vec2 operator+(const vec2& o) const { return { x + o.x, y + o.y }; }
    vec2 operator-(const vec2& o) const { return { x - o.x, y - o.y }; }
    vec2 operator*(float s)        const { return { x * s,   y * s   }; }
    vec2 operator/(float s)        const { return { x / s,   y / s   }; }

    vec2& operator+=(const vec2& o) { x += o.x; y += o.y; return *this; }
    vec2& operator-=(const vec2& o) { x -= o.x; y -= o.y; return *this; }
    vec2& operator*=(float s)       { x *= s;   y *= s;   return *this; }
    vec2& operator/=(float s)       { x /= s;   y /= s;   return *this; }

    float length()    const { return std::sqrt(x*x + y*y); }
    float lengthSq()  const { return x*x + y*y; }
    vec2  normalized()const { float l = length(); return l > 0 ? *this / l : *this; }
    float dot(const vec2& o) const { return x*o.x + y*o.y; }
};

using point = vec2;

struct vec3 {
    float x, y, z;

    vec3() : x(0), y(0), z(0) {}
    vec3(float x, float y, float z) : x(x), y(y), z(z) {}

    operator ::Vector3() const { return Vector3{ x, y, z }; }

    vec3 operator+(const vec3& o) const { return { x + o.x, y + o.y, z + o.z }; }
    vec3 operator-(const vec3& o) const { return { x - o.x, y - o.y, z - o.z }; }
    vec3 operator*(float s)        const { return { x * s,   y * s,   z * s   }; }
    vec3 operator/(float s)        const { return { x / s,   y / s,   z / s   }; }

    vec3& operator+=(const vec3& o) { x += o.x; y += o.y; z += o.z; return *this; }
    vec3& operator-=(const vec3& o) { x -= o.x; y -= o.y; z -= o.z; return *this; }
    vec3& operator*=(float s)       { x *= s;   y *= s;   z *= s;   return *this; }
    vec3& operator/=(float s)       { x /= s;   y /= s;   z /= s;   return *this; }

    float length()    const { return std::sqrt(x*x + y*y + z*z); }
    float lengthSq()  const { return x*x + y*y + z*z; }
    vec3  normalized()const { float l = length(); return l > 0 ? *this / l : *this; }
    float dot(const vec3& o)   const { return x*o.x + y*o.y + z*o.z; }
    vec3  cross(const vec3& o) const {
        return { y*o.z - z*o.y, z*o.x - x*o.z, x*o.y - y*o.x };
    }
};

using point3d = vec3;