/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "_utils.hpp"

namespace e2d
{
    template < typename T >
    [[nodiscard]]
    seconds<T> make_seconds(T v) noexcept {
        return make_unit<seconds_tag>(v);
    }

    template < typename T >
    [[nodiscard]]
    milliseconds<T> make_milliseconds(T v) noexcept {
        return make_unit<milliseconds_tag>(v);
    }

    template < typename T >
    [[nodiscard]]
    microseconds<T> make_microseconds(T v) noexcept {
        return make_unit<microseconds_tag>(v);
    }

    template <>
    struct unit_converter<seconds_tag, milliseconds_tag> {
        template < typename T >
        [[nodiscard]]
        milliseconds<T> operator()(const seconds<T>& u) const noexcept {
            const i64 seconds_to_milli = 1000;
            return make_milliseconds(u.value * seconds_to_milli)
                .template cast_to<T>();
        }
    };

    template <>
    struct unit_converter<seconds_tag, microseconds_tag> {
        template < typename T >
        [[nodiscard]]
        microseconds<T> operator()(const seconds<T>& u) const noexcept {
            const i64 seconds_to_micro = 1000 * 1000;
            return make_microseconds(u.value * seconds_to_micro)
                .template cast_to<T>();
        }
    };

    template <>
    struct unit_converter<milliseconds_tag, seconds_tag> {
        template < typename T >
        [[nodiscard]]
        seconds<T> operator()(const milliseconds<T>& u) const noexcept {
            const f64 milli_to_seconds = 1.0 / 1000.0;
            return make_seconds(u.value * milli_to_seconds)
                .template cast_to<T>();
        }
    };

    template <>
    struct unit_converter<milliseconds_tag, microseconds_tag> {
        template < typename T >
        [[nodiscard]]
        microseconds<T> operator()(const milliseconds<T>& u) const noexcept {
            const i64 milli_to_micro = 1000;
            return make_microseconds(u.value * milli_to_micro)
                .template cast_to<T>();
        }
    };

    template <>
    struct unit_converter<microseconds_tag, seconds_tag> {
        template < typename T >
        [[nodiscard]]
        seconds<T> operator()(const microseconds<T>& u) const noexcept {
            const f64 micro_to_seconds = 1.0 / 1000.0 / 1000.0;
            return make_seconds(u.value * micro_to_seconds)
                .template cast_to<T>();
        }
    };

    template <>
    struct unit_converter<microseconds_tag, milliseconds_tag> {
        template < typename T >
        [[nodiscard]]
        milliseconds<T> operator()(const microseconds<T>& u) const noexcept {
            const f64 micro_to_milli = 1.0 / 1000.0;
            return make_milliseconds(u.value * micro_to_milli)
                .template cast_to<T>();
        }
    };
}

namespace e2d::time
{
    template < typename T >
    [[nodiscard]]
    std::chrono::duration<T, std::ratio<1ll, 1ll>>
    to_chrono(const unit<T, seconds_tag>& u) noexcept {
        return std::chrono::duration<T, std::ratio<1ll>>(u.value);
    }

    template < typename T >
    [[nodiscard]]
    std::chrono::duration<T, std::ratio<1ll, 1000ll>>
    to_chrono(const unit<T, milliseconds_tag>& u) noexcept {
        return std::chrono::duration<T, std::ratio<1ll, 1000ll>>(u.value);
    }

    template < typename T >
    [[nodiscard]]
    std::chrono::duration<T, std::ratio<1ll, 1'000'000ll>>
    to_chrono(const unit<T, microseconds_tag>& u) noexcept {
        return std::chrono::duration<T, std::ratio<1ll, 1'000'000ll>>(u.value);
    }

    template < typename T, typename Tag >
    [[nodiscard]]
    constexpr seconds<T> to_seconds(const unit<T, Tag>& u) noexcept {
        return u.template convert_to<seconds_tag>();
    }

    template < typename T, typename Tag >
    [[nodiscard]]
    constexpr milliseconds<T> to_milliseconds(const unit<T, Tag>& u) noexcept {
        return u.template convert_to<milliseconds_tag>();
    }

    template < typename T, typename Tag >
    [[nodiscard]]
    constexpr microseconds<T> to_microseconds(const unit<T, Tag>& u) noexcept {
        return u.template convert_to<microseconds_tag>();
    }
}

namespace e2d::time
{
    template < typename T >
    [[nodiscard]]
    constexpr seconds<T> second() noexcept {
        return seconds<T>(T(1));
    }

    template < typename T >
    [[nodiscard]]
    constexpr milliseconds<T> second_ms() noexcept {
        return to_milliseconds(second<T>());
    }

    template < typename T >
    [[nodiscard]]
    constexpr microseconds<T> second_us() noexcept {
        return to_microseconds(second<T>());
    }

    template < typename T >
    [[nodiscard]]
    constexpr seconds<T> minute() noexcept {
        return second<T>() * T(60);
    }

    template < typename T >
    [[nodiscard]]
    constexpr milliseconds<T> minute_ms() noexcept {
        return to_milliseconds(minute<T>());
    }

    template < typename T >
    [[nodiscard]]
    constexpr microseconds<T> minute_us() noexcept {
        return to_microseconds(minute<T>());
    }

    template < typename T >
    [[nodiscard]]
    constexpr seconds<T> hour() noexcept {
        return minute<T>() * T(60);
    }

    template < typename T >
    [[nodiscard]]
    constexpr milliseconds<T> hour_ms() noexcept {
        return to_milliseconds(hour<T>());
    }

    template < typename T >
    [[nodiscard]]
    constexpr microseconds<T> hour_us() noexcept {
        return to_microseconds(hour<T>());
    }
}

namespace e2d::time
{
    template < typename TimeTag, typename T = i64 >
    [[nodiscard]]
    unit<T, TimeTag> now() noexcept {
        namespace ch = std::chrono;
        const auto n = ch::high_resolution_clock::now();
        const auto m = ch::time_point_cast<ch::microseconds>(n);
        const auto c = m.time_since_epoch().count();
        return make_microseconds(c).cast_to<T>().template convert_to<TimeTag>();
    }

    template < typename T = i64 >
    [[nodiscard]]
    unit<T, seconds_tag> now_s() noexcept {
        return now<seconds_tag, T>();
    }

    template < typename T = i64 >
    [[nodiscard]]
    unit<T, milliseconds_tag> now_ms() noexcept {
        return now<milliseconds_tag, T>();
    }

    template < typename T = i64 >
    [[nodiscard]]
    unit<T, microseconds_tag> now_us() noexcept {
        return now<microseconds_tag, T>();
    }
}
