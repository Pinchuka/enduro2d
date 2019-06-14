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
        http_request::content_t &&content,
        output_stream_uptr &&stream,
        stdex::promise<http_response> result)
    : timeout_(timeout)
    , content_(std::move(content))
    , debug_(debug)
    , slist_(nullptr)
    , add_to_curlm_(CURLM_LAST)
    , response_stream_(std::move(stream))
    , result_(result) {
        curl_ = curl_easy_init();
        curl_easy_setopt(curl_, CURLOPT_DEBUGDATA, this);
        curl_easy_setopt(curl_, CURLOPT_DEBUGFUNCTION, &debug_callback);
        curl_easy_setopt(curl_, CURLOPT_VERBOSE, 1L);
        curl_easy_setopt(curl_, CURLOPT_BUFFERSIZE, 64*1024);
        //curl_easy_setopt(curl_, CURLOPT_SHARE, );
        curl_easy_setopt(curl_, CURLOPT_USE_SSL, CURLUSESSL_ALL);
        curl_easy_setopt(curl_, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl_, CURLOPT_NOSIGNAL, 1L);
        curl_easy_setopt(curl_, CURLOPT_PRIVATE, this);
        //curl_easy_setopt(curl_, CURLOPT_PROXY, );
        curl_easy_setopt(curl_, CURLOPT_NOPROGRESS, 1L);

        // for SSL
        if ( true ) {
            //curl_easy_setopt(curl_, CURLOPT_CAINFO, nullptr);
            //curl_easy_setopt(curl_, CURLOPT_SSL_CTX_FUNCTION, sslctx_function);
            //curl_easy_setopt(curl_, CURLOPT_SSL_CTX_DATA, this);
        }

        if ( headers.size() ) {
            for (auto& hdr : headers) {
                slist_ = curl_slist_append(slist_, (str(hdr.first) + ": " + hdr.second).data());
            }
            curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, slist_);
        }

        switch (method)
        {
        case http_request::method::get :
            curl_easy_setopt(curl_, CURLOPT_HTTPGET, 1L);
            curl_easy_setopt(curl_, CURLOPT_READDATA, this);
            if ( auto* data = stdex::get_if<http_request::data_t>(&content_) ) {
                curl_easy_setopt(curl_, CURLOPT_READFUNCTION, &read_data_callback);
            } else {
                throw bad_network_operation();
            }
            break;

        case http_request::method::post :
            curl_easy_setopt(curl_, CURLOPT_POST, 1L);
            curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, nullptr);
            if ( auto* data = stdex::get_if<http_request::data_t>(&content_) ) {
                curl_easy_setopt(curl_, CURLOPT_POSTFIELDSIZE, data->size());
                curl_easy_setopt(curl_, CURLOPT_READDATA, this);
                curl_easy_setopt(curl_, CURLOPT_READFUNCTION, &read_data_callback);
            }
            else if ( auto* stream = stdex::get_if<input_stream_uptr>(&content_) ) {
                curl_easy_setopt(curl_, CURLOPT_INFILESIZE, (*stream)->length());
                curl_easy_setopt(curl_, CURLOPT_READDATA, this);
                curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, nullptr);
                curl_easy_setopt(curl_, CURLOPT_READFUNCTION, &read_stream_callback);
            }
            else {
                throw bad_network_operation();
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
        curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, write_callback);

        curl_easy_setopt(curl_, CURLOPT_CONNECTTIMEOUT, (long)math::round(timeout_.value));

        // TODO
        //curl_easy_setopt(curl_, CURLOPT_ACCEPT_ENCODING, "");

        canceled_.store(false);
        result_.except([this](std::exception_ptr) {
            canceled_.store(true);
            return http_response(
                    std::move(response_headers_),
                    std::move(response_content_));
        });
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

    void curl_http_request::enque(CURLSH* curlm) noexcept {
        add_to_curlm_ = curl_multi_add_handle(curlm, curl_);
    }

    void curl_http_request::complete(CURLSH* curlm) noexcept {
        curl_multi_remove_handle(curlm, curl_);
        add_to_curlm_ = CURLM_LAST;
    }

    debug& curl_http_request::dbg() const noexcept {
        return debug_;
    }

    CURL* curl_http_request::curl() const noexcept {
        return curl_;
    }

    size_t curl_http_request::read_data_callback(char *buffer, size_t size, size_t nitems, void *userdata) {
        auto* self = static_cast<curl_http_request *>(userdata);
        auto& src = stdex::get<http_request::data_t>(self->content_);
        size_t offset = self->sent_.load();
        size_t required = size * nitems;
        size_t written = 0;
        if ( offset + required <= src.size() ) {
            written = math::min(src.size() - offset, required);
            memcpy(buffer, src.data() + offset, written);
            self->sent_.fetch_add(written);
        }
        return written;
    }

    size_t curl_http_request::read_stream_callback(char *buffer, size_t size, size_t nitems, void *userdata) {
        auto* self = static_cast<curl_http_request *>(userdata);
        auto& src = stdex::get<input_stream_uptr>(self->content_);
        size_t offset = self->sent_.load();
        E2D_ASSERT(offset == src->tell());
        size_t required = size * nitems;
        size_t written = src->read(buffer, required);
        self->sent_.fetch_add(written);
        return written;
    }

    size_t curl_http_request::header_callback(char *buffer, size_t size, size_t nitems, void *userdata) {
        auto* self = static_cast<curl_http_request *>(userdata);
        size_t header_size = size * nitems;
        if ( header_size > 0 && header_size <= CURL_MAX_HTTP_HEADER ) {
            str_view header(buffer, header_size);
            size_t separator = header.find(':');
            if ( separator != str_view::npos ) {
                self->response_headers_.insert_or_assign(str(header.substr(0, separator)), str(header.substr(separator)));
            } else {
                self->response_headers_.insert_or_assign(str(header), str());
            }
        }
        return 0;
    }

    size_t curl_http_request::write_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
        auto* self = static_cast<curl_http_request *>(userdata);
        size_t buffer_size = size * nmemb;
        size_t offset = self->response_content_.size();
        if ( buffer_size == 0 )
            return 0;
        if ( self->response_stream_ ) {
            try {
                return self->response_stream_->write(ptr, buffer_size);
            } catch(...) {
                return 0;
            }
        } else {
            try {
                self->response_content_.resize(self->response_content_.size() + buffer_size);
            } catch(...) {
                return 0;
            }
            memcpy(self->response_content_.data() + offset, ptr, buffer_size);
            return buffer_size;
        }
        return 0;
    }

    int curl_http_request::debug_callback(CURL *handle,
                                          curl_infotype type,
                                          char *data,
                                          size_t size,
                                          void *userptr) {
        auto* self = static_cast<curl_http_request *>(userptr);
        E2D_ASSERT(handle == self->curl());
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
        if ( canceled_.load() )
            return true;

        if ( add_to_curlm_ != CURLM_OK )
            return true;

        // TODO: check timeout

        return false;
    }

    //
    // network::internal_state
    //

    network::internal_state::internal_state(
        debug& debug)
    : debug_(debug) {
        curl_global_init(CURL_GLOBAL_ALL);

        curl_ = curl_multi_init();
        curl_multi_setopt(curl_, CURLMOPT_MAX_TOTAL_CONNECTIONS, 1);
        curl_multi_setopt(curl_, CURLMOPT_MAX_HOST_CONNECTIONS, 1);

        curl_shared_ = curl_share_init();
        curl_share_setopt(curl_shared_, CURLSHOPT_SHARE, CURL_LOCK_DATA_COOKIE);
        curl_share_setopt(curl_shared_, CURLSHOPT_SHARE, CURL_LOCK_DATA_DNS);
        curl_share_setopt(curl_shared_, CURLSHOPT_SHARE, CURL_LOCK_DATA_SSL_SESSION);
    }

    network::internal_state::~internal_state() noexcept {
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

    void network::internal_state::enque(curl_http_request_uptr value) noexcept {
        value->enque(curl_);
        requests_.insert_or_assign(value->curl(), std::move(value));
    }

    void network::internal_state::tick() {
        int num_handles = -1;
        curl_multi_perform(curl_, &num_handles);
        if ( num_handles == int(requests_.size()) ) {
            return;
        }
        for (;;) {
            int msgs_in_queue = 0;
            CURLMsg* msg = curl_multi_info_read(curl_, &msgs_in_queue);
            if ( !msg || msg->msg != CURLMSG_DONE ) {
                break;
            }
            auto request = requests_.find(msg->easy_handle);
            if ( request != requests_.end() ) {
                request->second->complete(curl_);
                requests_.erase(request);
            }
        }
    }
}

#endif