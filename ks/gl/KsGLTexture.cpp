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


#include <ks/gl/KsGLTexture.hpp>
#include <iostream>

namespace ks
{
    namespace gl
    {
        uint64_t Texture::s_id_count = 0;

        Texture::Texture() :
            m_texture_handle(0)
        {
            m_log_prefix = "Texture: ";
        }

        Texture::~Texture()
        {
            // empty
        }

        GLuint Texture::GetHandle() const
        {
            return m_texture_handle;
        }

        std::string const &Texture::GetDesc() const
        {
            return m_log_prefix;
        }

        void Texture::SetDesc(std::string desc)
        {
            m_log_prefix = "Texture: " + desc;
        }

        bool Texture::GLInit()
        {
            if(!(m_texture_handle == 0)) {
                LOG.Error() << m_log_prefix
                            << "already have valid handle: "
                            << m_texture_handle;
                return false;
            }

            glGenTextures(1,&m_texture_handle);
            KS_CHECK_GL_ERROR(m_log_prefix+"gen textures");

            if(m_texture_handle == 0) {
                LOG.Error() << m_log_prefix
                           << "failed to gen texture";
                return false;
            }

            // We give each texture a unique id so that the
            // StateSet can minimize state changes when setting
            // the active texture unit or binding textures.
            // OpenGL texture handles aren't good enough because
            // they can be reclaimed when textures are destroyed.
            m_id = s_id_count++;

            return true;
        }

        void Texture::GLCleanUp()
        {
            if(!(m_texture_handle == 0)) {
                glDeleteTextures(1,&m_texture_handle);
                m_texture_handle = 0;
            }
        }
    } // gl
} // ks
