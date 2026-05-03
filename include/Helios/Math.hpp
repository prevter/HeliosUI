#pragma once
#ifndef HELIOS_MATH_HPP
#define HELIOS_MATH_HPP

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <fmt/format.h>

namespace Helios {
    struct Vec2 {
        float x = 0.f, y = 0.f;

        constexpr Vec2() noexcept = default;
        constexpr Vec2(float x, float y) noexcept : x(x), y(y) {}

        constexpr Vec2 operator+(Vec2 o) const noexcept { return Vec2(x + o.x, y + o.y); }
        constexpr Vec2 operator-(Vec2 o) const noexcept { return Vec2(x - o.x, y - o.y); }
        constexpr Vec2 operator*(Vec2 o) const noexcept { return Vec2(x * o.x, y * o.y); }
        constexpr Vec2 operator*(float s) const noexcept { return Vec2(x * s, y * s); }
        constexpr Vec2 operator/(float s) const noexcept { return Vec2(x / s, y / s); }
        constexpr Vec2& operator+=(Vec2 o) noexcept { x += o.x; y += o.y; return *this; }
        constexpr Vec2& operator-=(Vec2 o) noexcept { x -= o.x; y -= o.y; return *this; }
        constexpr Vec2& operator*=(Vec2 o) noexcept { x *= o.x; y *= o.y; return *this; }
        constexpr Vec2& operator*=(float s) noexcept { x *= s; y *= s; return *this; }
        constexpr Vec2& operator/=(float s) noexcept { x /= s; y /= s; return *this; }

        constexpr bool operator==(Vec2 o) const noexcept { return x == o.x && y == o.y; }

        constexpr float length() const noexcept { return std::sqrt(x * x + y * y); }
        constexpr float lengthSquared() const noexcept { return x * x + y * y; }

        constexpr Vec2 normalized() const noexcept {
            float len = length();
            return len > std::numeric_limits<float>::epsilon() ? *this / len : Vec2(0.f, 0.f);
        }

        constexpr float dot(Vec2 o) const noexcept { return x * o.x + y * o.y; }
        constexpr float cross(Vec2 o) const noexcept { return x * o.y - y * o.x; }

        constexpr Vec2 perpendicular() const noexcept { return Vec2(-y, x); }

        constexpr Vec2 lerp(Vec2 target, float t) const noexcept {
            return Vec2(x + (target.x - x) * t, y + (target.y - y) * t);
        }
    };

    inline Vec2 operator*(float s, Vec2 v) noexcept { return v * s; }

    struct Vec4 {
        float r = 0.f, g = 0.f, b = 0.f, a = 0.f;

        constexpr Vec4() noexcept = default;
        constexpr Vec4(float r, float g, float b, float a) noexcept : r(r), g(g), b(b), a(a) {}
    };

    struct Color {
        uint8_t r = 255, g = 255, b = 255, a = 255;

        constexpr Color() noexcept = default;
        constexpr Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) noexcept : r(r), g(g), b(b), a(a) {}

        static constexpr Color fromFloats(float r, float g, float b, float a = 1.f) noexcept {
            return Color(
                static_cast<uint8_t>(std::clamp(r * 255.f, 0.f, 255.f)),
                static_cast<uint8_t>(std::clamp(g * 255.f, 0.f, 255.f)),
                static_cast<uint8_t>(std::clamp(b * 255.f, 0.f, 255.f)),
                static_cast<uint8_t>(std::clamp(a * 255.f, 0.f, 255.f))
            );
        }

        static constexpr Color fromVec4(Vec4 v) noexcept {
            return fromFloats(v.r, v.g, v.b, v.a);
        }

        static constexpr Color white() noexcept { return Color(255, 255, 255); }
        static constexpr Color black() noexcept { return Color(0, 0, 0); }
        static constexpr Color red() noexcept { return Color(255, 0, 0); }
        static constexpr Color green() noexcept { return Color(0, 255, 0); }
        static constexpr Color blue() noexcept { return Color(0, 0, 255); }
        static constexpr Color transparent() noexcept { return Color(0, 0, 0, 0); }

        constexpr Vec4 toVec4() const noexcept {
            return Vec4(r / 255.f, g / 255.f, b / 255.f, a / 255.f);
        }

        constexpr Color withAlpha(uint8_t alpha) const noexcept {
            return Color(r, g, b, alpha);
        }

        constexpr uint32_t toRGBA8() const noexcept {
            return static_cast<uint32_t>(r) << 24 |
                   static_cast<uint32_t>(g) << 16 |
                   static_cast<uint32_t>(b) << 8 |
                   static_cast<uint32_t>(a);
        }

        constexpr bool operator==(Color o) const noexcept {
            return r == o.r && g == o.g && b == o.b && a == o.a;
        }
    };

    struct Rect {
        float x = 0.f, y = 0.f, w = 0.f, h = 0.f;

        constexpr Rect() noexcept = default;
        constexpr Rect(float x, float y, float w, float h) noexcept : x(x), y(y), w(w), h(h) {}
        constexpr Rect(Vec2 pos, Vec2 size) noexcept : x(pos.x), y(pos.y), w(size.x), h(size.y) {}

        static constexpr Rect fromMinMax(Vec2 min, Vec2 max) noexcept {
            return Rect(min.x, min.y, max.x - min.x, max.y - min.y);
        }

        constexpr float left() const noexcept { return x; }
        constexpr float right() const noexcept { return x + w; }
        constexpr float top() const noexcept { return y; }
        constexpr float bottom() const noexcept { return y + h; }

        constexpr Vec2 topLeft() const noexcept { return Vec2(x, y); }
        constexpr Vec2 topRight() const noexcept { return Vec2(x + w, y); }
        constexpr Vec2 bottomLeft() const noexcept { return Vec2(x, y + h); }
        constexpr Vec2 bottomRight() const noexcept { return Vec2(x + w, y + h); }
        constexpr Vec2 center() const noexcept { return Vec2(x + w * 0.5f, y + h * 0.5f); }

        constexpr bool isEmpty() const noexcept { return w <= 0.f || h <= 0.f; }

        constexpr bool contains(Vec2 p) const noexcept {
            return p.x >= left() && p.x <= right() && p.y >= top() && p.y <= bottom();
        }
        constexpr bool intersects(Rect const& o) const noexcept {
            return !(right() <= o.left() || left() >= o.right() || bottom() <= o.top() || top() >= o.bottom());
        }

        constexpr Rect intersection(Rect const& o) const noexcept {
            float nx = std::max(left(), o.left());
            float ny = std::max(top(), o.top());
            float nw = std::min(right(), o.right()) - nx;
            float nh = std::min(bottom(), o.bottom()) - ny;
            return (nw > 0.f && nh > 0.f) ? Rect(nx, ny, nw, nh) : Rect();
        }

        constexpr Rect unionWith(Rect const& o) const noexcept {
            float nx = std::min(left(), o.left());
            float ny = std::min(top(), o.top());
            float nw = std::max(right(), o.right()) - nx;
            float nh = std::max(bottom(), o.bottom()) - ny;
            return Rect(nx, ny, nw, nh);
        }

        constexpr Rect expanded(float px, float py) const noexcept {
            return Rect(x - px, y - py, w + 2 * px, h + 2 * py);
        }

        constexpr bool operator==(Rect const& o) const noexcept {
            return x == o.x && y == o.y && w == o.w && h == o.h;
        }

        static constexpr Rect maxRect() noexcept {
            constexpr float big = 1e18f;
            return Rect(-big, -big, 2.f * big, 2.f * big);
        }
    };

    struct Mat3 {
        float m[3][3] = {
            {1.f, 0.f, 0.f},
            {0.f, 1.f, 0.f},
            {0.f, 0.f, 1.f}
        };

        static constexpr Mat3 identity() noexcept { return Mat3(); }
        static constexpr Mat3 translate(float tx, float ty) noexcept {
            Mat3 result = identity();
            result.m[0][2] = tx;
            result.m[1][2] = ty;
            return result;
        }

        static constexpr Mat3 scale(float sx, float sy) noexcept {
            Mat3 result = identity();
            result.m[0][0] = sx;
            result.m[1][1] = sy;
            return result;
        }

        static constexpr Mat3 rotate(float radians) noexcept {
            Mat3 result = identity();
            float c = std::cos(radians);
            float s = std::sin(radians);
            result.m[0][0] = c;
            result.m[0][1] = -s;
            result.m[1][0] = s;
            result.m[1][1] = c;
            return result;
        }

        constexpr Vec2 transformPoint(Vec2 v) const noexcept {
            return Vec2(
                m[0][0] * v.x + m[0][1] * v.y + m[0][2],
                m[1][0] * v.x + m[1][1] * v.y + m[1][2]
            );
        }

        constexpr Vec2 transformDir(Vec2 v) const noexcept {
            return Vec2(
                m[0][0] * v.x + m[0][1] * v.y,
                m[1][0] * v.x + m[1][1] * v.y
            );
        }

        constexpr Rect transformRect(Rect r) const noexcept {
            Vec2 p0 = transformPoint(r.topLeft());
            Vec2 p1 = transformPoint(r.topRight());
            Vec2 p2 = transformPoint(r.bottomRight());
            Vec2 p3 = transformPoint(r.bottomLeft());

            float minX = std::min(std::min(p0.x, p1.x), std::min(p2.x, p3.x));
            float maxX = std::max(std::max(p0.x, p1.x), std::max(p2.x, p3.x));
            float minY = std::min(std::min(p0.y, p1.y), std::min(p2.y, p3.y));
            float maxY = std::max(std::max(p0.y, p1.y), std::max(p2.y, p3.y));

            return Rect(minX, minY, maxX - minX, maxY - minY);
        }

        bool inverseAffine(Mat3& out) const noexcept {
            float a = m[0][0], b = m[0][1], tx = m[0][2];
            float c = m[1][0], d = m[1][1], ty = m[1][2];
            float det = a * d - b * c;
            if (std::fabs(det) <= 1e-8f) {
                return false;
            }

            float invDet = 1.f / det;
            out = identity();
            out.m[0][0] = d * invDet;
            out.m[0][1] = -b * invDet;
            out.m[1][0] = -c * invDet;
            out.m[1][1] = a * invDet;
            out.m[0][2] = -(out.m[0][0] * tx + out.m[0][1] * ty);
            out.m[1][2] = -(out.m[1][0] * tx + out.m[1][1] * ty);
            return true;
        }

        constexpr Mat3 operator*(Mat3 const& o) const noexcept {
            Mat3 result;
            for (int col = 0; col < 3; ++col) {
                for (int row = 0; row < 3; ++row) {
                    result.m[row][col] = m[row][0] * o.m[0][col] + m[row][1] * o.m[1][col] + m[row][2] * o.m[2][col];
                }
            }
            return result;
        }
    };
}

template <>
struct fmt::formatter<Helios::Vec2> {
    constexpr auto parse(format_parse_context& ctx) noexcept { return ctx.begin(); }

    template <typename FormatContext>
    auto format(Helios::Vec2 const& v, FormatContext& ctx) const noexcept {
        return format_to(ctx.out(), "({}, {})", v.x, v.y);
    }
};

template <>
struct fmt::formatter<Helios::Rect> {
    constexpr auto parse(format_parse_context& ctx) noexcept { return ctx.begin(); }

    template <typename FormatContext>
    auto format(Helios::Rect const& r, FormatContext& ctx) const noexcept {
        return format_to(ctx.out(), "(x: {}, y: {}, w: {}, h: {})", r.x, r.y, r.w, r.h);
    }
};

template <>
struct fmt::formatter<Helios::Color> {
    constexpr auto parse(format_parse_context& ctx) noexcept { return ctx.begin(); }

    template <typename FormatContext>
    auto format(Helios::Color const& c, FormatContext& ctx) const noexcept {
        return format_to(ctx.out(), "(r: {}, g: {}, b: {}, a: {})", c.r, c.g, c.b, c.a);
    }
};

template <>
struct fmt::formatter<Helios::Vec4> {
    constexpr auto parse(format_parse_context& ctx) noexcept { return ctx.begin(); }

    template <typename FormatContext>
    auto format(Helios::Vec4 const& v, FormatContext& ctx) noexcept {
        return format_to(ctx.out(), "({}, {}, {}, {})", v.r, v.g, v.b, v.a);
    }
};

template <>
struct fmt::formatter<Helios::Mat3> {
    constexpr auto parse(format_parse_context& ctx) noexcept { return ctx.begin(); }

    template <typename FormatContext>
    auto format(Helios::Mat3 const& m, FormatContext& ctx) const noexcept {
        return format_to(ctx.out(),
            "[[{}, {}, {}], [{}, {}, {}], [{}, {}, {}]]",
            m.m[0][0], m.m[0][1], m.m[0][2],
            m.m[1][0], m.m[1][1], m.m[1][2],
            m.m[2][0], m.m[2][1], m.m[2][2]
        );
    }
};

#endif // HELIOS_MATH_HPP