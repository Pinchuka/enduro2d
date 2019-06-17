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
        resp2.handler()
            .then([] (auto resp){
                REQUIRE(resp.ready());
                REQUIRE(resp.status_code() == http_code::ok);
            })
            .except([](auto e){
                FAIL("http request was canceled");
            })
            .wait();
    }
    {
        the<network>().send(http_request("google.com", http_request::method::get).redirections(0)).handler()
            .then([] (auto resp){
                REQUIRE(resp.ready());
                REQUIRE(resp.status_code() == http_code::moved_permanently);
                str_view s = resp.content_as_str();
                REQUIRE(s == "<HTML><HEAD><meta http-equiv=\"content-type\" content=\"text/html;charset=utf-8\">\n<TITLE>301 Moved</TITLE></HEAD><BODY>\n<H1>301 Moved</H1>\nThe document has moved\n<A HREF=\"http://www.google.com/\">here</A>.\r\n</BODY></HTML>\r\n");
            })
            .except([](auto e){
                FAIL("http request was canceled");
            })
            .wait();
    }
    {
        const str file_path = "network_output_file";
        auto dst = make_write_file(file_path, false);
        REQUIRE(dst);

        the<network>().send(http_request("google.com", http_request::method::get)
            .redirections(0)
            .output_stream(std::move(dst))).handler()
            .then([file_path] (auto resp){
                REQUIRE(resp.ready());
                REQUIRE(resp.status_code() == http_code::moved_permanently);
                REQUIRE(resp.content().empty());

                str content;
                REQUIRE(filesystem::try_read_all(content, file_path));
                REQUIRE(content == "<HTML><HEAD><meta http-equiv=\"content-type\" content=\"text/html;charset=utf-8\">\n<TITLE>301 Moved</TITLE></HEAD><BODY>\n<H1>301 Moved</H1>\nThe document has moved\n<A HREF=\"http://www.google.com/\">here</A>.\r\n</BODY></HTML>\r\n");
            })
            .except([](auto e){
                FAIL("http request was canceled");
            })
            .wait();
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

        buffer image_data(src->length());
        REQUIRE(src->read(image_data.data(), image_data.size()) == image_data.size());

        resp.wait();
        REQUIRE(buffer_view(resp.content()) == image_data);
    }
    {
        auto resp1 = the<network>().send(http_request("ydxcftgyuhjhga.ru", http_request::method::get)
                .timeout(15.f)
                .header("name", "value"));
        resp1.wait();
        REQUIRE(resp1.ready());
        REQUIRE(resp1.status_code() == http_code::connection_error);
    }

#if 0
/*
    Server API:
    - if header "test-number: [N]" return header "test-[N]: passed", return content "test-[N] - passed"
    - if header "delay: [T]" wait [T] seconds and then return any data
    - if header "redirect: [N]" redirect [N] times
*/
    str dbg_server = "ghjokjnhbgvbhnjk";    // TODO
    SECTION("get"){
        the<network>().send(http_request(dbg_server, http_request::method::get)
            .header("test-number", "101")).handler()
            .then([](http_response resp) {
                REQUIRE(resp.ready());
                REQUIRE(resp.status_code() == http_code::ok);
                REQUIRE(resp.content_as_str() == "test-101 - passed");
            })
            .except([](auto e) {
                FAIL("http request was canceled");
            })
            .wait();
        the<network>().send(http_request(dbg_server, http_request::method::get)
            .header("test-number", "102")).handler()
            .then([](http_response resp) {
                REQUIRE(resp.ready());
                REQUIRE(resp.status_code() == http_code::ok);
                REQUIRE(resp.content_as_str() == "test-102 - passed");
                auto iter = resp.headers().find("test-102");
                REQUIRE(iter != resp.headers().end());
                REQUIRE(iter->second == "passed");
            })
            .except([](auto e) {
                FAIL("http request was canceled");
            })
            .wait();
        the<network>().send(http_request(dbg_server, http_request::method::get)
            .header("test-number", "103")
            .header("delay", "20")
            .timeout(15.0f)).handler()
            .then([](http_response resp) {
                REQUIRE(resp.ready());
                REQUIRE(resp.status_code() != http_code::request_timeout);
                REQUIRE(resp.headers().empty());
                REQUIRE(resp.content().empty());
            })
            .except([](auto e) {
                FAIL("http request was canceled");
            })
            .wait();
        the<network>().send(http_request(dbg_server, http_request::method::get)
            .header("test-number", "104")
            .header("redirect", "2")).handler()
            .then([](http_response resp) {
                REQUIRE(resp.ready());
                REQUIRE(resp.status_code() == http_code::ok);
                REQUIRE(resp.content_as_str() == "test-104 - passed");
            })
            .except([](auto e) {
                FAIL("http request was canceled");
            })
            .wait();
        the<network>().send(http_request(dbg_server, http_request::method::get)
            .redirections(0)
            .header("test-number", "105")
            .header("redirect", "2")).handler()
            .then([](http_response resp) {
                REQUIRE(resp.ready());
                REQUIRE(resp.status_code() == http_code::moved_permanently);
            })
            .except([](auto e) {
                FAIL("http request was canceled");
            })
            .wait();
        the<network>().send(http_request(dbg_server, http_request::method::get)
            .redirections(1)
            .header("test-number", "106")
            .header("redirect", "2")).handler()
            .then([](http_response resp) {
                REQUIRE(resp.ready());
                REQUIRE(resp.status_code() == http_code::moved_permanently);
            })
            .except([](auto e) {
                FAIL("http request was canceled");
            })
            .wait();
        the<network>().send(http_request(dbg_server, http_request::method::get)
            .redirections(2)
            .header("test-number", "107")
            .header("redirect", "2")).handler()
            .then([](http_response resp) {
                REQUIRE(resp.ready());
                REQUIRE(resp.status_code() == http_code::ok);
                REQUIRE(resp.content_as_str() == "test-107 - passed");
            })
            .except([](auto e) {
                FAIL("http request was canceled");
            })
            .wait();
        {
            auto resp1 = the<network>().send(http_request(dbg_server, http_request::method::get)
                .header("test-number", "108")
                .header("delay", "30"));
            resp1.handler()
                .then([](http_response resp) {
                    FAIL("http request must be canceled");
                })
                .except([](std::exception_ptr e) {
                    // OK
                });
            resp1.cancel();
            resp1.wait();
        }
        {
            const str file_path = "network_output_file";
            auto dst = make_write_file(file_path, false);

            the<network>().send(http_request(dbg_server, http_request::method::get)
                .header("test-number", "109")
                .output_stream(std::move(dst))).handler()
            .then([file_path](http_response resp) {
                REQUIRE(resp.ready());
                REQUIRE(resp.status_code() == http_code::ok);
                REQUIRE(resp.content().empty());

                str content;
                REQUIRE(filesystem::try_read_all(content, file_path));
                REQUIRE(content == "test-109 - passed");
            })
            .except([](auto e) {
                FAIL("http request was canceled");
            })
            .wait();
        }
    }
    SECTION("post"){
        {
            str resources;
            REQUIRE(filesystem::extract_predef_path(
                resources,
                filesystem::predef_path::resources));
            auto src = make_read_file(path::combine(resources, "bin/gnome/gnome.png"));
            REQUIRE(src);

            buffer image_data(src->length());
            REQUIRE(src->read(image_data.data(), image_data.size()) == image_data.size());

            the<network>().send(http_request(dbg_server, http_request::method::post)
                .header("test-content", "gnome")
                .content(image_data)).handler()
                .then([] (auto resp){
                    REQUIRE(resp.ready());
                    REQUIRE(resp.status_code() == http_code::ok);
                })
                .except([](auto e){
                    FAIL("http request was canceled");
                })
                .wait();
        }
        {
            str resources;
            REQUIRE(filesystem::extract_predef_path(
                resources,
                filesystem::predef_path::resources));
            auto src = make_read_file(path::combine(resources, "bin/gnome/gnome.png"));
            REQUIRE(src);

            the<network>().send(http_request(dbg_server, http_request::method::post)
                .header("test-content", "gnome")
                .content(std::move(src))).handler()
                .then([] (auto resp){
                    REQUIRE(resp.ready());
                    REQUIRE(resp.status_code() == http_code::ok);
                })
                .except([](auto e){
                    FAIL("http request was canceled");
                })
                .wait();
        }
    }
    SECTION("header"){
        the<network>().send(http_request(dbg_server, http_request::method::head)
            .header("test-number", "301")).handler()
            .then([](http_response resp) {
                REQUIRE(resp.ready());
                REQUIRE(resp.status_code() == http_code::ok);
                REQUIRE(resp.content().empty());
                auto iter = resp.headers().find("test-301");
                REQUIRE(iter != resp.headers().end());
                REQUIRE(iter->second == "passed");
            })
            .except([](auto e) {
                FAIL("http request was canceled");
            })
            .wait();
    }
    // TODO
    // - SSL
#endif
    modules::shutdown<network>();
    modules::shutdown<debug>();
}
