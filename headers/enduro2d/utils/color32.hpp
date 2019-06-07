/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "_utils.hpp"
#include <enduro2d/utils/color.hpp>

namespace e2d
{
    class color32 final {
    public:
        u8 r = 255;
        u8 g = 255;
        u8 b = 255;
        u8 a = 255;
    public:
        static constexpr color32 clear() noexcept;
        static constexpr color32 black() noexcept;
        static constexpr color32 white() noexcept;
        static constexpr color32 red() noexcept;
        static constexpr color32 green() noexcept;
        static constexpr color32 blue() noexcept;
        static constexpr color32 yellow() noexcept;
        static constexpr color32 magenta() noexcept;
        static constexpr color32 cyan() noexcept;
    public:
        constexpr color32() noexcept = default;

        constexpr color32(const color32& other) noexcept = default;
        color32& operator=(const color32& other) noexcept = default;

        constexpr color32(u8 r, u8 g, u8 b, u8 a = 255) noexcept;
        constexpr explicit color32(const color& other) noexcept;

        [[nodiscard]] u8* data() noexcept;
        [[nodiscard]] const u8* data() const noexcept;

        [[nodiscard]] u8& operator[](std::size_t index) noexcept;
        [[nodiscard]] u8 operator[](std::size_t index) const noexcept;

        color32& operator+=(u8 v) noexcept;
        color32& operator-=(u8 v) noexcept;
        color32& operator*=(u8 v) noexcept;
        color32& operator/=(u8 v) noexcept;

        color32& operator+=(const color32& other) noexcept;
        color32& operator-=(const color32& other) noexcept;
        color32& operator*=(const color32& other) noexcept;
        color32& operator/=(const color32& other) noexcept;
    };
}

namespace e2d
{
    [[nodiscard]] bool operator<(const color32& l, const color32& r) noexcept;
    [[nodiscard]] bool operator==(const color32& l, const color32& r) noexcept;
    [[nodiscard]] bool operator!=(const color32& l, const color32& r) noexcept;

    [[nodiscard]] color32 operator+(color32 l, u8 v) noexcept;
    [[nodiscard]] color32 operator-(color32 l, u8 v) noexcept;
    [[nodiscard]] color32 operator*(color32 l, u8 v) noexcept;
    [[nodiscard]] color32 operator/(color32 l, u8 v) noexcept;

    [[nodiscard]] color32 operator+(u8 v, const color32& r) noexcept;
    [[nodiscard]] color32 operator-(u8 v, const color32& r) noexcept;
    [[nodiscard]] color32 operator*(u8 v, const color32& r) noexcept;
    [[nodiscard]] color32 operator/(u8 v, const color32& r) noexcept;

    [[nodiscard]] color32 operator+(color32 l, const color32& r) noexcept;
    [[nodiscard]] color32 operator-(color32 l, const color32& r) noexcept;
    [[nodiscard]] color32 operator*(color32 l, const color32& r) noexcept;
    [[nodiscard]] color32 operator/(color32 l, const color32& r) noexcept;
}

namespace e2d::math
{
    [[nodiscard]] bool approximately(const color32& l, const color32& r) noexcept;
    [[nodiscard]] bool approximately(const color32& l, const color32& r, u8 precision) noexcept;

    [[nodiscard]] u8 minimum(const color32& c) noexcept;
    [[nodiscard]] u8 maximum(const color32& c) noexcept;

    [[nodiscard]] color32 minimized(const color32& c, const color32& cmin) noexcept;
    [[nodiscard]] color32 maximized(const color32& c, const color32& cmax) noexcept;
    [[nodiscard]] color32 clamped(const color32& c, const color32& cmin, const color32& cmax) noexcept;

    [[nodiscard]] bool contains_nan(const color32& c) noexcept;
}

namespace e2d::colors
{
    [[nodiscard]] u32 pack_color32(const color32& c) noexcept;
    [[nodiscard]] color32 unpack_color32(u32 argb) noexcept;
}

namespace e2d
{
    [[nodiscard]]
    constexpr color32 color32::clear() noexcept {
        return color32(0, 0, 0, 0);
    }

    [[nodiscard]]
    constexpr color32 color32::black() noexcept {
        return color32(0, 0, 0, 255);
    }

    [[nodiscard]]
    constexpr color32 color32::white() noexcept {
        return color32(255, 255, 255, 255);
    }

    [[nodiscard]]
    constexpr color32 color32::red() noexcept {
        return color32(255, 0, 0, 255);
    }

    [[nodiscard]]
    constexpr color32 color32::green() noexcept {
        return color32(0, 255, 0, 255);
    }

    [[nodiscard]]
    constexpr color32 color32::blue() noexcept {
        return color32(0, 0, 255, 255);
    }

    [[nodiscard]]
    constexpr color32 color32::yellow() noexcept {
        return color32(255, 255, 0, 255);
    }

    [[nodiscard]]
    constexpr color32 color32::magenta() noexcept {
        return color32(255, 0, 255, 255);
    }

    [[nodiscard]]
    constexpr color32 color32::cyan() noexcept {
        return color32(0, 255, 255, 255);
    }

    constexpr color32::color32(const color& other) noexcept
    : r(math::numeric_cast<u8>(other.r * 255.f + 0.5f))
    , g(math::numeric_cast<u8>(other.g * 255.f + 0.5f))
    , b(math::numeric_cast<u8>(other.b * 255.f + 0.5f))
    , a(math::numeric_cast<u8>(other.a * 255.f + 0.5f)) {}

    constexpr color32::color32(u8 nr, u8 ng, u8 nb, u8 na) noexcept
    : r(nr)
    , g(ng)
    , b(nb)
    , a(na) {}

    constexpr color::color(const color32& other) noexcept
    : r(other.r / 255.f)
    , g(other.g / 255.f)
    , b(other.b / 255.f)
    , a(other.a / 255.f) {}
}
