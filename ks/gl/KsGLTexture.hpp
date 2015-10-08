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


#ifndef KS_GL_TEXTURE_HPP
#define KS_GL_TEXTURE_HPP

// stl
#include <memory>

// ks
#include <ks/gl/KsGLStateSet.hpp>
#include <ks/gl/KsGLResource.hpp>

namespace ks
{
    namespace gl
    {
        class Texture : public Resource
        {
        public:
            enum class Filter : GLenum {
                Linear = GL_LINEAR,
                Nearest = GL_NEAREST
            };

            enum class Wrap : GLenum {
                ClampToEdge = GL_CLAMP_TO_EDGE, // npot is okay TODO verify
                MirroredRepeat = GL_MIRRORED_REPEAT, // must be power of two
                Repeat = GL_REPEAT // must be power of two
            };

            Texture();
            ~Texture();
            GLuint GetHandle() const;
            std::string const & GetDesc() const;
            void SetDesc(std::string desc);

            virtual bool GLInit();

            virtual bool GLBind(StateSet* state_set,
                                GLuint tex_unit) = 0;

            virtual void GLUnbind() = 0;

            virtual void GLCleanUp();

        protected:
            std::string m_log_prefix;
            u64 m_id;

            GLuint m_texture_handle;

        private:
            static u64 s_id_count;
        };
    } // gl
} // ks


#endif // KS_GL_TEXTURE_HPP
