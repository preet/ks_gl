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


#ifndef KS_GL_INDEX_BUFFER_HPP
#define KS_GL_INDEX_BUFFER_HPP

// stl
#include <vector>

// ks
#include <ks/gl/KsGLVertexBuffer.hpp>

namespace ks
{
    namespace gl
    {
        class IndexBuffer final : public Buffer
        {
        public:
            IndexBuffer(Usage usage=Usage::Static);
            ~IndexBuffer();
        };
    } // gl
} // ks

#endif // KS_GL_INDEX_BUFFER_HPP
