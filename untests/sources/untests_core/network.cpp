/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include "_core.hpp"
using namespace e2d;

TEST_CASE("network"){
    modules::initialize<debug>();
    modules::initialize<network>(the<debug>());
    {
        auto resp1 = the<network>().send(http_request("ya.ru", http_request::method::get)
                .timeout(15.f)
                .header("name", "value"));

        resp1.wait();
        REQUIRE(resp1.ready());
        REQUIRE(resp1.status_code() == http_code::OK);
    }
    modules::shutdown<network>();
    modules::shutdown<debug>();
}
