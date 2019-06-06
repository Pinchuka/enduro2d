/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "_math.hpp"
#include "unit.hpp"

namespace e2d::math
{
    template < typename T >
    [[nodiscard]]
    std::enable_if_t<
        std::is_floating_point_v<T>,
        rad<T>>
    pi() noexcept {
        return rad<T>(T(3.14159265358979323846264338327950288));
    }

    template < typename T >
    [[nodiscard]]
    std::enable_if_t<
        std::is_floating_point_v<T>,
        rad<T>>
    inv_pi() noexcept {
        return T(1) / pi<T>();
    }

    template < typename T >
    [[nodiscard]]
    std::enable_if_t<
        std::is_floating_point_v<T>,
        rad<T>>
    two_pi() noexcept {
        return pi<T>() * T(2);
    }

    template < typename T >
    [[nodiscard]]
    std::enable_if_t<
        std::is_floating_point_v<T>,
        rad<T>>
    four_pi() noexcept {
        return pi<T>() * T(4);
    }

    template < typename T >
    [[nodiscard]]
    std::enable_if_t<
        std::is_floating_point_v<T>,
        rad<T>>
    half_pi() noexcept {
        return pi<T>() / T(2);
    }

    template < typename T >
    [[nodiscard]]
    std::enable_if_t<
        std::is_floating_point_v<T>,
        rad<T>>
    quarter_pi() noexcept {
        return pi<T>() / T(4);
    }
}

namespace e2d
{
    template < typename T >
    [[nodiscard]]
    deg<T> make_deg(T v) noexcept {
        return make_unit<deg_tag>(v);
    }

    template < typename T >
    [[nodiscard]]
    rad<T> make_rad(T v) noexcept {
        return make_unit<rad_tag>(v);
    }

    template <>
    struct unit_converter<rad_tag, deg_tag> {
        template < typename T >
        std::enable_if_t<std::is_integral_v<T>, deg<T>>
        operator()(const rad<T>& u) const noexcept {
            const f64 rad_to_deg = 180.0 / math::pi<f64>().value;
            return make_deg(u.value * rad_to_deg).template cast_to<T>();
        }

        template < typename T >
        std::enable_if_t<std::is_floating_point_v<T>, deg<T>>
        operator()(const rad<T>& u) const noexcept {
            const T rad_to_deg = T(180) / math::pi<T>().value;
            return make_deg(u.value * rad_to_deg);
        }
    };

    template <>
    struct unit_converter<deg_tag, rad_tag> {
    public:
        template < typename T >
        std::enable_if_t<std::is_integral_v<T>, rad<T>>
        operator()(const deg<T>& u) const noexcept {
            const f64 deg_to_rad = math::pi<f64>().value / 180.0;
            return make_rad(u.value * deg_to_rad).template cast_to<T>();
        }

        template < typename T >
        std::enable_if_t<std::is_floating_point_v<T>, rad<T>>
        operator()(const deg<T>& u) const noexcept {
            const T deg_to_rad = math::pi<T>().value / T(180);
            return make_rad(u.value * deg_to_rad);
        }
    };
}

namespace e2d::math
{
    //
    // to_deg/to_rad
    //

    template < typename T, typename Tag >
    [[nodiscard]]
    deg<T> to_deg(const unit<T, Tag>& u) noexcept {
        return u.template convert_to<deg_tag>();
    }

    template < typename T, typename Tag >
    [[nodiscard]]
    rad<T> to_rad(const unit<T, Tag>& u) noexcept {
        return u.template convert_to<rad_tag>();
    }

    //
    // cos/sin/tan
    //

    template < typename T, typename Tag >
    [[nodiscard]]
    std::enable_if_t<std::is_floating_point_v<T>, T>
    cos(const unit<T, Tag>& u) noexcept {
        return std::cos(to_rad(u).value);
    }

    template < typename T, typename Tag >
    [[nodiscard]]
    std::enable_if_t<std::is_floating_point_v<T>, T>
    sin(const unit<T, Tag>& u) noexcept {
        return std::sin(to_rad(u).value);
    }

    template < typename T, typename Tag >
    [[nodiscard]]
    std::enable_if_t<std::is_floating_point_v<T>, T>
    tan(const unit<T, Tag>& u) noexcept {
        return std::tan(to_rad(u).value);
    }

    //
    // acos, asin, atan
    //

    template < typename T, typename Tag >
    [[nodiscard]]
    std::enable_if_t<std::is_floating_point_v<T>, rad<T>>
    acos(const unit<T, Tag>& u) noexcept {
        return rad<T>(std::acos(to_rad(u).value));
    }

    template < typename T, typename Tag >
    [[nodiscard]]
    std::enable_if_t<std::is_floating_point_v<T>, rad<T>>
    asin(const unit<T, Tag>& u) noexcept {
        return rad<T>(std::asin(to_rad(u).value));
    }

    template < typename T, typename Tag >
    [[nodiscard]]
    std::enable_if_t<std::is_floating_point_v<T>, rad<T>>
    atan(const unit<T, Tag>& u) noexcept {
        return rad<T>(std::atan(to_rad(u).value));
    }

    //
    // atan2
    //

    template < typename T >
    [[nodiscard]]
    std::enable_if_t<std::is_floating_point_v<T>, rad<T>>
    atan2(T y, T x) noexcept {
        return rad<T>(std::atan2(y, x));
    }

    //
    // normalized_angle
    //

    template < typename T, typename Tag >
    [[nodiscard]]
    std::enable_if_t<std::is_floating_point_v<T>, unit<T, Tag>>
    normalized_angle(const unit<T, Tag>& u) noexcept {
        const auto four_pi = math::four_pi<T>().template convert_to<Tag>();
        return u - four_pi * math::floor((u + T(0.5) * four_pi).value / four_pi.value);
    }
}
