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

#ifndef KS_GL_BUFFER_HPP
#define KS_GL_BUFFER_HPP

// stl
#include <string>
#include <vector>
#include <unordered_map>

// ks
#include <ks/gl/KsGLResource.hpp>
#include <ks/gl/KsGLShaderProgram.hpp>

namespace ks
{
    namespace gl
    {
        class Buffer : public Resource
        {
        public:
            enum class Target : GLenum
            {
                ArrayBuffer = GL_ARRAY_BUFFER,
                ElementArrayBuffer = GL_ELEMENT_ARRAY_BUFFER
            };

            enum class Usage : GLenum
            {
                // For vertex buffers that are rendered many times,
                // and whose contents are specified once and never change
                Static  = GL_STATIC_DRAW,

                // For vertex buffers that are rendered many times, and
                // whose contents change during the rendering loop
                Dynamic = GL_DYNAMIC_DRAW,

                // for vertex buffers that are rendered a small number
                // of times and then discarded
                Stream  = GL_STREAM_DRAW
            };

            struct Update
            {
                static u8 const Defaults    = 0;
                static u8 const ReUpload    = 1 << 0;
                static u8 const KeepSrcData = 1 << 1;

                u8 options;
                uint dst_byte_offset;
                uint src_byte_offset;
                size_t src_sz_bytes;
                std::vector<u8>* src_data;
            };


            Buffer(Target target, Usage usage);
            ~Buffer();

            std::string const & GetDesc() const;
            void SetDesc(std::string desc);

            uint GetSizeBytes() const;

            // * Returns the current list of updates, used
            //   primarily for debugging
            std::vector<Update> const & GetUpdates() const;

            bool GLInit();
            virtual bool GLBind();
            virtual void GLUnbind();
            void GLCleanUp();

            // * Must be called from a thread that has a valid GL
            //   context and must be synchronous with respect to
            //   any threads calling Update
            // * The buffer must be bound before this is called
            void GLSync();

            // * May be called asynchronously with respect to the
            //   current rendering thread, but is not thread safe
            void UpdateBuffer(Update const &update);

            // TODO desc
            template<typename T>
            static T& GetElement(std::vector<u8> &buffer,size_t index)
            {
                size_t byte_index = index*sizeof(T);
                return *(reinterpret_cast<T*>(&buffer[byte_index]));
            }

            // TODO desc
            template<typename T>
            static void PushElement(std::vector<u8> &buffer,T &&element)
            {
                // http://stackoverflow.com/questions/2692383/
                // http://stackoverflow.com/questions/259297/

                u8* element_mem = reinterpret_cast<u8*>(&element);

                buffer.insert(
                            buffer.end(),
                            element_mem,
                            element_mem+sizeof(T));
            }

        protected:
            Target m_target;
            Usage m_usage;
            bool m_opt_retain_buffer{false}; // TODO

            GLuint m_buffer_handle{0};
            uint m_lk_buffer_size{0};
            std::vector<u8> m_buffer;

            std::vector<Update> m_list_updates;

            std::string m_log_prefix = "Buffer: ";
        };

    } // gl
} // ks

#endif // KS_GL_BUFFER_HPP
