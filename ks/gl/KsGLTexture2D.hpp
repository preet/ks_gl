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

#ifndef KS_GL_TEXTURE_2D_HPP
#define KS_GL_TEXTURE_2D_HPP

// glm
#include <glm/vec2.hpp>
#include <glm/gtc/type_precision.hpp>

// ks
#include <ks/gl/KsGLTexture.hpp>

namespace ks
{
    struct ImageData;

    namespace gl
    {
        class StateSet;

        class Texture2D : public Texture
        {
        public:
            enum class Format {
                RGBA8,
                RGB8,
                LUMINANCE8,
                RGBA4,
                RGB5_A1,
                RGB565,
                DEPTH_COMPONENT16,
                DEPTH_COMPONENT32,
                DEPTH24_STENCIL8
            };

            struct Update
            {
                bool dst_reupload;

                bool src_null;
                glm::u16vec2 src_offset;
                std::shared_ptr<ImageData const> src_data;
            };

            // Note: Textures should only be created after
            // the GL Implementation has been captured (this
            // should be true of all GL resources)
            Texture2D(u16 width,
                      u16 height,
                      Format format,
                      Filter filter_min = Filter::Linear,
                      Filter filter_mag = Filter::Linear,
                      Wrap wrap_s = Wrap::Repeat,
                      Wrap wrap_t = Wrap::Repeat);

            ~Texture2D();

            bool GLBind(StateSet* state_set,GLuint tex_unit);

            void GLUnbind();

            void GLSync();

            void UpdateTexture(Update update);

        private:
            // calculate the number of bytes in the texture
            // based on the dimensions, format and datatype
            u32 calcNumBytes() const;

            // Flag that indicates whether or not the
            // the texture has been uploaded to OpenGL
            // and the texture params have been set
            u16 const m_width;
            u16 const m_height;
            Format m_format;

            GLenum m_gl_format;
            GLenum m_gl_datatype;

            Filter m_filter_min;
            Filter m_filter_mag;

            Wrap m_wrap_s;
            Wrap m_wrap_t;

            std::vector<Update> m_list_updates;
        };
    } // gl
} // ks

#endif // KS_GL_TEXTURE_2D_HPP
