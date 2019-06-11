/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "_core.hpp"

namespace e2d
{
    //
    // request
    //

    class request {
    public:
        enum class method : u8 {
            get,
            post,
            head
        };
    public:
        request(str_view);
        request& timeout(f32);
        request& header(str_view, str_view);
    };

    //
    // response
    //

    class response {
    public:
        u16 status_code() const;
    };

    //
    // network
    //

    class network final : public module<network> {
    public:
        network(debug& d);
        ~network() noexcept;

        stdex::promise<response> send(request);
    private:
        class internal_state;
        std::unique_ptr<internal_state> state_;
    };
}
