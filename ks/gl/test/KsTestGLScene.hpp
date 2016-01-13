/*
   Copyright (C) 2016 Preet Desai (preet.desai@gmail.com)

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

#include <ks/gui/KsGuiWindow.hpp>
#include <ks/gui/KsGuiApplication.hpp>
#include <ks/platform/KsPlatformMain.hpp>

namespace ks
{
    namespace
    {
        class Scene : public ks::Object
        {
        public:
            using base_type = ks::Object;

            Scene(ks::Object::Key const &key,
                  shared_ptr<gui::Application> app,
                  shared_ptr<gui::Window> win,
                  std::function<void()> render_callback) :
                ks::Object(key,app->GetEventLoop()),
                m_app(app),
                m_win(win),
                m_vsync(m_win->swap_interval.Get() > 0),
                m_render_callback(std::move(render_callback))
            {
                if(!m_vsync)
                {                   
                    // NOTE:

                    // Using a timer like this is probably not a good
                    // idea in general. Rather, iterations should be
                    // scheduled on the fly against a given update_dt
                    // by measuring how much time has passed first.

                    // Some games seem to allow 'unlocked' FPS which
                    // just renders as fast as possible or a target FPS
                    // which tries to actively throttle.

                    // For now though this serves as an okay test of
                    // rendering with vsync off.

                    m_timer =
                            make_object<CallbackTimer>(
                                m_app->GetEventLoop(),
                                Milliseconds(4),
                                [this](){ OnTimeout(); });
                }
            }

            void Init(ks::Object::Key const &,
                      shared_ptr<Scene> const &this_scene)
            {
                // Application ---> Scene
                m_app->signal_init.Connect(
                            this_scene,
                            &Scene::OnAppInit);

                m_app->signal_pause.Connect(
                            this_scene,
                            &Scene::OnAppPause,
                            ks::ConnectionType::Direct);

                m_app->signal_resume.Connect(
                            this_scene,
                            &Scene::OnAppResume,
                            ks::ConnectionType::Direct);

                m_app->signal_quit.Connect(
                            this_scene,
                            &Scene::OnAppQuit,
                            ks::ConnectionType::Direct);

                if(m_vsync)
                {
                    m_app->signal_processed_events->Connect(
                                this_scene,
                                &Scene::OnAppProcEvents);
                }

                // Scene ---> Application
                auto win_ptr = m_win.get();
                m_render_task_fn =
                        [=](){
                            win_ptr->InvokeWithContext(m_render_callback);
                            win_ptr->SwapBuffers();
                        };

                signal_app_process_events.Connect(
                            m_app,
                            &gui::Application::ProcessEvents);
            }

            ~Scene()
            {}

            void OnAppInit()
            {
                m_running = true;
                signal_app_process_events.Emit();

                if(!m_vsync)
                {
                    m_timer->Start();
                }
            }

            void OnAppPause()
            {
                m_running = false;

                if(!m_vsync)
                {
                    m_timer->Stop();
                }
            }

            void OnAppResume()
            {
                m_running = true;
                signal_app_process_events.Emit();

                if(!m_vsync)
                {
                    m_timer->Start();
                }
            }

            void OnAppQuit()
            {
                m_running = false;

                if(!m_vsync)
                {
                    m_timer->Stop();
                }
            }

            void OnAppProcEvents(bool)
            {
                if(m_running)
                {
                    auto render_task = make_shared<ks::Task>(m_render_task_fn);
                    m_win->GetEventLoop()->PostTask(render_task);

                    render_task->Wait();

                    // We're assuming vsync is true and will introduce
                    // a delay so we schedule the next ProcessEvents task
                    // immediately
                    signal_app_process_events.Emit();
                }
            }

            void OnTimeout()
            {
                m_app->ProcessEvents();

                auto render_task = make_shared<ks::Task>(m_render_task_fn);
                m_win->GetEventLoop()->PostTask(render_task);
                render_task->Wait();
            }

        private:
            std::atomic<bool> m_running;
            shared_ptr<gui::Application> m_app;
            shared_ptr<gui::Window> m_win;

            bool m_vsync;
            shared_ptr<CallbackTimer> m_timer;
            Signal<> signal_app_process_events;

            std::function<void()> m_render_task_fn;
            std::function<void()> m_render_callback;
        };
    }
}
