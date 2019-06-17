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
        REQUIRE(resp1.status_code() == http_code::ok);

        auto resp2 = the<network>().send(http_request("ya.ru", http_request::method::get)
                .timeout(15.f)
                .header("name", "value"));
        resp2.handler().then([] (auto resp){
            REQUIRE(resp.ready());
            REQUIRE(resp.status_code() == http_code::ok);
        });
    }
    {
        auto resp = the<network>().send(http_request(http_request::method::get)
                .url("https://media.githubusercontent.com/media/enduro2d/enduro2d/master/untests/bin/gnome/gnome.png"));

        str resources;
        REQUIRE(filesystem::extract_predef_path(
            resources,
            filesystem::predef_path::resources));
        auto src = make_read_file(path::combine(resources, "bin/gnome/gnome.png"));
        REQUIRE(src);

        buffer image_data;
        if ( src ) {
            image_data = buffer(src->length());
            REQUIRE(src->read(image_data.data(), image_data.size()) == image_data.size());
        }

        resp.wait();
        REQUIRE(buffer_view(resp.content()) == image_data);
    }
    modules::shutdown<network>();
    modules::shutdown<debug>();
}
