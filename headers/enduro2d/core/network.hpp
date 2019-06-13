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
    // http_request
    //

    class http_request {
    public:
        enum class method : u8 {
            get,
            post,
            head
        };
        using data_t = std::vector<u8>;
    public:
        http_request(str_view, method) noexcept;
        http_request(const e2d::url&, method) noexcept;
        http_request(str&&, method) noexcept;
        http_request& timeout(secf value) noexcept;
        http_request& header(const str&, const str&) noexcept;
        http_request& content(output_stream_uptr) noexcept;
        http_request& content(buffer_view value) noexcept;
        http_request& content(const void* data, std::size_t size) noexcept;
        http_request& append_content(buffer_view value) noexcept;
        http_request& output_stream(input_stream_uptr) noexcept;
        [[nodiscard]] secf timeout() const noexcept;
        [[nodiscard]] const str& url() const noexcept;
        [[nodiscard]] const flat_map<str, str>& headers() const noexcept;
        [[nodiscard]] method type() const noexcept;
        [[nodiscard]] const data_t& content_data() const; // throw(stdex::bad_variant_access)
        [[nodiscard]] const output_stream_uptr& content_stream() const; // throw(stdex::bad_variant_access)
        [[nodiscard]] const input_stream_uptr& output_stream() const noexcept;
    private:
        [[nodiscard]] data_t& content_data() noexcept;
        [[nodiscard]] output_stream_uptr& content_stream() noexcept;
    private:
        using content_t = stdex::variant<stdex::monostate, data_t, output_stream_uptr>;
        content_t content_;
        str url_;
        method method_;
        flat_map<str, str> headers_;
        secf timeout_;
        input_stream_uptr output_stream_;
    };

    //
    // http_response
    //

    class http_response {
    public:
        explicit http_response(const http_request&);
        [[nodiscard]] u16 status_code() const;
    private:
        class internal_state;
        std::unique_ptr<internal_state> state_;
    };

    //
    // network
    //

    class network final : public module<network> {
    public:
        using response_t = stdex::promise<http_response>;
    public:
        network(debug& d);
        ~network() noexcept;

        [[nodiscard]] response_t send(http_request);

        // temp:
        void update();
    private:
        class internal_state;
        std::unique_ptr<internal_state> state_;

        using request_queue_t = std::deque<std::pair<http_request, response_t>>;
        request_queue_t queue_;
    };
}
