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

    stdex::promise<http_response> network::send(http_request&&) {
        stdex::promise<http_response> result;
        return result;
    }

    stdex::promise<http_response> network::send(http_request&) {
        stdex::promise<http_response> result;
        return result;
    }

}

#endif
