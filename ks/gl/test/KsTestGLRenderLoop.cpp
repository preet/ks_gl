/*
   Copyright (C) 2015-2016 Preet Desai (preet.desai@gmail.com)

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

#include <unistd.h>
#include <glm/glm.hpp>
#include <ks/shared/KsCallbackTimer.hpp>
#include <ks/gl/KsGLImplementation.hpp>
#include <ks/gl/KsGLStateSet.hpp>
#include <ks/gl/KsGLCommands.hpp>
#include <ks/gl/test/KsTestGLScene.hpp>

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
                "attribute vec4 a_v4_color;\n"
                "\n"
                "varying lowp vec4 v_v4_color;\n"
                "\n"
                "void main()\n"
                "{\n"
                "   v_v4_color = a_v4_color;\n"
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
                "varying lowp vec4 v_v4_color;\n"
                "\n"
                "void main()\n"
                "{\n"
                "    gl_FragColor = v_v4_color;\n"
                "}\n";

    // ============================================================= //

    // VertexLayout
    using AttrType = gl::VertexBuffer::Attribute::Type;

    struct Vertex {
        glm::vec3 a_v3_position; // 12
        glm::u8vec4 a_v4_color;
    }; // sizeof == 20

    gl::VertexLayout const vx_layout {
        { "a_v4_position", AttrType::Float, 3, false },
        { "a_v4_color", AttrType::UByte, 4, true },
    };

    struct BufferRange {
        uint start_byte;
        uint size_bytes;
    };

    // ============================================================= //

    class Renderable
    {
    public:
        Renderable() :
            m_init(false)
        {}

        ~Renderable()
        {}

        void OnRender()
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

                m_angle_rads = 0.0;

                // done init
                m_init = true;
            }

            // Create vertex buffer
            unique_ptr<std::vector<u8>> list_vx =
                    make_unique<std::vector<u8>>();

            m_angle_rads += (6.28f/300.0f)*0.2;
            float disp = sin(m_angle_rads);
            glm::vec3 disp_v(disp,0,0);

            gl::Buffer::PushElement<Vertex>(
                        *list_vx,
                        Vertex{
                            glm::vec3{-0.4,-0.4,0}+disp_v,
                            glm::u8vec4(0,255,255,255)
                        });

            gl::Buffer::PushElement<Vertex>(
                        *list_vx,
                        Vertex{
                            glm::vec3{0.4,-0.4,0}+disp_v,
                            glm::u8vec4(255,0,255,255)
                        });

            gl::Buffer::PushElement<Vertex>(
                        *list_vx,
                        Vertex{
                            glm::vec3{0,0.6,0}+disp_v,
                            glm::u8vec4(255,255,0,255)
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
                            list_vx.release()
                        ));

            m_vx_buff->GLInit();
            m_vx_buff->GLBind();
            m_vx_buff->GLSync();
            m_vx_buff->GLUnbind();

            m_range.start_byte = 0;
            m_range.size_bytes = list_vx_sz;

            // ClearScreen
            gl::Clear(gl::ColorBufferBit);

            // Draw!
            m_shader->GLEnable(m_state_set.get());

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

        float m_angle_rads;

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


    // Create the Renderable
    shared_ptr<Renderable> renderable =
            make_shared<Renderable>();

    std::function<void()> render_callback =
            [renderable](){
                renderable->OnRender();
            };

    // Create window
    gui::Window::Attributes win_attribs;
    gui::Window::Properties win_props;
    win_props.width = 640;
    win_props.height = 480;


    // (uncomment one option)

    // Note:
    // Disable Desktop Compositing (for both
    // vsync=on and vsync=off)!
    // KDE: disable compositing with Alt+Shift+F12

//    {
//        // === 1. RenderLoop type: [VSync] === //

//        win_props.swap_interval = 1;
//    }


    {
        // === 2. RenderLoop type: [Timer (VSync off)] === //
        win_props.swap_interval = 0;

        // Note: See KsTestGLScene for frame rate details
        // with vsync off.
    }


    shared_ptr<gui::Window> window =
            app->CreateWindow(
                render_evl,
                win_attribs,
                win_props);

    shared_ptr<Scene> scene =
            ks::make_object<Scene>(
                app,
                window,
                render_callback);

    // Run!
    app->Run();

    // Stop threads
    EventLoop::RemoveFromThread(render_evl,render_thread,true);

    return 0;
}

