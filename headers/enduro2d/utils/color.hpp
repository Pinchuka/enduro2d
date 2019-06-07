/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "_utils.hpp"

namespace e2d
{
    class color final {
    public:
        f32 r = 1.f;
        f32 g = 1.f;
        f32 b = 1.f;
        f32 a = 1.f;
    public:
        static constexpr color clear() noexcept;
        static constexpr color black() noexcept;
        static constexpr color white() noexcept;
        static constexpr color red() noexcept;
        static constexpr color green() noexcept;
        static constexpr color blue() noexcept;
        static constexpr color yellow() noexcept;
        static constexpr color magenta() noexcept;
        static constexpr color cyan() noexcept;
    public:
        constexpr color() noexcept = default;

        constexpr color(const color& other) noexcept = default;
        color& operator=(const color& other) noexcept = default;

        constexpr color(f32 r, f32 g, f32 b, f32 a = 1.f) noexcept;
        constexpr explicit color(const color32& other) noexcept;

        [[nodiscard]] f32* data() noexcept;
        [[nodiscard]] const f32* data() const noexcept;

        [[nodiscard]] f32& operator[](std::size_t index) noexcept;
        [[nodiscard]] f32 operator[](std::size_t index) const noexcept;

        color& operator+=(f32 v) noexcept;
        color& operator-=(f32 v) noexcept;
        color& operator*=(f32 v) noexcept;
        color& operator/=(f32 v) noexcept;

        color& operator+=(const color& other) noexcept;
        color& operator-=(const color& other) noexcept;
        color& operator*=(const color& other) noexcept;
        color& operator/=(const color& other) noexcept;
    };
}

namespace e2d
{
    [[nodiscard]] bool operator<(const color& l, const color& r) noexcept;
    [[nodiscard]] bool operator==(const color& l, const color& r) noexcept;
    [[nodiscard]] bool operator!=(const color& l, const color& r) noexcept;

    [[nodiscard]] color operator+(color l, f32 v) noexcept;
    [[nodiscard]] color operator-(color l, f32 v) noexcept;
    [[nodiscard]] color operator*(color l, f32 v) noexcept;
    [[nodiscard]] color operator/(color l, f32 v) noexcept;

    [[nodiscard]] color operator+(f32 v, const color& r) noexcept;
    [[nodiscard]] color operator-(f32 v, const color& r) noexcept;
    [[nodiscard]] color operator*(f32 v, const color& r) noexcept;
    [[nodiscard]] color operator/(f32 v, const color& r) noexcept;

    [[nodiscard]] color operator+(color l, const color& r) noexcept;
    [[nodiscard]] color operator-(color l, const color& r) noexcept;
    [[nodiscard]] color operator*(color l, const color& r) noexcept;
    [[nodiscard]] color operator/(color l, const color& r) noexcept;
}

namespace e2d::math
{
    [[nodiscard]] bool approximately(const color& l, const color& r) noexcept;
    [[nodiscard]] bool approximately(const color& l, const color& r, f32 precision) noexcept;

    [[nodiscard]] f32 minimum(const color& c) noexcept;
    [[nodiscard]] f32 maximum(const color& c) noexcept;

    [[nodiscard]] color minimized(const color& c, const color& cmin) noexcept;
    [[nodiscard]] color maximized(const color& c, const color& cmax) noexcept;
    [[nodiscard]] color clamped(const color& c, const color& cmin, const color& cmax) noexcept;

    [[nodiscard]] bool contains_nan(const color& c) noexcept;
}

namespace e2d::colors
{
    [[nodiscard]] u32 pack_color(const color& c) noexcept;
    [[nodiscard]] color unpack_color(u32 argb) noexcept;
}

namespace e2d
{
    [[nodiscard]]
    constexpr color color::clear() noexcept {
        return color(0, 0, 0, 0);
    }

    [[nodiscard]]
    constexpr color color::black() noexcept {
        return color(0, 0, 0, 1);
    }

    [[nodiscard]]
    constexpr color color::white() noexcept {
        return color(1, 1, 1, 1);
    }

    [[nodiscard]]
    constexpr color color::red() noexcept {
        return color(1, 0, 0, 1);
    }

    [[nodiscard]]
    constexpr color color::green() noexcept {
        return color(0, 1, 0, 1);
    }

    [[nodiscard]]
    constexpr color color::blue() noexcept {
        return color(0, 0, 1, 1);
    }

    [[nodiscard]]
    constexpr color color::yellow() noexcept {
        return color(1, 1, 0, 1);
    }

    [[nodiscard]]
    constexpr color color::magenta() noexcept {
        return color(1, 0, 1, 1);
    }

    [[nodiscard]]
    constexpr color color::cyan() noexcept {
        return color(0, 1, 1, 1);
    }

    constexpr color::color(f32 nr, f32 ng, f32 nb, f32 na) noexcept
    : r(nr)
    , g(ng)
    , b(nb)
    , a(na) {}
}
