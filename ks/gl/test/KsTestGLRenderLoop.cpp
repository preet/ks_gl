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
#include <ks/shared/KsCallbackTimer.hpp>
#include <ks/gl/KsGLImplementation.hpp>
#include <ks/gl/KsGLStateSet.hpp>
#include <ks/gl/KsGLCommands.hpp>
#include <ks/gui/KsGuiWindow.hpp>
#include <ks/gui/KsGuiApplication.hpp>
#include <ks/platform/KsPlatformMain.hpp>

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

    class Updater
    {
    public:
        Updater(shared_ptr<gui::Window> window,
                shared_ptr<EventLoop> update_evl,
                Milliseconds timer_ms) :
            m_window(window),
            m_upd_dt_ms(timer_ms),
            m_init(false)
        {
            m_timer =
                    make_object<CallbackTimer>(
                        update_evl,
                        timer_ms,
                        [this](){ this->OnUpdate(); });

            m_timer->SetRepeating(false);
            m_timer->Start();
        }

        ~Updater()
        {}

        void OnUpdate()
        {
            auto curr_upd_time =
                    std::chrono::high_resolution_clock::now();

            auto upd_start_time = curr_upd_time;

            // pretend to do some work
//            std::this_thread::sleep_for(Milliseconds(5));

            requestSyncAndRender();

            auto upd_end_time = std::chrono::high_resolution_clock::now();

            double upd_elapsed_ms =
                    std::chrono::duration_cast<
                        std::chrono::microseconds>(
                            upd_end_time-upd_start_time).count()/1000.0;

            double upd_sched_delay =
                    double(m_upd_dt_ms.count()) - upd_elapsed_ms;

            if(upd_sched_delay > 0)
            {
                // Schedule delayed
                Milliseconds delay_ms(
                            static_cast<u64>(ceil(upd_sched_delay)));

                m_timer->SetInterval(delay_ms);
                m_timer->Start();
            }
            else
            {
                // Schedule immediately
                auto event =
                        make_unique<SlotEvent>(
                            std::bind(&Updater::OnUpdate,this));

                m_timer->GetEventLoop()->PostEvent(std::move(event));
            }
        }

        void requestSyncAndRender()
        {
            auto sync_callback = std::bind(&Updater::onSync,this);
            auto render_callback = std::bind(&Updater::onRender,this);

            ks::gui::Window* window_ptr = m_window.get();

            auto sync_task =
                    make_shared<ks::Task>(
                        [=](){
                            window_ptr->InvokeWithContext(sync_callback);
                        });

            m_window->GetEventLoop()->PostTask(sync_task);
            sync_task->Wait();

            auto render_task =
                    make_shared<ks::Task>(
                        [=](){
                            window_ptr->InvokeWithContext(render_callback);
                            window_ptr->SwapBuffers();
                        });

            m_window->GetEventLoop()->PostTask(render_task);
        }

        void onSync()
        {}

        void onRender()
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

            m_angle_rads += 6.28/300.0;
            float disp = sin(m_angle_rads);
            glm::vec3 disp_v(disp,0,0);

            gl::Buffer::PushElement<Vertex>(
                        *list_vx,
                        Vertex{
                            glm::vec3{-0.433,-0.433,0}+disp_v,
                            glm::u8vec4(0,255,255,255)
                        });

            gl::Buffer::PushElement<Vertex>(
                        *list_vx,
                        Vertex{
                            glm::vec3{0.433,-0.433,0}+disp_v,
                            glm::u8vec4(255,0,255,255)
                        });

            gl::Buffer::PushElement<Vertex>(
                        *list_vx,
                        Vertex{
                            glm::vec3{0,0.5,0}+disp_v,
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
        shared_ptr<gui::Window> m_window;
        Milliseconds m_upd_dt_ms;
        std::chrono::high_resolution_clock::time_point m_prev_upd_time;
        shared_ptr<CallbackTimer> m_timer;

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


    // Create window
    gui::Window::Attributes win_attribs;
    gui::Window::Properties win_props;


    // Note:
    // Disable Desktop Compositing!
    // KDE: Alt+Shift+F12

    // Possible rendering setups:
    //
    // 1. No VSync, Single Threaded
    //    Seems to look better/have less jitter with
    //    a faster update rate (8ms seemed okay)

//    win_props.swap_interval = 0;

//    shared_ptr<gui::Window> window =
//            app->CreateWindow(
//                render_evl,
//                win_attribs,
//                win_props);

//    shared_ptr<Updater> updater =
//            make_shared<Updater>(
//                window,
//                app->GetEventLoop(),
//                Milliseconds(8));



    // 2. No VSync, Multi Threaded
    //    Similar to Single Threaded but looks a bit
    //    worse with more jitter, I'm guessing the
    //    thread blocking because of Sync between the
    //    update and render threads causes this

//    win_props.swap_interval = 0;

//    shared_ptr<gui::Window> window =
//            app->CreateWindow(
//                render_evl,
//                win_attribs,
//                win_props);

//    shared_ptr<Updater> updater =
//            make_shared<Updater>(
//                window,
//                render_evl,
//                Milliseconds(8));



    // 3. VSync, Single Threaded
    //    Smooth, keep the update rate just under the
    //    refresh rate (14-15ms for 60hz is good)
    //    Con: Can Block the update thread for a long
    //    time every frame

//    win_props.swap_interval = 1;

//    shared_ptr<gui::Window> window =
//            app->CreateWindow(
//                render_evl,
//                win_attribs,
//                win_props);

//    shared_ptr<Updater> updater =
//            make_shared<Updater>(
//                window,
//                render_evl,
//                Milliseconds(15));



    // 4. VSync, Multi Threaded
    //    Same as (3). Will block the update thread
    //    substantially (because we still need to sync)
    //    but rendering will happen in a separate thread
    //    which might be useful when there is a lot to
    //    render or a lot to update

    win_props.swap_interval = 1;

    shared_ptr<gui::Window> window =
            app->CreateWindow(
                render_evl,
                win_attribs,
                win_props);

    shared_ptr<Updater> updater =
            make_shared<Updater>(
                window,
                app->GetEventLoop(),
                Milliseconds(15));


    (void)updater;

    // Run!
    app->Run();

    // Stop threads
    EventLoop::RemoveFromThread(render_evl,render_thread,true);

    return 0;
}

