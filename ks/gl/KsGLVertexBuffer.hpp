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


#ifndef KS_GL_VERTEX_BUFFER_HPP
#define KS_GL_VERTEX_BUFFER_HPP

// stl
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>

// ks
#include <ks/gl/KsGLBuffer.hpp>
#include <ks/gl/KsGLShaderProgram.hpp>

namespace ks
{
    namespace gl
    {       
        enum class Primitive : GLenum
        {
            Triangles       = GL_TRIANGLES,
            TriangleFan     = GL_TRIANGLE_FAN,
            TriangleStrip   = GL_TRIANGLE_STRIP,
            Lines           = GL_LINES,
            LineLoop        = GL_LINE_LOOP,
            LineStrip       = GL_LINE_STRIP,
            Points          = GL_POINTS
        };

        // ============================================================= //

        class VertexBuffer final : public Buffer
        {
        public:

            // ============================================================= //

            struct Attribute
            {
                static const std::vector<GLenum> list_type_glenums;
                static const std::vector<u8> list_type_sizes;

                enum class Type : u8 {
                    Byte    = 0,
                    UByte   = 1,
                    Short   = 2,
                    UShort  = 3,
                    Float   = 4
                };

                class Desc
                {
                    friend class VertexBuffer;

                public:
                    Desc(std::string name,
                         Type type,
                         u8 component_count,
                         bool normalized) :
                        m_name(std::move(name)),
                        m_type(type),
                        m_normalized(normalized)
                    {
                        bool const invalid_count =
                                (component_count < 1) ||
                                (component_count > 4);

                        if(invalid_count) {
                            LOG.Error() << "VertexBuffer::Attribute::Desc: "
                                           "Invalid component count! Must be "
                                           "in the range [1-4]";
                            assert(invalid_count);
                        }

                        m_component_count = component_count;
                        uint const type_index = static_cast<uint>(m_type);
                        m_sz_bytes = m_component_count*list_type_sizes[type_index];
                    }

                    u8 GetSizeBytes() const {
                        return m_sz_bytes;
                    }

                private:
                    std::string m_name;
                    Type m_type;
                    bool m_normalized;
                    u8 m_component_count;
                    u8 m_sz_bytes;
                };
            };

            // ============================================================= //

            VertexBuffer(std::vector<Attribute::Desc> list_attribs,
                         Usage usage = Usage::Static);

            ~VertexBuffer();

            uint GetVertexSizeBytes() const {
                return m_vertex_sz_bytes;
            }

            bool GLBindVxBuff(ShaderProgram* shader,
                              uint const offset_bytes=0);

        private:
            static u16 calcVertexSize(
                    std::vector<Attribute::Desc> const &list_attribs);

            // * The total size in bytes for a single vertex
            u16 const m_vertex_sz_bytes;

            // * List of attributes and their description
            std::vector<Attribute::Desc> const m_list_attribs;

            // * Cache for quick lookup for shader vertex attribute locations
            //   corresponding to this buffer's vertex attribute indices
            using AttribLocsByShader =
                std::pair<ShaderProgram*,std::vector<GLuint>>;

            std::vector<AttribLocsByShader> m_list_shader_attr_locs;
        };

        using VertexLayout = std::vector<VertexBuffer::Attribute::Desc>;

        // ============================================================= //

    } // gl
} // ks

#endif // KS_GL_VERTEX_BUFFER_HPP
