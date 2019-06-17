/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include "network.hpp"

namespace e2d
{
    //
    // http_request
    //

    http_request::http_request(method m) noexcept
    : method_(m) {
    }

    http_request::http_request(const char* u, method m)
    : url_(u)
    , method_(m) {
    }

    http_request::http_request(str_view u, method m)
    : url_(u)
    , method_(m) {
    }

    http_request::http_request(const e2d::url& u, method m)
    : url_(u.schemepath())
    , method_(m) {
    }

    http_request::http_request(str&& u, method m) noexcept
    : url_(std::move(u))
    , method_(m) {
    }

    http_request::data_t& http_request::edit_content_data() noexcept {
        if ( !stdex::holds_alternative<data_t>(content_) )
            content_ = data_t();
        return stdex::get<data_t>(content_);
    }

    input_stream_uptr& http_request::edit_content_stream() noexcept {
        if ( !stdex::holds_alternative<input_stream_uptr>(content_) )
            content_ = input_stream_uptr();
        return stdex::get<input_stream_uptr>(content_);
    }

    http_request& http_request::timeout(secf value) noexcept {
        timeout_ = value;
        return *this;
    }

    http_request& http_request::timeout(f32 value) noexcept {
        timeout_ = secf(value);
        return *this;
    }

    http_request& http_request::header(const str& key, const str& value) {
        headers_.insert({key, value});
        return *this;
    }

    http_request& http_request::content(buffer_view value) {
        edit_content_data().assign((u8*)value.data(), (u8*)value.data() + value.size());
        return *this;
    }

    http_request& http_request::content(str_view value) {
        edit_content_data().assign(value.begin(), value.end());
        return *this;
    }

    http_request& http_request::content(data_t&& value) {
        edit_content_data() = std::move(value);
        return *this;
    }

    http_request& http_request::content(const void* data, std::size_t size) {
        E2D_ASSERT(data && size);
        edit_content_data().assign((u8*)data, (u8*)data + size);
        return *this;
    }

    http_request& http_request::append_content(buffer_view value) {
        data_t& d = edit_content_data();
        d.insert(d.end(), (u8*)value.data(), (u8*)value.data() + value.size());
        return *this;
    }

    http_request& http_request::output_stream(output_stream_uptr value) noexcept {
        output_stream_ = std::move(value);
        return *this;
    }

    http_request& http_request::url(const char* value) {
        url_ = value;
        return *this;
    }

    http_request& http_request::url(str_view value) {
        url_ = value;
        return *this;
    }

    http_request& http_request::url(const e2d::url& value) {
        url_ = value.schemepath();
        return *this;
    }

    http_request& http_request::url(str&& value) noexcept {
        url_ = std::move(value);
        return *this;
    }

    const http_request::data_t& http_request::content_data() const {
        return stdex::get<data_t>(content_);
    }

    const input_stream_uptr& http_request::content_stream() const {
        return stdex::get<input_stream_uptr>(content_);
    }

    const output_stream_uptr& http_request::output_stream() const noexcept {
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

    //
    // http_response
    //

    http_response::http_response(
        http_code status,
        flat_map<str, str>&& headers,
        std::vector<u8>&& content)
    : headers_(headers)
    , content_(content)
    , status_(status) {}

    http_code http_response::status_code() const noexcept {
        return status_;
    }

    const std::vector<u8>& http_response::content() const noexcept {
        return content_;
    }

    const flat_map<str, str>& http_response::headers() const noexcept {
        return headers_;
    }
}

