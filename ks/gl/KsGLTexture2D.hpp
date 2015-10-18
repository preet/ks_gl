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
                static u8 const Defaults    = 0;
                static u8 const ReUpload    = 1 << 0;
                static u8 const KeepSrcData = 1 << 1;

                u8 options;
                glm::u16vec2 src_offset;
                ImageData const * src_data;
            };

            // Note: Textures should only be created after
            // the GL Implementation has been captured (this
            // should be true of all GL resources)
            Texture2D(Format format);

            ~Texture2D();

            bool GLBind(StateSet* state_set,GLuint tex_unit);

            void GLUnbind();

            void GLSync();

            uint GetUpdateCount() const;

            void UpdateTexture(Update update);

            void SetFilterModes(Filter filter_min,Filter filter_mag);

            void SetWrapModes(Wrap wrap_s,Wrap wrap_t);

        private:
            // calculate the number of bytes in the texture
            // based on the dimensions, format and datatype
            u32 calcNumBytes() const;

            u16 m_width;
            u16 m_height;
            Format m_format;

            GLenum m_gl_format;
            GLenum m_gl_datatype;

            Filter m_filter_min;
            Filter m_filter_mag;

            Wrap m_wrap_s;
            Wrap m_wrap_t;

            bool m_upd_params;

            std::vector<Update> m_list_updates;
        };
    } // gl
} // ks

#endif // KS_GL_TEXTURE_2D_HPP
