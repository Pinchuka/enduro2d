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
    // bad_http_request
    //

    class bad_http_request final : public exception {
    public:
        const char* what() const noexcept final {
            return "bad http request";
        }
    };

    //
    // http_response_not_ready
    //

    class http_response_not_ready final : public exception {
    public:
        const char* what() const noexcept final {
            return "http response is not ready";
        }
    };

    //
    // http_code
    //
    enum class http_code : u16 {
        // non-http codes
        unknown = 0xFFFF,
        operation_timeout = 0x1001,
        connection_error = 0x1002,
        timeout_after_last_response = 0x1003,
        // http codes
        ok = 200,
        moved_permanently = 301,
        request_timeout = 408,
        // TODO
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
        http_request(const char*, method);
        http_request(str_view, method);
        http_request(const e2d::url&, method);
        http_request(str&&, method) noexcept;
        http_request(http_request&&) = default;
        http_request& timeout(secf value) noexcept;
        http_request& timeout(f32 value) noexcept;
        http_request& header(const str&, const str&);
        http_request& content(input_stream_uptr);
        http_request& content(buffer_view value);
        http_request& content(str_view value);
        http_request& content(data_t&& value);
        http_request& content(const void* data, std::size_t size);
        http_request& append_content(buffer_view value);
        http_request& output_stream(output_stream_uptr) noexcept;
        http_request& url(const char*);
        http_request& url(str_view);
        http_request& url(const e2d::url&);
        http_request& url(str&&) noexcept;
        http_request& redirections(u32 count) noexcept;
        [[nodiscard]] secf timeout() const noexcept;
        [[nodiscard]] const str& url() const noexcept;
        [[nodiscard]] const flat_map<str, str>& headers() const noexcept;
        [[nodiscard]] method type() const noexcept;
        [[nodiscard]] const data_t& content_data() const; // throw(stdex::bad_variant_access)
        [[nodiscard]] const input_stream_uptr& content_stream() const; // throw(stdex::bad_variant_access)
        [[nodiscard]] const output_stream_uptr& output_stream() const noexcept;
        [[nodiscard]] u32 redirections() const noexcept;
    private:
        [[nodiscard]] data_t& edit_content_data() noexcept;
        [[nodiscard]] input_stream_uptr& edit_content_stream() noexcept;
    private:
        str url_;
        method method_;
        flat_map<str, str> headers_;
        secf timeout_;
        u32 max_redirections_ {~0u}; // default - unlimited number of redirections
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
        using promise_t = stdex::promise<http_response>;
        class internal_state final : public std::enable_shared_from_this<internal_state> {
        public:
            enum class status {
                pending, // response data locked by network implementation
                ready, // response data is readonly and available for all
                canceled, // response data is readonly, but may be empty or incomplete
            };
            internal_state() = default;
        public:
            mutable std::atomic<status> status_ {status::pending};
            flat_map<str, str> headers_;
            std::vector<u8> content_;
            http_code status_code_ {http_code::unknown};
            promise_t handler_;
        };
        using internal_state_ptr = std::shared_ptr<internal_state>;
        using status = internal_state::status;
    public:
        http_response(const http_response&) = default;
        http_response(http_response&&) = default;
        http_response(internal_state_ptr) noexcept;
        bool cancel() const;
        void wait() const;
        [[nodiscard]] promise_t& handler() noexcept;
        [[nodiscard]] bool ready() const noexcept;
        [[nodiscard]] bool canceled() const noexcept;
        [[nodiscard]] http_code status_code() const;
        [[nodiscard]] const std::vector<u8>& content() const;
        [[nodiscard]] str_view content_as_str() const;
        [[nodiscard]] const flat_map<str, str>& headers() const;
    private:
        internal_state_ptr state_;
    };

    //
    // network
    //

    class network final : public module<network> {
    public:
        network(debug& d);
        ~network() noexcept;

        [[nodiscard]] http_response send(http_request&&);
        [[nodiscard]] http_response send(http_request&);
    private:
        class internal_state;
        std::unique_ptr<internal_state> state_;
    };
}
