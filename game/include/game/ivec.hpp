#pragma once
#include <raylib.h>
#include <functional>

namespace g
{
    struct ivec2 {
        int x, y;
        
        ivec2() : x(0), y(0) {}
        ivec2(int x, int y) : x(x), y(y) {}
        ivec2 operator+(const ivec2& o) const { return { x + o.x, y + o.y }; }
        ivec2 operator-(const ivec2& o) const { return { x - o.x, y - o.y }; }
        ivec2 operator*(int s)          const { return { x * s,   y * s   }; }
        ivec2 operator/(int s)          const { return { x / s,   y / s   }; }
        ivec2& operator+=(const ivec2& o) { x += o.x; y += o.y; return *this; }
        ivec2& operator-=(const ivec2& o) { x -= o.x; y -= o.y; return *this; }
        ivec2& operator*=(int s)          { x *= s;   y *= s;   return *this; }
        ivec2& operator/=(int s)          { x /= s;   y /= s;   return *this; }
        bool operator==(const ivec2& o) const { return x == o.x && y == o.y; }
        bool operator!=(const ivec2& o) const { return !(*this == o); }
        int lengthSq() const { return x*x + y*y; }
        int dot(const ivec2& o) const { return x*o.x + y*o.y; }
        int volume() const { return x * y; }
    };

    struct ivec3 {
        int x, y, z;
        ivec3() : x(0), y(0), z(0) {}
        ivec3(int x, int y, int z) : x(x), y(y), z(z) {}
        ivec3 operator+(const ivec3& o) const { return { x + o.x, y + o.y, z + o.z }; }
        ivec3 operator-(const ivec3& o) const { return { x - o.x, y - o.y, z - o.z }; }
        ivec3 operator*(int s)          const { return { x * s,   y * s,   z * s   }; }
        ivec3 operator/(int s)          const { return { x / s,   y / s,   z / s   }; }
        ivec3& operator+=(const ivec3& o) { x += o.x; y += o.y; z += o.z; return *this; }
        ivec3& operator-=(const ivec3& o) { x -= o.x; y -= o.y; z -= o.z; return *this; }
        ivec3& operator*=(int s)          { x *= s;   y *= s;   z *= s;   return *this; }
        ivec3& operator/=(int s)          { x /= s;   y /= s;   z /= s;   return *this; }
        bool operator==(const ivec3& o) const { return x == o.x && y == o.y && z == o.z; }
        bool operator!=(const ivec3& o) const { return !(*this == o); }
        int lengthSq() const { return x*x + y*y + z*z; }
        int dot(const ivec3& o)   const { return x*o.x + y*o.y + z*o.z; }
        ivec3 cross(const ivec3& o) const {
            return { y*o.z - z*o.y, z*o.x - x*o.z, x*o.y - y*o.x };
        }

        int volume() const { return x * y * z; }
    };

    struct isize2
    {
        int width{0};
        int height{0};

        isize2() = default;
        isize2(int w, int h) :  width(w), height(h) {} 
        
        int volume() const { return width * height; }
    };

    struct irect
    {
        int x{0};
        int y{0};
        int w{0};
        int h{0};

        irect() = default;
        irect(int x, int y, int w, int h) : x(x), y(y), w(w), h(h) {}
        irect(int w, int h) : x(0), y(0), w(w), h(h) {}

        bool contains(const ivec2& pos) const { return pos.x >= x && pos.y >= y && pos.x < w && pos.y < h; };
        
    };
}

namespace g::detail
{
    template<typename T>
    void hash_combine(size_t& seed, const T& val) {
        seed ^= std::hash<T>{}(val) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
}


namespace std {
    template<> struct hash<g::ivec2> {
        size_t operator()(const g::ivec2& v) const {
            size_t seed = 0;
            g::detail::hash_combine(seed, v.x);
            g::detail::hash_combine(seed, v.y);
            return seed;
        }
    };

    template<> struct hash<g::ivec3> {
        size_t operator()(const g::ivec3& v) const {
            size_t seed = 0;
            g::detail::hash_combine(seed, v.x);
            g::detail::hash_combine(seed, v.y);
            g::detail::hash_combine(seed, v.z);
            return seed;
        }
    };

    template<> struct hash<g::isize2> {
        size_t operator()(const g::isize2& v) const {
            size_t seed = 0;
            g::detail::hash_combine(seed, v.width);
            g::detail::hash_combine(seed, v.height);
            return seed;
        }
    };

}