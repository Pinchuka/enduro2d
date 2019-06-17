/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include "network.hpp"

#if defined(E2D_NETWORK_MODE) && E2D_NETWORK_MODE == E2D_NETWORK_MODE_NONE

namespace e2d
{
    //
    // network::internal_state
    //

    class network::internal_state final : private e2d::noncopyable {
    public:
        internal_state() = default;
        ~internal_state() = default;
    };

    //
    // network
    //

    network::network(debug&)
    : state_(std::make_unique<internal_state>()) {
    }

    network::~network() noexcept {
    }

    http_response network::send(http_request&&) {
        return http_response(std::make_shared<http_response::internal_state>());
    }

    http_response network::send(http_request&) {
        return http_response(std::make_shared<http_response::internal_state>());
    }

}

#endif
