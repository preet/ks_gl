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

// stl
#include <string>
#include <vector>
#include <cstdint>
#include <unordered_map>
#include <algorithm>

// glm
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

// ks
#include <ks/gl/KsGLVertexBuffer.hpp>

namespace ks
{
    namespace gl
    {
        const std::vector<GLenum> VertexBuffer::Attribute::list_type_glenums = {
            GL_BYTE,
            GL_UNSIGNED_BYTE,
            GL_SHORT,
            GL_UNSIGNED_SHORT,
            GL_FLOAT
        };

        const std::vector<u8> VertexBuffer::Attribute::list_type_sizes = {
            1,
            1,
            2,
            2,
            4
        };

        // ============================================================= //

        VertexBuffer::VertexBuffer(std::vector<Attribute::Desc> list_attribs,
                                   Usage usage) :
            Buffer(Target::ArrayBuffer,usage),
            m_vertex_sz_bytes(calcVertexSize(list_attribs)),
            m_list_attribs(list_attribs)
        {

        }

        VertexBuffer::~VertexBuffer()
        {

        }

        bool VertexBuffer::GLBindVxBuff(ShaderProgram* shader,
                                        uint const offset_bytes)
        {
            if(!this->GLBind()) {
                return false;
            }

            std::uintptr_t offset = offset_bytes;

            // Get the attribute location list for this shader
            auto attr_it = std::find_if(
                        m_list_shader_attr_locs.begin(),
                        m_list_shader_attr_locs.end(),
                        [shader](AttribLocsByShader const &attrs_by_shader) -> bool {
                            return(attrs_by_shader.first == shader);
                        });

            // If we don't already have it cache all attribute
            // locations for this shader for quick lookup
            if(attr_it == m_list_shader_attr_locs.end())
            {
                std::vector<GLuint> list_attr_locs;
                list_attr_locs.reserve(m_list_attribs.size());

                for(auto& attr : m_list_attribs) {
                    auto attrib_loc = shader->GetAttributeLocation(attr.m_name);
                    if(attrib_loc < 0) {
                        LOG.Error() << "VertexBuffer::GLBindVxBuff: "
                                       "invalid attrib loc: " << attr.m_name;
                        return false;
                    }
                    list_attr_locs.push_back(attrib_loc);
                }

                attr_it = m_list_shader_attr_locs.emplace(
                            m_list_shader_attr_locs.end(),
                            shader,
                            list_attr_locs);
            }

            // Call glVertexAttribPointer to specify the layout
            // of the vertex attributes in the buffer data
            for(size_t i=0; i < m_list_attribs.size(); i++)
            {
                GLint const attrib_location = attr_it->second[i];

                // TODO
                // Is it okay to specify vertex attribute locations
                // out of order? There's no guarantee we specify the
                // locations (0,1,2...)

                u8 type_idx = static_cast<u8>(m_list_attribs[i].m_type);

                // The GLenum that represents the data format of the
                // attribute (GL_BYTE, GL_UNSIGNED_BYTE, GL_FLOAT, etc)
                GLenum attrib_type = Attribute::list_type_glenums[type_idx];

                glVertexAttribPointer(attrib_location,
                                      m_list_attribs[i].m_component_count,
                                      attrib_type,
                                      m_list_attribs[i].m_normalized,
                                      m_vertex_sz_bytes,
                                      (const void*)offset);

                // debug
                // LOG.Info() << m_log_prefix
                //            << "index: " << attrib_location
                //            << ", size: " << num_components
                //            << ", type: " << attrib_type
                //            << ", normalized: " << m_list_attribs[i].normalized
                //            << ", stride: " << m_vertex_sz
                //            << ", offset: " << 0;

                offset += m_list_attribs[i].m_sz_bytes;
            }
            return true;
        }

        u16 VertexBuffer::calcVertexSize(
                std::vector<Attribute::Desc> const &list_attribs)
        {
            u16 vertex_sz_bytes = 0;
            for(auto &attr_desc : list_attribs) {
                vertex_sz_bytes += attr_desc.m_sz_bytes;
            }

            return vertex_sz_bytes;
        }

    } // gl
} // ks
