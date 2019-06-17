/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "network.hpp"

#if defined(E2D_NETWORK_MODE) && E2D_NETWORK_MODE == E2D_NETWORK_MODE_CURL
#include <curl/curl.h>

namespace e2d
{
    //
    // curl_http_request
    //

    class curl_http_request final : private e2d::noncopyable {
    public:
        curl_http_request(
            debug& debug,
            str_view url,
            const flat_map<str, str>& headers,
            http_request::method method,
            secf timeout,
            http_request::content_t &&content,
            output_stream_uptr &&stream,
            stdex::promise<http_response> result);
        ~curl_http_request() noexcept;
    public:
        [[nodiscard]] debug& dbg() const noexcept;
        [[nodiscard]] CURL* curl() const noexcept;
        [[nodiscard]] bool is_complete() noexcept;
        void enque(CURLM*) noexcept;
        void complete(CURLM*, CURLcode);
        void cancel() noexcept;
    private:
        static size_t read_data_callback(char *buffer, size_t size, size_t nitems, void *userdata);
        static size_t read_stream_callback(char *buffer, size_t size, size_t nitems, void *userdata);
        static size_t header_callback(char *buffer, size_t size, size_t nitems, void *userdata);
        static size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata);
        static int debug_callback(CURL *handle, curl_infotype type, char *data, size_t size, void *userptr);
    private:
        // request
        const secf timeout_;
        const http_request::content_t content_;
        //
        debug& debug_;
        CURL* curl_;
        curl_slist* slist_;
        CURLMcode add_to_curlm_result_;
        CURLcode completion_result_;
        http_code http_code_;
        std::atomic<int> sent_;
        std::atomic<bool> canceled_;
        // response
        using time_point_t = std::chrono::high_resolution_clock::time_point;
        time_point_t last_response_time_;
        flat_map<str, str> response_headers_;
        std::vector<u8> response_content_;
        output_stream_uptr response_stream_;
        stdex::promise<http_response> result_;
    };

    //
    // network::internal_state
    //

    class network::internal_state final : private e2d::noncopyable {
    public:
        internal_state(
            debug& debug);
        ~internal_state() noexcept;
    public:
        using curl_http_request_uptr = std::unique_ptr<curl_http_request>;
        void enque(curl_http_request_uptr);
        void tick();
        [[nodiscard]] debug& dbg() const noexcept;
    private:
        using requests_t = flat_map<CURL*, curl_http_request_uptr>;
        requests_t requests_;
        CURLM* curl_;
        CURLSH* curl_shared_;
        debug& debug_;
        std::atomic<bool> exit_;
        std::thread thread_;
    };
}

#endif
