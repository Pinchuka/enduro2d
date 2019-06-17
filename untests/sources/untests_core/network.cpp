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
        auto req1 = the<network>().send(http_request("ya.ru", http_request::method::get)
                .timeout(15.f)
                .header("name", "value"));
        req1.then([](const http_response& resp){
            REQUIRE(resp.status_code() == 200);
        });

        req1.wait();
    }
    modules::shutdown<network>();
    modules::shutdown<debug>();

    /*
    auto req = request("ya.ru")
        .timeout(15.f)
        .header("name", "value");

    the<network>()
        .send(req)
        .then([](const response& resp){
            resp.status();
        });*/
}
