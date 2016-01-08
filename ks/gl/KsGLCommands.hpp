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


#ifndef KS_GL_COMMANDS_HPP
#define KS_GL_COMMANDS_HPP

// stl
#include <vector>

// ks
#include <ks/gl/KsGLVertexBuffer.hpp>
#include <ks/gl/KsGLIndexBuffer.hpp>

namespace ks
{
    namespace gl
    {
        // ============================================================= //

        inline void DrawArrays(Primitive primitive,
                               uint vx_size_bytes,
                               uint vx_array_start_byte,
                               uint vx_array_size_bytes)
        {
            glDrawArrays(static_cast<GLenum>(primitive),
                         vx_array_start_byte/vx_size_bytes,
                         vx_array_size_bytes/vx_size_bytes);

            KS_CHECK_GL_ERROR("DrawArrays");
        }

        // ============================================================= //

        inline void DrawElements(Primitive primitive,
                                 uint ix_range_start_byte,
                                 uint ix_range_size_bytes)
        {
            std::uintptr_t offset_bytes = ix_range_start_byte;

            glDrawElements(static_cast<GLenum>(primitive),
                           ix_range_size_bytes/2, // 2 bytes per index
                           GL_UNSIGNED_SHORT,
                           reinterpret_cast<void*>(offset_bytes));

            KS_CHECK_GL_ERROR("DrawElements");
        }

        // ============================================================= //

        inline void DrawArrays(Primitive primitive,
                               ShaderProgram* shader,
                               VertexBuffer* vertex_buffer,
                               uint vx_range_start_byte,
                               uint vx_range_size_bytes)
        {
            bool const ok = vertex_buffer->GLBindVxBuff(shader);
            assert(ok);

            // bytes per vertex
            uint const vx_size = vertex_buffer->GetVertexSizeBytes();

            glDrawArrays(static_cast<GLenum>(primitive),
                         vx_range_start_byte/vx_size,
                         vx_range_size_bytes/vx_size);

            KS_CHECK_GL_ERROR(vertex_buffer->GetDesc()+"DrawArrays");
            vertex_buffer->GLUnbind();
        }

        // ============================================================= //

        inline void DrawElements(Primitive primitive,
                                 ShaderProgram* shader,
                                 VertexBuffer* vertex_buffer,
                                 IndexBuffer* index_buffer,
                                 uint ix_range_start,
                                 uint ix_range_size)
        {
            bool const ok =
                vertex_buffer->GLBindVxBuff(shader) &&
                index_buffer->GLBind();

            assert(ok);

            // bytes per index
            uint const ix_size = 2; // 2 bytes per index

            glDrawElements(static_cast<GLenum>(primitive),
                           ix_range_size/ix_size,
                           GL_UNSIGNED_SHORT,
                           reinterpret_cast<void*>(ix_range_start)); // byte offset into index buffer

            KS_CHECK_GL_ERROR(vertex_buffer->GetDesc()+"DrawElements");

            vertex_buffer->GLUnbind();
            index_buffer->GLUnbind();
        }

        // ============================================================= //
        // ============================================================= //

        inline void Scissor(sint x, sint y, sint width, sint height)
        {
            glScissor(x,y,width,height);
            KS_CHECK_GL_ERROR("set scissor");
        }

        // ============================================================= //

        inline void Viewport(sint x, sint y, sint width, sint height)
        {
            glViewport(x,y,width,height);
            KS_CHECK_GL_ERROR("set viewport");
        }

        // ============================================================= //

        enum ClearBufferBits
        {
            ColorBufferBit = GL_COLOR_BUFFER_BIT,
            DepthBufferBit = GL_DEPTH_BUFFER_BIT,
            StencilBufferBit = GL_STENCIL_BUFFER_BIT
        };

        inline void Clear(uint mask)
        {
            glClear(mask);
            KS_CHECK_GL_ERROR("clear");
        }

        // ============================================================= //

        inline void Finish()
        {
            glFinish();
            KS_CHECK_GL_ERROR("finish");
        }

        // ============================================================= //

    } // gl
} // ks

#endif // KS_GL_COMMANDS_HPP
