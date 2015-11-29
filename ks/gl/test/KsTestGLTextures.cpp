/*
   Copyright (C) 2015 Preet Desai (preet.desai@gmail.com)

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include <cassert>
#include <glm/glm.hpp>
#include <ks/shared/KsImage.hpp>
#include <ks/gl/KsGLImplementation.hpp>
#include <ks/gl/KsGLStateSet.hpp>
#include <ks/gl/KsGLCommands.hpp>
#include <ks/gl/KsGLTexture2D.hpp>
#include <ks/gl/KsGLUniform.hpp>
#include <ks/gui/KsGuiWindow.hpp>
#include <ks/gui/KsGuiApplication.hpp>
#include <ks/platform/KsPlatformMain.hpp>

// NOTE: "SOA" stands for structure of arrays, and "AOS" stands for
// arrays of structures, referring to vertex attribute layout
//
// This test renders geometry in four different configurations:
// * AOS non-indexed
// * AOS indexed
// * SOA non-indexed
// * SOA indexed

using namespace ks;

namespace {

    // ============================================================= //

    // Shader
    std::string const vertex_shader =
                "#ifdef GL_ES\n"
                "    //\n"
                "#else\n"
                "    #define lowp\n"
                "    #define mediump\n"
                "    #define highp\n"
                "#endif\n"
                "\n"
                "attribute vec4 a_v4_position;\n"
                "attribute vec2 a_v2_tex0;\n"
                "\n"
                "varying lowp vec2 v_v2_tex0;\n"
                "\n"
                "void main()\n"
                "{\n"
                "   v_v2_tex0 = a_v2_tex0;\n"
                "   gl_Position = a_v4_position;\n"
                "}\n";

    std::string const frag_shader =
                "#ifdef GL_ES\n"
                "    precision mediump float;\n"
                "#else\n"
                "    #define lowp\n"
                "    #define mediump\n"
                "    #define highp\n"
                "#endif\n"
                "\n"
                "varying lowp vec2 v_v2_tex0;\n"
                "uniform lowp sampler2D u_s_tex0;\n"
                "\n"
                "void main()\n"
                "{\n"
                "    gl_FragColor = texture2D(u_s_tex0,v_v2_tex0);\n"
                "}\n";

    // ============================================================= //

    // VertexLayout
    using AttrType = gl::VertexBuffer::Attribute::Type;

    struct Vertex {
        glm::vec3 a_v3_position; // 12
        glm::vec2 a_v2_tex0; // 8
    }; // sizeof == 20

    gl::VertexLayout const vx_layout {
        { "a_v4_position", AttrType::Float, 3, false },
        { "a_v2_tex0", AttrType::Float, 2, false }
    };

    struct BufferRange {
        uint start_byte;
        uint size_bytes;
    };

    // ============================================================= //

    class RenderLayer
    {
    public:
        RenderLayer() :
            m_init(false)
        {}

        ~RenderLayer()
        {}

        void Sync()
        {}

        void Render()
        {
            if(!m_init)
            {
                // setup
                ks::gl::Implementation::GLCapture();
                m_state_set = make_unique<gl::StateSet>();
                m_state_set->CaptureState();
                m_state_set->SetClearColor(0.15,0.15,0.15,1.0);
                m_state_set->SetDepthTest(GL_FALSE);

                // Create shader
                m_shader =
                        make_unique<gl::ShaderProgram>(
                            vertex_shader,
                            frag_shader);

                m_shader->GLInit();

                // Create vertex buffer
                unique_ptr<std::vector<u8>> list_vx =
                        make_unique<std::vector<u8>>();

                gl::Buffer::PushElement<Vertex>(
                            *list_vx,
                            Vertex{
                                glm::vec3{-0.8,-0.8,0},
                                glm::vec2{0,1}
                            });

                gl::Buffer::PushElement<Vertex>(
                            *list_vx,
                            Vertex{
                                glm::vec3{0.8,0.8,0},
                                glm::vec2{1,0}
                            });

                gl::Buffer::PushElement<Vertex>(
                            *list_vx,
                            Vertex{
                                glm::vec3{-0.8,0.8,0},
                                glm::vec2{0,0}
                            });

                gl::Buffer::PushElement<Vertex>(
                            *list_vx,
                            Vertex{
                                glm::vec3{-0.8,-0.8,0},
                                glm::vec2{0,1}
                            });

                gl::Buffer::PushElement<Vertex>(
                            *list_vx,
                            Vertex{
                                glm::vec3{0.8,-0.8,0},
                                glm::vec2{1,1}
                            });

                gl::Buffer::PushElement<Vertex>(
                            *list_vx,
                            Vertex{
                                glm::vec3{0.8,0.8,0},
                                glm::vec2{1,0}
                            });

                m_vx_buff =
                        make_unique<gl::VertexBuffer>(
                            vx_layout,
                            gl::Buffer::Usage::Static);

                auto list_vx_sz = list_vx->size();

                m_vx_buff->UpdateBuffer(
                            make_unique<gl::Buffer::UpdateFreeData>(
                                gl::Buffer::Update::ReUpload,
                                0,0,
                                list_vx_sz,
                                list_vx.release()));

                m_vx_buff->GLInit();
                m_vx_buff->GLBind();
                m_vx_buff->GLSync();
                m_vx_buff->GLUnbind();

                m_range.start_byte = 0;
                m_range.size_bytes = list_vx_sz;

                // Create the texture
                Image<RGBA8> image(16,16);
                auto& list_pixels = image.GetData();
                for(uint i=0; i < 256; i++) {

                    u8 r,g,b,a;
                    r = 0;
                    g = 0;
                    b = 0;
                    a = 255;

                    if(i < 64) {
                        r = 255;
                    }
                    else if(i < 128) {
                        g = 255;
                    }
                    else if(i < 192) {
                        b = 255;
                    }
                    else {
                        r = 255;
                        g = 255;
                    }

                    list_pixels.push_back(RGBA8{r,g,b,a});
                }

                m_texture =
                        make_unique<gl::Texture2D>(
                            gl::Texture2D::Format::RGBA8);

                m_texture->SetFilterModes(
                            gl::Texture2D::Filter::Nearest,
                            gl::Texture2D::Filter::Nearest);

                m_texture->SetWrapModes(
                            gl::Texture2D::Wrap::ClampToEdge,
                            gl::Texture2D::Wrap::ClampToEdge);

                shared_ptr<ImageData> sptr_image_data(
                            image.ConvertToImageDataPtr().release());

                m_texture->UpdateTexture(
                            gl::Texture2D::Update{
                                gl::Texture2D::Update::ReUpload,
                                glm::u16vec2(0,0),
                                sptr_image_data
                            });

                m_texture->GLInit();
                m_texture->GLBind(m_state_set.get(),0);
                m_texture->GLSync();

                // done init
                m_init = true;
            }

            // ClearScreen
            gl::Clear(gl::ColorBufferBit);

            // Draw!
            m_shader->GLEnable(m_state_set.get());

            m_texture->GLBind(m_state_set.get(),0);

            gl::Uniform<GLint> u_sampler("u_s_tex0",0);
            u_sampler.GLSetUniform(m_shader.get());

            gl::DrawArrays(gl::Primitive::Triangles,
                           m_shader.get(),
                           m_vx_buff.get(),
                           m_range.start_byte,
                           m_range.size_bytes);
        }

    private:
        bool m_init;
        unique_ptr<gl::StateSet> m_state_set;
        unique_ptr<gl::ShaderProgram> m_shader;
        unique_ptr<gl::VertexBuffer> m_vx_buff;
        unique_ptr<gl::Texture2D> m_texture;

        BufferRange m_range;
    };

    // ============================================================= //
}


int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    // Create application
    shared_ptr<gui::Application> app =
            make_object<gui::Application>();

    // Create the render thread
    shared_ptr<EventLoop> render_evl =
            make_shared<EventLoop>();

    std::thread render_thread =
            EventLoop::LaunchInThread(render_evl);


    // Create window
    gui::Window::Attributes win_attribs;
    gui::Window::Properties win_props;

    shared_ptr<gui::Window> window =
            app->CreateWindow(
                render_evl,
                win_attribs,
                win_props);

    shared_ptr<RenderLayer> render_layer =
            make_shared<RenderLayer>();

    // Create render timer
    auto render_callback =
            std::bind(&RenderLayer::Render,
                      render_layer.get());

    shared_ptr<CallbackTimer> win_timer =
            make_object<CallbackTimer>(
                render_evl,
                milliseconds(15),
                [window,render_callback](){
                    window->InvokeWithContext(render_callback);
                    window->SwapBuffers();
                });

    win_timer->Start();

    // Run!
    app->Run();

    // Stop threads
    EventLoop::RemoveFromThread(render_evl,render_thread,true);

    return 0;
}

