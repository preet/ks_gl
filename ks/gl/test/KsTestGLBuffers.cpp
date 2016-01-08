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
#include <ks/gl/KsGLImplementation.hpp>
#include <ks/gl/KsGLStateSet.hpp>
#include <ks/gl/KsGLCommands.hpp>
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
                "attribute vec4 a_v4_color;\n"
                "attribute float a_f_index;\n"
                "\n"
                "varying lowp vec4 v_v4_color;\n"
                "\n"
                "void main()\n"
                "{\n"
                "   vec4 colors[6];\n"
                "   colors[0] = vec4(1.0, 0.0, 0.0, 1.0);\n"
                "   colors[1] = vec4(0.0, 1.0, 0.0, 1.0);\n"
                "   colors[2] = vec4(0.0, 0.0, 1.0, 1.0);\n"
                "   colors[3] = vec4(0.0, 1.0, 1.0, 1.0);\n"
                "   colors[4] = vec4(1.0, 0.0, 1.0, 1.0);\n"
                "   colors[5] = vec4(1.0, 1.0, 0.0, 1.0);\n"
                "   int index = int(a_f_index);\n"
                "   gl_Position = a_v4_position;\n"
                "   v_v4_color = a_v4_color*colors[index];\n"
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

    // AOS
    struct VertexAttrAll {
        glm::vec3 a_v3_position; // 12
        glm::u8vec4 a_v4_color; // 4
        float a_f_index; // 4
    }; // sizeof == 20

    gl::VertexLayout const vx_layout_aos {
        { "a_v4_position", AttrType::Float, 3, false },
        { "a_v4_color", AttrType::UByte, 4, true },
        { "a_f_index", AttrType::Float, 1, false }
    };

    // SOA
    struct VertexAttr0 {
        glm::vec3 a_v3_position;
    };

    gl::VertexLayout const vx_layout_soa0 {
        { "a_v4_position", AttrType::Float, 3, false }
    };

    struct VertexAttr1 {
        glm::u8vec4 a_v4_color;
    };

    gl::VertexLayout const vx_layout_soa1 {
        { "a_v4_color", AttrType::UByte, 4, true }
    };

    struct VertexAttr2 {
        float a_f_index;
    };

    gl::VertexLayout const vx_layout_soa2 {
        { "a_f_index", AttrType::Float, 1, false }
    };

    struct BufferRange {
        uint start_byte;
        uint size_bytes;
    };

    // ============================================================= //

    void CreateTriangleIndices(unique_ptr<std::vector<u8>>& list_ix,
                               std::vector<BufferRange>& list_ix_ranges)
    {
        for(uint row=0; row < 2; row++) {
            for(uint col=0; col < 3; col++) {
                BufferRange ix_range;
                ix_range.start_byte = list_ix->size();

                gl::IndexBuffer::PushElement<u16>(*list_ix,0);
                gl::IndexBuffer::PushElement<u16>(*list_ix,1);
                gl::IndexBuffer::PushElement<u16>(*list_ix,2);

                ix_range.size_bytes = 3*sizeof(u16);
                list_ix_ranges.push_back(ix_range);
            }
        }
    }

    // ============================================================= //

    void CreateTrianglesAOS(unique_ptr<std::vector<u8>>& list_vx,
                            std::vector<BufferRange>& list_vx_ranges)
    {
        float const dx = 0.4;
        float const dy = 0.33;
        glm::vec2 pen{-1.0,1.0};

        for(uint row=0; row < 2; row++)
        {
            for(uint col=0; col < 3; col++)
            {
                BufferRange vx_range;
                vx_range.start_byte = list_vx->size();

                // top left
                gl::VertexBuffer::PushElement(
                            *list_vx,
                            VertexAttrAll{
                                glm::vec3(pen.x,pen.y,1),
                                glm::u8vec4(255,255,255,255),
                                static_cast<float>((row*3)+col)
                            });

                // bottom left
                gl::VertexBuffer::PushElement(
                            *list_vx,
                            VertexAttrAll{
                                glm::vec3(pen.x,pen.y-dy,1),
                                glm::u8vec4(153,153,153,255),
                                static_cast<float>((row*3)+col)
                            });

                // bottom right
                gl::VertexBuffer::PushElement(
                            *list_vx,
                            VertexAttrAll{
                                glm::vec3(pen.x+dx,pen.y-dy,1),
                                glm::u8vec4(85,85,85,255),
                                static_cast<float>((row*3)+col)
                            });

                vx_range.size_bytes = 3*sizeof(VertexAttrAll);
                list_vx_ranges.push_back(vx_range);

                pen.x += dx*2;
            }

            pen.x = -1.0;
            pen.y -= dy*2;
        }
    }

    // ============================================================= //

    void CreateTrianglesSOA(unique_ptr<std::vector<u8>>& list_vx0,
                            unique_ptr<std::vector<u8>>& list_vx1,
                            unique_ptr<std::vector<u8>>& list_vx2,
                            std::vector<BufferRange>& list_vx_ranges0,
                            std::vector<BufferRange>& list_vx_ranges1,
                            std::vector<BufferRange>& list_vx_ranges2)
    {
        float const dx = 0.4;
        float const dy = 0.33;
        glm::vec2 pen{-1.0,0.0};

        for(uint row=0; row < 2; row++)
        {
            for(uint col=0; col < 3; col++)
            {
                BufferRange vx_range0;
                BufferRange vx_range1;
                BufferRange vx_range2;

                vx_range0.start_byte = list_vx0->size();
                vx_range1.start_byte = list_vx1->size();
                vx_range2.start_byte = list_vx2->size();

                // top left
                gl::VertexBuffer::PushElement(
                            *list_vx0,
                            VertexAttr0{glm::vec3(pen.x,pen.y,1)});

                gl::VertexBuffer::PushElement(
                            *list_vx1,
                            VertexAttr1{glm::u8vec4(255,255,255,255)});

                gl::VertexBuffer::PushElement(
                            *list_vx2,
                            VertexAttr2{static_cast<float>((row*3)+col)});

                // bottom left
                gl::VertexBuffer::PushElement(
                            *list_vx0,
                            VertexAttr0{glm::vec3(pen.x,pen.y-dy,1)});

                gl::VertexBuffer::PushElement(
                            *list_vx1,
                            VertexAttr1{glm::u8vec4(150,150,150,255)});

                gl::VertexBuffer::PushElement(
                            *list_vx2,
                            VertexAttr2{static_cast<float>((row*3)+col)});

                // bottom right
                gl::VertexBuffer::PushElement(
                            *list_vx0,
                            VertexAttr0{glm::vec3(pen.x+dx,pen.y-dy,1)});

                gl::VertexBuffer::PushElement(
                            *list_vx1,
                            VertexAttr1{glm::u8vec4(80,80,80,255)});

                gl::VertexBuffer::PushElement(
                            *list_vx2,
                            VertexAttr2{static_cast<float>((row*3)+col)});


                vx_range0.size_bytes = 3*sizeof(VertexAttr0);
                list_vx_ranges0.push_back(vx_range0);

                vx_range1.size_bytes = 3*sizeof(VertexAttr1);
                list_vx_ranges1.push_back(vx_range1);

                vx_range2.size_bytes = 3*sizeof(VertexAttr2);
                list_vx_ranges2.push_back(vx_range2);

                pen.x += dx*2;
            }

            pen.x = -1.0;
            pen.y -= dy*2;
        }
    }

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

                // Create shader
                m_shader =
                        make_unique<gl::ShaderProgram>(
                            vertex_shader,
                            frag_shader);

                m_shader->GLInit();

                // Create indices
                unique_ptr<std::vector<u8>> list_ix =
                        make_unique<std::vector<u8>>();

                CreateTriangleIndices(list_ix,m_list_ix_ranges);
                auto list_ix_size = list_ix->size();


                // AOS vx buff
                unique_ptr<std::vector<u8>> list_vx_aos =
                        make_unique<std::vector<u8>>();

                CreateTrianglesAOS(list_vx_aos,m_list_vx_aos_ranges);
                auto list_vx_aos_size = list_vx_aos->size();

                // create vx buffs
                m_vx_buff_aos =
                        make_shared<gl::VertexBuffer>(
                            vx_layout_aos,
                            gl::Buffer::Usage::Static);

                m_vx_buff_aos->UpdateBuffer(
                            make_unique<gl::Buffer::UpdateFreeData>(
                                gl::Buffer::Update::ReUpload,
                                0,0,
                                list_vx_aos_size,
                                list_vx_aos.release())
                            );

                m_vx_buff_aos->GLInit();
                m_vx_buff_aos->GLBind();
                m_vx_buff_aos->GLSync();
                m_vx_buff_aos->GLUnbind();

                // SOA vx buff
                unique_ptr<std::vector<u8>> list_vx_soa0 =
                        make_unique<std::vector<u8>>();

                unique_ptr<std::vector<u8>> list_vx_soa1 =
                        make_unique<std::vector<u8>>();

                unique_ptr<std::vector<u8>> list_vx_soa2 =
                        make_unique<std::vector<u8>>();

                CreateTrianglesSOA(list_vx_soa0,
                                   list_vx_soa1,
                                   list_vx_soa2,
                                   m_list_vx_soa_ranges0,
                                   m_list_vx_soa_ranges1,
                                   m_list_vx_soa_ranges2);

                auto list_vx_soa0_size = list_vx_soa0->size();
                auto list_vx_soa1_size = list_vx_soa1->size();
                auto list_vx_soa2_size = list_vx_soa2->size();

                m_vx_buff_soa0 =
                        make_unique<gl::VertexBuffer>(
                            vx_layout_soa0,
                            gl::Buffer::Usage::Static);

                m_vx_buff_soa0->UpdateBuffer(
                            make_unique<gl::Buffer::UpdateFreeData>(
                                gl::Buffer::Update::ReUpload,
                                0,0,
                                list_vx_soa0_size,
                                list_vx_soa0.release()));

                m_vx_buff_soa0->GLInit();
                m_vx_buff_soa0->GLBind();
                m_vx_buff_soa0->GLSync();
                m_vx_buff_soa0->GLUnbind();


                m_vx_buff_soa1 =
                        make_unique<gl::VertexBuffer>(
                            vx_layout_soa1,
                            gl::Buffer::Usage::Static);

                m_vx_buff_soa1->UpdateBuffer(
                            make_unique<gl::Buffer::UpdateFreeData>(
                                gl::Buffer::Update::ReUpload,
                                0,0,
                                list_vx_soa1_size,
                                list_vx_soa1.release())
                            );

                m_vx_buff_soa1->GLInit();
                m_vx_buff_soa1->GLBind();
                m_vx_buff_soa1->GLSync();
                m_vx_buff_soa1->GLUnbind();


                m_vx_buff_soa2 =
                        make_unique<gl::VertexBuffer>(
                            vx_layout_soa2,
                            gl::Buffer::Usage::Static);

                m_vx_buff_soa2->UpdateBuffer(
                            make_unique<gl::Buffer::UpdateFreeData>(
                                gl::Buffer::Update::ReUpload,
                                0,0,
                                list_vx_soa2_size,
                                list_vx_soa2.release())
                            );

                m_vx_buff_soa2->GLInit();
                m_vx_buff_soa2->GLBind();
                m_vx_buff_soa2->GLSync();
                m_vx_buff_soa2->GLUnbind();


                // create ix buff
                m_ix_buff =
                        make_shared<gl::IndexBuffer>(
                            gl::Buffer::Usage::Static);

                m_ix_buff->UpdateBuffer(
                            make_unique<gl::Buffer::UpdateFreeData>(
                                gl::Buffer::Update::ReUpload,
                                0,0,
                                list_ix_size,
                                list_ix.release())
                            );

                m_ix_buff->GLInit();
                m_ix_buff->GLBind();
                m_ix_buff->GLSync();
                m_ix_buff->GLUnbind();


                // done init
                m_init = true;
            }

            // ClearScreen
            gl::Clear(gl::ColorBufferBit);

            m_shader->GLEnable(m_state_set.get());

            // Non indexed AOS geometry
            for(uint i=0; i < 3; i++)
            {
                auto& range = m_list_vx_aos_ranges[i];

                bool const ok =
                        m_vx_buff_aos->GLBindVxBuff(
                            m_shader.get(),
                            range.start_byte);
                assert(ok);

                gl::DrawArrays(gl::Primitive::Triangles,
                               m_vx_buff_aos->GetVertexSizeBytes(),
                               0,range.size_bytes);
            }

            // Indexed AOS geometry
            for(uint i=3; i < 6; i++)
            {
                auto& vx_range = m_list_vx_aos_ranges[i];
                auto& ix_range = m_list_ix_ranges[i];

                bool const ok =
                        m_vx_buff_aos->GLBindVxBuff(
                            m_shader.get(),
                            vx_range.start_byte) &&
                        m_ix_buff->GLBind();
                assert(ok);

                gl::DrawElements(gl::Primitive::Triangles,
                                 ix_range.start_byte,
                                 ix_range.size_bytes);
            }

            m_vx_buff_aos->GLUnbind();


            // Non indexed SOA geometry
            for(uint i=0; i < 3; i++)
            {
                auto& range0 = m_list_vx_soa_ranges0[i];
                auto& range1 = m_list_vx_soa_ranges1[i];
                auto& range2 = m_list_vx_soa_ranges2[i];

                bool const ok =
                        m_vx_buff_soa0->GLBindVxBuff(
                            m_shader.get(),
                            range0.start_byte) &&

                        m_vx_buff_soa1->GLBindVxBuff(
                            m_shader.get(),
                            range1.start_byte) &&

                        m_vx_buff_soa2->GLBindVxBuff(
                            m_shader.get(),
                            range2.start_byte);
                assert(ok);

                // only need one range; they should all line up
                gl::DrawArrays(gl::Primitive::Triangles,
                               m_vx_buff_soa0->GetVertexSizeBytes(),
                               0,range0.size_bytes);
            }

            // Indexed SOA geometry
            for(uint i=3; i < 6; i++)
            {
                auto& ix_range = m_list_ix_ranges[i];
                auto& range0 = m_list_vx_soa_ranges0[i];
                auto& range1 = m_list_vx_soa_ranges1[i];
                auto& range2 = m_list_vx_soa_ranges2[i];

                bool const ok =
                        m_vx_buff_soa0->GLBindVxBuff(
                            m_shader.get(),
                            range0.start_byte) &&

                        m_vx_buff_soa1->GLBindVxBuff(
                            m_shader.get(),
                            range1.start_byte) &&

                        m_vx_buff_soa2->GLBindVxBuff(
                            m_shader.get(),
                            range2.start_byte);
                assert(ok);

                gl::DrawElements(gl::Primitive::Triangles,
                                 ix_range.start_byte,
                                 ix_range.size_bytes);
            }

            m_vx_buff_soa0->GLUnbind();
            m_vx_buff_soa1->GLUnbind();
            m_vx_buff_soa2->GLUnbind();
            m_ix_buff->GLUnbind();
        }

    private:
        bool m_init;
        unique_ptr<gl::StateSet> m_state_set;
        unique_ptr<gl::ShaderProgram> m_shader;

        shared_ptr<gl::IndexBuffer> m_ix_buff;
        std::vector<BufferRange> m_list_ix_ranges;

        // aos
        shared_ptr<gl::VertexBuffer> m_vx_buff_aos;
        std::vector<BufferRange> m_list_vx_aos_ranges;

        // soa
        unique_ptr<gl::VertexBuffer> m_vx_buff_soa0;
        unique_ptr<gl::VertexBuffer> m_vx_buff_soa1;
        unique_ptr<gl::VertexBuffer> m_vx_buff_soa2;
        std::vector<BufferRange> m_list_vx_soa_ranges0;
        std::vector<BufferRange> m_list_vx_soa_ranges1;
        std::vector<BufferRange> m_list_vx_soa_ranges2;
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
                Milliseconds(15),
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



