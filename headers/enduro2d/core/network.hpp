/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "_core.hpp"
#include <enduro2d/utils/url.hpp>
#include <deque>

namespace e2d
{
    //
    // bad_network_operation
    //

    class bad_network_operation final : public exception {
    public:
        const char* what() const noexcept final {
            return "bad network operation";
        }
    };

    //
    // http_request
    //

    class http_request final {
    public:
        enum class method : u8 {
            get,
            post,
            head
        };
        using data_t = std::vector<u8>;
        using content_t = stdex::variant<stdex::monostate, data_t, input_stream_uptr>;
    public:
        explicit http_request(method) noexcept;
        http_request(str_view, method) noexcept;
        http_request(const e2d::url&, method) noexcept;
        http_request(str&&, method) noexcept;
        http_request(http_request&&) = default;
        http_request& timeout(secf value) noexcept;
        http_request& header(const str&, const str&) noexcept;
        http_request& content(output_stream_uptr) noexcept;
        http_request& content(buffer_view value) noexcept;
        http_request& content(str_view value) noexcept;
        http_request& content(data_t&& value) noexcept;
        http_request& content(const void* data, std::size_t size) noexcept;
        http_request& append_content(buffer_view value) noexcept;
        http_request& output_stream(output_stream_uptr) noexcept;
        http_request& url(const char*) noexcept;
        http_request& url(str_view) noexcept;
        http_request& url(const e2d::url&) noexcept;
        http_request& url(str&&) noexcept;
        [[nodiscard]] secf timeout() const noexcept;
        [[nodiscard]] const str& url() const noexcept;
        [[nodiscard]] const flat_map<str, str>& headers() const noexcept;
        [[nodiscard]] method type() const noexcept;
        [[nodiscard]] const data_t& content_data() const; // throw(stdex::bad_variant_access)
        [[nodiscard]] const input_stream_uptr& content_stream() const; // throw(stdex::bad_variant_access)
        [[nodiscard]] const output_stream_uptr& output_stream() const noexcept;
    private:
        [[nodiscard]] data_t& edit_content_data() noexcept;
        [[nodiscard]] input_stream_uptr& edit_content_stream() noexcept;
    private:
        str url_;
        method method_;
        flat_map<str, str> headers_;
        secf timeout_;
    private:
        friend class network;
        content_t content_;
        output_stream_uptr output_stream_;
    };

    //
    // http_response
    //

    class http_response final {
    public:
        http_response(
            u16 status,
            flat_map<str, str>&& headers,
            std::vector<u8>&& content);
        [[nodiscard]] u16 status_code() const noexcept;
        [[nodiscard]] const std::vector<u8>& content() const noexcept;
        [[nodiscard]] const flat_map<str, str>& headers() const noexcept;
    private:
        flat_map<str, str> headers_;
        std::vector<u8> content_;
        u16 status_;
    };

    //
    // network
    //

    class network final : public module<network> {
    public:
        network(debug& d);
        ~network() noexcept;

        [[nodiscard]] stdex::promise<http_response> send(http_request&&);
        [[nodiscard]] stdex::promise<http_response> send(http_request&);
        void tick();
    private:
        class internal_state;
        std::unique_ptr<internal_state> state_;
    };
}
