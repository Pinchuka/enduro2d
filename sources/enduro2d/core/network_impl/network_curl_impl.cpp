/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include "network_curl_impl.hpp"

#if defined(E2D_NETWORK_MODE) && E2D_NETWORK_MODE == E2D_NETWORK_MODE_CURL

namespace e2d
{
    //
    // curl_http_request
    //

    curl_http_request::curl_http_request(
        debug& debug,
        str_view url,
        const flat_map<str, str>& headers,
        http_request::method method,
        secf timeout,
        u32 maxRedirections,
        http_request::content_t &&content,
        output_stream_uptr &&stream,
        const response_t& result)
    : timeout_(timeout)
    , content_(std::move(content))
    , url_(url)
    , debug_(debug)
    , slist_(nullptr)
    , add_to_curlm_result_(CURLM_OK)
    , completion_result_(CURLE_OK)
    , http_code_(http_code::unknown)
    , response_stream_(std::move(stream))
    , result_(result) {
        curl_ = curl_easy_init();
        curl_easy_setopt(curl_, CURLOPT_DEBUGDATA, this);
        curl_easy_setopt(curl_, CURLOPT_DEBUGFUNCTION, &debug_callback);
        curl_easy_setopt(curl_, CURLOPT_VERBOSE, 1L); // for debug_callback
        curl_easy_setopt(curl_, CURLOPT_BUFFERSIZE, 64*1024);
        curl_easy_setopt(curl_, CURLOPT_NOSIGNAL, 1L);
        curl_easy_setopt(curl_, CURLOPT_PRIVATE, this);
        //curl_easy_setopt(curl_, CURLOPT_PROXY, );
        curl_easy_setopt(curl_, CURLOPT_NOPROGRESS, 1L);

        if ( maxRedirections ) {
            curl_easy_setopt(curl_, CURLOPT_FOLLOWLOCATION, 1L);
            curl_easy_setopt(curl_, CURLOPT_MAXREDIRS, maxRedirections);
        } else {
            // disable redirections
            curl_easy_setopt(curl_, CURLOPT_FOLLOWLOCATION, 0L);
        }

        // TODO: for SSL
        curl_easy_setopt(curl_, CURLOPT_USE_SSL, CURLUSESSL_ALL);
        //curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYPEER, 1L);
        //curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(curl_, CURLOPT_SSLCERTTYPE, "PEM");
        if ( true ) {
            //curl_easy_setopt(curl_, CURLOPT_CAINFO, nullptr);
            //curl_easy_setopt(curl_, CURLOPT_SSL_CTX_FUNCTION, sslctx_function);
            //curl_easy_setopt(curl_, CURLOPT_SSL_CTX_DATA, this);
        }

        if ( headers.size() ) {
            for (auto& hdr : headers) {
                slist_ = curl_slist_append(slist_, (str(hdr.first) + ": " + hdr.second).data());
            }
            if ( slist_ ) {
                curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, slist_);
            }
        }

        switch (method)
        {
        case http_request::method::get :
            curl_easy_setopt(curl_, CURLOPT_HTTPGET, 1L);
            if ( auto* data = stdex::get_if<http_request::data_t>(&content_) ) {
                curl_easy_setopt(curl_, CURLOPT_READDATA, this);
                curl_easy_setopt(curl_, CURLOPT_READFUNCTION, &read_data_callback);
            }
            else if ( !stdex::holds_alternative<stdex::monostate>(content_) ) {
                dbg().warning("unsupported content type for http get request");
                throw bad_http_request();
            }
            break;

        case http_request::method::post :
            curl_easy_setopt(curl_, CURLOPT_POST, 1L);
            if ( auto* data = stdex::get_if<http_request::data_t>(&content_) ) {
                curl_easy_setopt(curl_, CURLOPT_POSTFIELDSIZE, data->size());
                curl_easy_setopt(curl_, CURLOPT_READDATA, this);
                curl_easy_setopt(curl_, CURLOPT_READFUNCTION, &read_data_callback);
                curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, nullptr);
            }
            else if ( auto* stream = stdex::get_if<input_stream_uptr>(&content_) ) {
                curl_easy_setopt(curl_, CURLOPT_INFILESIZE, (*stream)->length());
                curl_easy_setopt(curl_, CURLOPT_READDATA, this);
                curl_easy_setopt(curl_, CURLOPT_READFUNCTION, &read_stream_callback);
                curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, nullptr);
            }
            else {
                dbg().warning("unsupported content type for http post request");
                throw bad_http_request();
            }
            break;

        case http_request::method::head :
            curl_easy_setopt(curl_, CURLOPT_NOBODY, 1L);
            break;
        }

        curl_easy_setopt(curl_, CURLOPT_URL, url.data());

        // to receive responce headers
        curl_easy_setopt(curl_, CURLOPT_HEADERDATA, this);
        curl_easy_setopt(curl_, CURLOPT_HEADERFUNCTION, &header_callback);

        // to receive responce data
        curl_easy_setopt(curl_, CURLOPT_WRITEDATA, this);
        curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, &write_callback);

        if ( timeout_.value > 0.0f ) {
            long connection_timeout = math::max<long>(1, math::round(timeout_.value));
            curl_easy_setopt(curl_, CURLOPT_CONNECTTIMEOUT, connection_timeout);
        }

        // TODO
        //curl_easy_setopt(curl_, CURLOPT_ACCEPT_ENCODING, "");
    }

    curl_http_request::~curl_http_request() noexcept {
        if ( curl_ ) {
            curl_easy_setopt(curl_, CURLOPT_DEBUGDATA, nullptr);
            curl_easy_cleanup(curl_);
            curl_ = nullptr;
        }
        if ( slist_ ) {
            curl_slist_free_all(slist_);
            slist_ = nullptr;
        }
    }

    void curl_http_request::enque(CURLM* curlm, CURLSH* shared) noexcept {
        last_response_time_ = time_point_t::clock::now();
        curl_easy_setopt(curl_, CURLOPT_SHARE, shared);
        add_to_curlm_result_ = curl_multi_add_handle(curlm, curl_);
    }

    void curl_http_request::complete(CURLM* curlm, CURLcode code) {
        if ( add_to_curlm_result_ == CURLM_OK ) {
            curl_multi_remove_handle(curlm, curl_);
        }
        completion_result_ = code;
        switch (completion_result_)
        {
        case CURLE_OPERATION_TIMEDOUT :
            http_code_ = http_code::operation_timeout;
            break;
        case CURLE_COULDNT_CONNECT :
        case CURLE_COULDNT_RESOLVE_PROXY :
        case CURLE_COULDNT_RESOLVE_HOST :
            http_code_ = http_code::connection_error;
            break;
        }
        long response_code = 0;
        if ( curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &response_code) == CURLE_OK && response_code != 0 ) {
            http_code_ = http_code(response_code);
        }
        result_->status_code_ = http_code_;

        status expected = status::pending;
        for (; expected == status::pending && !result_->status_.compare_exchange_weak(expected, status::ready);) {
        }
        if ( expected == status::pending ) {
            result_->handler_.resolve(http_response(result_));
        }

        if ( http_code_ != http_code::ok ) {
            dbg().warning("http request code: ($0), url: (%1), curl error: (%2)", int(http_code_), url_, curl_easy_strerror(completion_result_));
        }
    }

    void curl_http_request::cancel() {
        http_response(result_).cancel();
    }

    debug& curl_http_request::dbg() const noexcept {
        return debug_;
    }

    CURL* curl_http_request::curl() const noexcept {
        return curl_;
    }

    bool curl_http_request::is_canceled() const noexcept {
        return result_->status_.load() == status::canceled;
    }

    size_t curl_http_request::read_data_callback(char *buffer, size_t size, size_t nitems, void *userdata) noexcept {
        auto* self = static_cast<curl_http_request *>(userdata);
        if ( self->is_canceled() ) {
            return CURLE_ABORTED_BY_CALLBACK;
        }
        auto& src = stdex::get<http_request::data_t>(self->content_);
        size_t offset = self->result_->bytes_sent_.load();
        size_t required = size * nitems;
        size_t written = 0;
        if ( offset + required <= src.size() ) {
            written = math::min(src.size() - offset, required);
            memcpy(buffer, src.data() + offset, written);
            self->result_->bytes_sent_.fetch_add(written);
        }
        return written;
    }

    size_t curl_http_request::read_stream_callback(char *buffer, size_t size, size_t nitems, void *userdata) noexcept {
        try {
            auto* self = static_cast<curl_http_request *>(userdata);
            if ( self->is_canceled() ) {
                return CURLE_ABORTED_BY_CALLBACK;
            }
            auto& src = stdex::get<input_stream_uptr>(self->content_);
            size_t offset = self->result_->bytes_sent_.load();
            E2D_ASSERT(offset == src->tell());
            size_t required = size * nitems;
            size_t written = src->read(buffer, required);
            self->result_->bytes_sent_.fetch_add(written);
            return written;
        } catch(...) {
            return CURLE_ABORTED_BY_CALLBACK;
        }
    }

    size_t curl_http_request::header_callback(char *buffer, size_t size, size_t nitems, void *userdata) noexcept {
        try {
            auto* self = static_cast<curl_http_request *>(userdata);
            if ( self->is_canceled() ) {
                return 0;
            }
            self->last_response_time_ = time_point_t::clock::now();
            size_t header_size = size * nitems;
            if ( header_size > 0 && header_size <= CURL_MAX_HTTP_HEADER ) {
                str header(buffer, header_size);
                header.erase(std::remove_if(header.begin(), header.end(), [](auto c){ return c == '\n' || c == '\r'; }), header.end());
                if ( header.size() ) {
                    size_t separator = header.find(": ");
                    if ( separator != str_view::npos ) {
                        self->result_->headers_.insert_or_assign(header.substr(0, separator), header.substr(separator+2));
                    } else {
                        self->result_->headers_.insert_or_assign(std::move(header), str());
                    }
                }
                return header_size;
            }
        } catch(...) {
        }
        return 0;
    }

    size_t curl_http_request::write_callback(char *ptr, size_t size, size_t nmemb, void *userdata) noexcept {
        try {
            auto* self = static_cast<curl_http_request *>(userdata);
            if ( self->is_canceled() ) {
                return 0;
            }
            self->last_response_time_ = time_point_t::clock::now();
            size_t buffer_size = size * nmemb;
            size_t offset = self->result_->content_.size();
            if ( buffer_size == 0 )
                return 0;
            self->result_->bytes_received_.fetch_add(buffer_size);
            if ( self->response_stream_ ) {
                return self->response_stream_->write(ptr, buffer_size);
            } else {
                self->result_->content_.resize(self->result_->content_.size() + buffer_size);
                memcpy(self->result_->content_.data() + offset, ptr, buffer_size);
                return buffer_size;
            }
        } catch(...) {
        }
        return 0;
    }

    int curl_http_request::debug_callback(CURL *handle,
                                          curl_infotype type,
                                          char *data,
                                          size_t size,
                                          void *userptr) noexcept {
        auto* self = static_cast<curl_http_request *>(userptr);
        E2D_ASSERT(self == nullptr || handle == self->curl());
        switch (type)
        {
        case CURLINFO_TEXT :
            break;
        case CURLINFO_HEADER_IN :
            break;
        case CURLINFO_HEADER_OUT :
            break;
        case CURLINFO_DATA_IN :
            break;
        case CURLINFO_DATA_OUT :
            break;
        case CURLINFO_SSL_DATA_IN :
            break;
        case CURLINFO_SSL_DATA_OUT :
            break;
        case CURLINFO_END :
            break;
        }
        return 0;
    }

    bool curl_http_request::is_complete() noexcept {
        if ( is_canceled() ) {
            return true;
        }
        if ( add_to_curlm_result_ != CURLM_OK ) {
            return true;
        }
        if ( completion_result_ != CURLE_OK ) {
            return true;
        }
        constexpr auto response_timeout = std::chrono::seconds(60);
        auto dt = time_point_t::clock::now() - last_response_time_;
        if ( dt > response_timeout ) {
            http_code_ = http_code::timeout_after_last_response;
            dbg().warning("http request timed out, url (%0)", url_);
            return true;
        }
        return false;
    }

    //
    // network::internal_state
    //

    network::internal_state::internal_state(
        debug& debug)
    : debug_(debug) {
        const curl_ssl_backend** available = nullptr;
        curl_global_sslset(CURLSSLBACKEND_NONE, nullptr, &available);
        for (u32 i = 0; available[i]; ++i) {
            if ( curl_global_sslset(available[i]->id, nullptr, nullptr) == CURLSSLSET_OK ) {
                dbg().trace("used SSL backend ($0)", available[i]->name);
                break;
            }
        }
        curl_global_init(CURL_GLOBAL_ALL);

        curl_ = curl_multi_init();
        curl_multi_setopt(curl_, CURLMOPT_MAX_TOTAL_CONNECTIONS, 1);
        curl_multi_setopt(curl_, CURLMOPT_MAX_HOST_CONNECTIONS, 1);

        curl_shared_ = curl_share_init();
        curl_share_setopt(curl_shared_, CURLSHOPT_SHARE, CURL_LOCK_DATA_COOKIE);
        curl_share_setopt(curl_shared_, CURLSHOPT_SHARE, CURL_LOCK_DATA_DNS);
        curl_share_setopt(curl_shared_, CURLSHOPT_SHARE, CURL_LOCK_DATA_SSL_SESSION);

        exit_.store(false);
        thread_ = std::thread([this] () {
            using namespace std::chrono ;
            using time_point_t = high_resolution_clock::time_point;
            using nonos = std::chrono::nanoseconds;
            constexpr nonos min_frame_time = duration_cast<nonos>(std::chrono::milliseconds(10));
            while ( !exit_.load() ) {
                try {
                    auto begin_time = time_point_t::clock::now();
                    tick();
                    auto dt = duration_cast<nonos>(time_point_t::clock::now() - begin_time);
                    if ( dt < min_frame_time ) {
                        std::this_thread::sleep_for(min_frame_time - dt);
                    }
                } catch(...) {
                    // TODO ?
                }
            }
        });
    }

    network::internal_state::~internal_state() noexcept {
        exit_.store(true);
        thread_.join();
        try {
            for (auto& request : requests_) {
                request.second->cancel();
            }
            requests_.clear();
        } catch(...) {
        }
        if ( curl_ ) {
            curl_multi_cleanup(curl_);
            curl_ = nullptr;
        }
        if ( curl_shared_ ) {
            curl_share_cleanup(curl_shared_);
            curl_shared_ = nullptr;
        }
        curl_global_cleanup();
    }

    debug& network::internal_state::dbg() const noexcept {
        return debug_;
    }

    void network::internal_state::enque(curl_http_request_uptr value) {
        value->enque(curl_, curl_shared_);
        requests_.insert_or_assign(value->curl(), std::move(value));
    }

    void network::internal_state::tick() {
        int num_handles = -1;
        curl_multi_perform(curl_, &num_handles);
        if ( num_handles == int(requests_.size()) ) {
            return;
        }
        // remove completed requests
        for (;;) {
            int msgs_in_queue = 0;
            CURLMsg* msg = curl_multi_info_read(curl_, &msgs_in_queue);
            if ( !msg ) {
                break;
            }
            if ( msg->msg != CURLMSG_DONE ) {
                continue;
            }
            auto request = requests_.find(msg->easy_handle);
            if ( request != requests_.end() ) {
                request->second->complete(curl_, msg->data.result);
                requests_.erase(request);
            }
        }
        // update active requests
        for (auto request = requests_.begin(); request != requests_.end();) {
            if ( request->second->is_complete() ) {
                request->second->complete(curl_, CURLE_OK);
                request = requests_.erase(request);
            } else {
                ++request;
            }
        }
    }
}

#endif
