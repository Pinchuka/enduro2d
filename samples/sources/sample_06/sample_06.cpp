/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include "../common.hpp"
using namespace e2d;

namespace
{
    const char* vs_source_cstr = R"glsl(
        #version 120

        attribute vec3 a_position;
        attribute vec2 a_uv;

        uniform float u_time;
        uniform mat4 u_MVP;

        varying vec2 v_uv;

        void main(){
          v_uv = a_uv;
          float s = 0.7 + 0.3 * (cos(u_time * 3.0) + 1.0);
          gl_Position = vec4(a_position * s, 1.0) * u_MVP;
        }
    )glsl";

    const char* fs_source_cstr = R"glsl(
        #version 120

        uniform float u_time;
        uniform sampler2D u_texture1;
        varying vec2 v_uv;

        void main(){
          vec2 uv = vec2(v_uv.s, 1.0 - v_uv.t);
          gl_FragColor = texture2D(u_texture1, uv);
        }
    )glsl";

    struct vertex1 {
        v3f position;
        v2hu uv;
        static vertex_declaration decl() noexcept {
            return vertex_declaration()
                .add_attribute<v3f>("a_position")
                .add_attribute<v2hu>("a_uv");
        }
    };

    std::array<u8,6> generate_quad_indices() noexcept {
        return {0, 1, 2, 2, 1, 3};
    }

    std::array<vertex1,4> generate_quad_vertices(const v2u& size) noexcept {
        f32 hw = size.x * 0.5f;
        f32 hh = size.y * 0.5f;
        return {
            vertex1{{-hw,  hh, 0.f}, {0, 1}},
            vertex1{{-hw, -hh, 0.f}, {0, 0}},
            vertex1{{ hw,  hh, 0.f}, {1, 1}},
            vertex1{{ hw, -hh, 0.f}, {1, 0}}};
    }

    class game final : public engine::application {
    public:
        bool initialize() final {
            the<vfs>().register_scheme<archive_file_source>(
                "piratepack",
                the<vfs>().read(url("resources://bin/kenney_piratepack.zip")));

            the<vfs>().register_scheme_alias(
                "ships",
                url("piratepack://PNG/Retina/Ships"));

            the<network>().send(http_request(http_request::method::get)
                .url("https://media.githubusercontent.com/media/enduro2d/enduro2d/master/samples/bin/library/cube_0.png"))
                .handler().then([this](http_response resp) {
                    the<deferrer>().do_in_main_thread([this, resp]() {
                        texture1_ = the<render>().create_texture(
                            make_memory_stream(buffer(resp.content().data(), resp.content().size())));
                        material_.pass(0).properties()
                            .sampler("u_texture1", render::sampler_state()
                                .texture(texture1_)
                                .min_filter(render::sampler_min_filter::linear)
                                .mag_filter(render::sampler_mag_filter::linear));
                    });
                });

            shader_ = the<render>().create_shader(
                vs_source_cstr, fs_source_cstr);
            texture1_ = the<render>().create_texture(
                the<vfs>().read(url("ships://ship (2).png")));

            if ( !shader_ || !texture1_ ) {
                return false;
            }

            const auto indices = generate_quad_indices();
            index_buffer_ = the<render>().create_index_buffer(
                indices,
                index_declaration::index_type::unsigned_byte,
                index_buffer::usage::static_draw);

            const auto vertices1 = generate_quad_vertices(texture1_->size());
            vertex_buffer1_ = the<render>().create_vertex_buffer(
                vertices1,
                vertex1::decl(),
                vertex_buffer::usage::static_draw);

            if ( !index_buffer_ || !vertex_buffer1_ ) {
                return false;
            }

            material_ = render::material()
                .add_pass(render::pass_state()
                    .states(render::state_block()
                        .capabilities(render::capabilities_state()
                            .blending(true))
                        .blending(render::blending_state()
                            .src_factor(render::blending_factor::src_alpha)
                            .dst_factor(render::blending_factor::one_minus_src_alpha)))
                    .shader(shader_)
                    .properties(render::property_block()
                        .sampler("u_texture1", render::sampler_state()
                            .texture(texture1_)
                            .min_filter(render::sampler_min_filter::linear)
                            .mag_filter(render::sampler_mag_filter::linear))));

            geometry_ = render::geometry()
                .indices(index_buffer_)
                .add_vertices(vertex_buffer1_);

            return true;
        }

        bool frame_tick() final {
            const keyboard& k = the<input>().keyboard();

            if ( the<window>().should_close() || k.is_key_just_released(keyboard_key::escape) ) {
                return false;
            }

            if ( k.is_key_just_pressed(keyboard_key::f12) ) {
                the<dbgui>().toggle_visible(!the<dbgui>().visible());
            }

            if ( k.is_key_pressed(keyboard_key::lsuper) && k.is_key_just_released(keyboard_key::enter) ) {
                the<window>().toggle_fullscreen(!the<window>().fullscreen());
            }

            return true;
        }

        void frame_render() final {
            const auto framebuffer_size = the<window>().real_size().cast_to<f32>();
            const auto projection = math::make_orthogonal_lh_matrix4(
                framebuffer_size, 0.f, 1.f);

            material_.properties()
                .property("u_time", the<engine>().time())
                .property("u_MVP", projection);

            the<render>().execute(render::command_block<64>()
                .add_command(render::viewport_command(
                    the<window>().real_size()))
                .add_command(render::clear_command()
                    .color_value({1.f, 0.4f, 0.f, 1.f}))
                .add_command(render::draw_command(material_, geometry_)));
        }
    private:
        shader_ptr shader_;
        texture_ptr texture1_;
        index_buffer_ptr index_buffer_;
        vertex_buffer_ptr vertex_buffer1_;
        render::material material_;
        render::geometry geometry_;
    };
}

int e2d_main(int argc, char *argv[]) {
    auto params = engine::parameters("sample_06", "enduro2d")
        .timer_params(engine::timer_parameters()
            .maximal_framerate(100));
    modules::initialize<engine>(argc, argv, params).start<game>();
    modules::shutdown<engine>();
    return 0;
}
