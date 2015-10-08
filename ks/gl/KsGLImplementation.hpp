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

#include <unordered_set>
#include <ks/gl/KsGLDebug.hpp>

namespace ks
{
    namespace gl
    {
        namespace Implementation
        {
            // Must only be called from within an active
            // OpenGL context
            void GLCapture();

            std::string const & GetGLVendor();
            std::string const & GetGLRenderer();
            std::string const & GetGLVersion();
            std::string const & GetGLSLVersion();
            bool GetGLExtensionExists(std::string const &ext);
            GLint GetMaxTextureSize();
            GLint GetMaxCubeMapTextureSize();
            GLint GetMaxVertexAttribs();
            GLint GetMaxVertexUniformVectors();
            GLint GetMaxVaryingVectors();
            GLint GetMaxCombinedTextureImageUnits();
            GLint GetMaxVertexTextureImageUnits();
            GLint GetMaxTextureImageUnits();
            GLint GetMaxFragmentUniformVectors();
            GLint GetMaxRenderBufferSize();
        }
    }
}
