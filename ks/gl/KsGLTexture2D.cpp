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

#include <ks/gl/KsGLTexture2D.hpp>
#include <ks/gl/KsGLStateSet.hpp>
#include <ks/gl/KsGLImplementation.hpp>
#include <ks/shared/KsImage.hpp>

namespace ks
{
    namespace gl
    {
        Texture2D::Texture2D(u16 width,
                             u16 height,
                             Format format,
                             Filter filter_min,
                             Filter filter_mag,
                             Wrap wrap_s,
                             Wrap wrap_t) :
            m_width(width),
            m_height(height),
            m_format(format)
        {
            // save params
            if(m_format == Format::RGB8) {
                m_gl_format = GL_RGB;
                m_gl_datatype = GL_UNSIGNED_BYTE;
            }
            else if(m_format == Format::RGBA8) {
                m_gl_format = GL_RGBA;
                m_gl_datatype = GL_UNSIGNED_BYTE;
            }
            else if(m_format == Format::LUMINANCE8) {
                m_gl_format = GL_LUMINANCE;
                m_gl_datatype = GL_UNSIGNED_BYTE;
            }
            else if(m_format == Format::RGBA4) {
                m_gl_format = GL_RGBA;
                m_gl_datatype = GL_UNSIGNED_SHORT_4_4_4_4;
            }
            else if(m_format == Format::RGB5_A1) {
                m_gl_format = GL_RGBA;
                m_gl_datatype = GL_UNSIGNED_SHORT_5_5_5_1;
            }
            else if(m_format == Format::RGB565) {
                m_gl_format = GL_RGB;
                m_gl_datatype = GL_UNSIGNED_SHORT_5_6_5;
            }
            else if(m_format == Format::DEPTH_COMPONENT16) {
                #ifdef KS_ENV_GL_ES
                if(!Implementation::GetGLExtensionExists("GL_OES_depth_texture")) {
                    LOG.Error()<< m_log_prefix
                              << "Depth texture requested but "
                                 "GL_OES_depth_texture N/A";
                }
                #endif
                m_gl_format = GL_DEPTH_COMPONENT;
                m_gl_datatype = GL_UNSIGNED_SHORT;
            }
            else if(m_format == Format::DEPTH_COMPONENT32) {
                #ifdef KS_ENV_GL_ES
                if(!Implementation::GetGLExtensionExists("GL_OES_depth_texture")) {
                    LOG.Error()<< m_log_prefix
                              << "Depth texture requested but "
                                 "GL_OES_depth_texture N/A";
                }
                #endif
                m_gl_format = GL_DEPTH_COMPONENT;
                m_gl_datatype = GL_UNSIGNED_INT;
            }
            else if(m_format == Format::DEPTH24_STENCIL8) {
                #if defined(KS_ENV_GL_ES)
                if(!Implementation::GetGLExtensionExists("GL_OES_packed_depth_stencil")) {
                    LOG.Error()<< m_log_prefix
                              << "Packed depth+stencil texture requested but "
                                 "GL_OES_packed_depth_stencil N/A";
                }
                m_gl_format = GL_DEPTH_STENCIL_OES;
                m_gl_datatype = GL_UNSIGNED_INT_24_8_OES;
                #elif defined(KS_ENV_GL_DESKTOP)
                if(!Implementation::GetGLExtensionExists("GL_ARB_framebuffer_object")) {
                    // GL_ARB_framebuffer_object combines:
                    // EXT_framebuffer_object
                    // EXT_framebuffer_blit
                    // EXT_framebuffer_multisample
                    // EXT_packed_depth_stencil <---
                    LOG.Error()<< m_log_prefix
                              << "Packed depth+stencil texture requested but "
                                 "GL_ARB_framebuffer_object N/A";
                }
                m_gl_format = GL_DEPTH_STENCIL;
                m_gl_datatype = GL_UNSIGNED_INT_24_8;
                #endif
            }

            // We explicitly set the filtering and wrap modes
            // in the constructor because texturing might not
            // work if these aren't set at least once.

            // Texture parameters (wrap,filter) are owned/
            // saved to the texture object so we shouldn't
            // need to set them per frame.
            m_filter_min = filter_min;
            m_filter_mag = filter_mag;

            m_wrap_s = wrap_s;
            m_wrap_t = wrap_t;
        }

        Texture2D::~Texture2D()
        {
            // empty
        }

        bool Texture2D::GLBind(StateSet* state_set,GLuint tex_unit)
        {
            if(m_texture_handle == 0) {
                LOG.Error() << m_log_prefix
                            << "tried to bind with texture 0";
                return false;
            }

            // set the active texture unit
            state_set->SetActiveTexUnitAndBind(
                        tex_unit,
                        m_texture_handle,
                        GL_TEXTURE_2D,
                        m_id);

            return true;
        }


        void Texture2D::GLUnbind()
        {
            // do nothing
        }

        void Texture2D::GLSync()
        {
            // Find the last update where reupload==true
            for(auto it = m_list_updates.end();
                it != m_list_updates.begin();)
            {
                --it;

                if(it->dst_reupload)
                {
                    if(!it->src_null)
                    {
                        assert(m_width == it->src_data->width);
                        assert(m_height == it->src_data->height);
                        assert(0 == it->src_offset.x);
                        assert(0 == it->src_offset.y);

                        glTexImage2D(GL_TEXTURE_2D,
                                     0, // mipmap level
                                     m_gl_format,
                                     m_width,
                                     m_height,
                                     0, // border, not used for GLES
                                     m_gl_format,
                                     m_gl_datatype,
                                     it->src_data->data_ptr);
                    }
                    else
                    {
                        // Create the texture with @data set to 0;
                        // This is well defined behavior; the texture will
                        // be created but image data is unspecified (see
                        // ES 2 spec, 3.7.1 p 69)
                        glTexImage2D(GL_TEXTURE_2D,
                                     0, // mipmap level
                                     m_gl_format,
                                     m_width,
                                     m_height,
                                     0, // border, not used for GLES
                                     m_gl_format,
                                     m_gl_datatype,
                                     0); // data
                    }
                    KS_CHECK_GL_ERROR(m_log_prefix+"upload texture");

                    // set filter
                    glTexParameteri(GL_TEXTURE_2D,
                                    GL_TEXTURE_MIN_FILTER,
                                    static_cast<GLint>(m_filter_min));

                    glTexParameteri(GL_TEXTURE_2D,
                                    GL_TEXTURE_MAG_FILTER,
                                    static_cast<GLint>(m_filter_mag));

                    KS_CHECK_GL_ERROR(m_log_prefix+"texture filter params");

                    // set wrap
                    glTexParameteri(GL_TEXTURE_2D,
                                    GL_TEXTURE_WRAP_S,
                                    static_cast<GLint>(m_wrap_s));

                    glTexParameteri(GL_TEXTURE_2D,
                                    GL_TEXTURE_WRAP_T,
                                    static_cast<GLint>(m_wrap_t));

                    KS_CHECK_GL_ERROR(m_log_prefix+"texture wrap params");

                    m_list_updates.erase(m_list_updates.begin(),
                                         std::next(it));
                    break;
                }
            }

            for(Update& update : m_list_updates)
            {
                // upload the image
                glTexSubImage2D(GL_TEXTURE_2D,
                                0, // mipmap level
                                update.src_offset.x,
                                update.src_offset.y,
                                update.src_data->width,
                                update.src_data->height,
                                m_gl_format,
                                m_gl_datatype,
                                update.src_data->data_ptr);

                KS_CHECK_GL_ERROR(m_log_prefix+"upload subimage");
            }

            m_list_updates.clear();
        }

        void Texture2D::UpdateTexture(Update update)
        {
            m_list_updates.push_back(std::move(update));
        }

        u32 Texture2D::calcNumBytes() const
        {
            double num_pixels = m_width*m_height;
            double bpp =
                    (m_format == Format::RGBA8) ? 4 :
                    (m_format == Format::RGB8) ? 3 :
                    (m_format == Format::LUMINANCE8) ? 1 :
                    (m_format == Format::RGBA4) ? 2 :
                    (m_format == Format::RGB5_A1) ? 2 :
                    (m_format == Format::RGB565) ? 2 :
                    (m_format == Format::DEPTH_COMPONENT16) ? 2 :
                    (m_format == Format::DEPTH_COMPONENT32) ? 4 :
                    (m_format == Format::DEPTH24_STENCIL8) ? 4 : 0;

            return static_cast<u32>(num_pixels*bpp);
        }
    } // gl
} // ks
