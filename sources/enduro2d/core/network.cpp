/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include <enduro2d/core/network.hpp>

namespace
{
    using namespace e2d;
}

namespace e2d
{
    class network::internal_state final : private e2d::noncopyable {
    public:
    };

    network::network(debug& d)
    : state_(new internal_state()){}

    network::~network() noexcept = default;

    stdex::promise<response> send(request req) {
        return stdex::make_resolved_promise(response());
    }
}
