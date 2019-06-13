/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include "network.hpp"

namespace e2d
{
    http_request::http_request(str_view u, method m) noexcept
    : url_(u)
    , method_(m) {
    }

    http_request::http_request(const e2d::url& u, method m) noexcept
    : url_(u.schemepath())
    , method_(m) {
    }

    http_request::http_request(str&& u, method m) noexcept
    : url_(std::move(u))
    , method_(m) {
    }

    http_request::data_t& http_request::content_data() noexcept {
        if ( !stdex::holds_alternative<data_t>(content_) )
            content_ = data_t();
        return stdex::get<data_t>(content_);
    }

    output_stream_uptr& http_request::content_stream() noexcept {
        if ( !stdex::holds_alternative<output_stream_uptr>(content_) )
            content_ = output_stream_uptr();
        return stdex::get<output_stream_uptr>(content_);
    }

    http_request& http_request::timeout(secf value) noexcept {
        timeout_ = value;
        return *this;
    }

    http_request& http_request::header(const str& key, const str& value) noexcept {
        headers_.insert({key, value});
        return *this;
    }

    http_request& http_request::content(buffer_view value) noexcept {
        content_data().assign((u8*)value.data(), (u8*)value.data() + value.size());
        return *this;
    }

    http_request& http_request::content(const void* data, std::size_t size) noexcept {
        E2D_ASSERT(data && size);
        content_data().assign((u8*)data, (u8*)data + size);
        return *this;
    }

    http_request& http_request::append_content(buffer_view value) noexcept {
        data_t& d = content_data();
        d.insert(d.end(), (u8*)value.data(), (u8*)value.data() + value.size());
        return *this;
    }

    http_request& http_request::output_stream(input_stream_uptr value) noexcept {
        output_stream_ = std::move(value);
        return *this;
    }

    const http_request::data_t& http_request::content_data() const {
        return stdex::get<data_t>(content_);
    }

    const output_stream_uptr& http_request::content_stream() const {
        return stdex::get<output_stream_uptr>(content_);
    }

    const input_stream_uptr& http_request::output_stream() const noexcept {
        return output_stream_;
    }

    secf http_request::timeout() const noexcept {
        return timeout_;
    }

    const str& http_request::url() const noexcept {
        return url_;
    }

    const flat_map<str, str>& http_request::headers() const noexcept {
        return headers_;
    }

    http_request::method http_request::type() const noexcept {
        return method_;
    }
}

