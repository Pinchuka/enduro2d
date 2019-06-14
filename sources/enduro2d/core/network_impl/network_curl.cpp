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
    // network
    //

    network::network(debug& d)
    : state_(std::make_unique<internal_state>(d)) {
    }

    network::~network() noexcept {
    }

    stdex::promise<http_response> network::send(http_request&& req) {
        stdex::promise<http_response> result;
        state_->enque(std::make_unique<curl_http_request>(
                state_->dbg(),
                req.url(),
                req.headers(),
                req.type(),
                req.timeout(),
                std::move(req.content_),
                std::move(req.output_stream_),
                result
            ));
        return result;
    }

    stdex::promise<http_response> network::send(http_request& req) {
        http_request::content_t tmp_content;
        output_stream_uptr tmp_stream;
        std::swap(req.content_, tmp_content);
        std::swap(req.output_stream_, tmp_stream);
        stdex::promise<http_response> result;
        state_->enque(std::make_unique<curl_http_request>(
                state_->dbg(),
                req.url(),
                req.headers(),
                req.type(),
                req.timeout(),
                std::move(tmp_content),
                std::move(tmp_stream),
                result
            ));
        return result;
    }

}

#endif
