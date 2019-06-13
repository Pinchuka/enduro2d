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
    // http_response:internal_state
    //

    class http_response::internal_state final : private e2d::noncopyable {
    public:
        internal_state(
            debug& debug,
            http_request&& request);
        ~internal_state() noexcept;
    public:
        [[nodiscard]] debug& dbg() const noexcept;
        [[nodiscard]] CURL* curl() const noexcept;
        void add(CURLSH*) noexcept;
        void remove(CURLSH*) noexcept;
    private:
        static size_t read_data_callback(char *buffer, size_t size, size_t nitems, void *userdata);
        static size_t read_stream_callback(char *buffer, size_t size, size_t nitems, void *userdata);
        static size_t header_callback(char *buffer, size_t size, size_t nitems, void *userdata);
        static size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata);
        static int debug_callback(CURL *handle, curl_infotype type, char *data, size_t size, void *userptr);
    private:
        CURL* curl_;
        curl_slist* slist_;
        http_request request_;
        CURLMcode add_to_curlm_;
        std::atomic<int> sent_;
        flat_map<str, str> headers_;
        std::vector<u8> content_;
        debug& debug_;
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
        [[nodiscard]] debug& dbg() const noexcept;
        [[nodiscard]] CURLM* curl() const noexcept;
        [[nodiscard]] CURLSH* curl_shared() const noexcept;
    private:
        CURLM* curl_;
        CURLSH* curl_shared_;
        debug& debug_;
    };
}

#endif
