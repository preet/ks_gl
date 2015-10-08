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

#include <ks/KsMiscUtils.hpp>
#include <ks/gl/KsGLDebug.hpp>

namespace ks
{
    namespace gl
    {
        void CheckGLError()
        {
            int err_count=0;
            GLenum gl_error;
            while((gl_error=glGetError()) != GL_NO_ERROR) {
                err_count++;
                LOG.Error() << "GL Error: " << GetGLErrorDesc(gl_error);
            }
            assert(err_count == 0);
        }

        void CheckGLError(std::string const &info)
        {
            int err_count=0;
            GLenum gl_error;
            while((gl_error=glGetError()) != GL_NO_ERROR) {
                err_count++;
                if(!info.empty()) {
                    LOG.Error() << "GL Error: " << info;
                }
                LOG.Error() << "GL Error: " << GetGLErrorDesc(gl_error);
            }
            assert(err_count == 0);
        }

        std::string GetGLErrorDesc(GLenum const gl_error)
        {
            switch(gl_error)
            {
                case GL_NO_ERROR: {
                    return "GL_NO_ERROR";
                }
                case GL_INVALID_ENUM: {
                    return "GL_INVALID_ENUM";
                }
                case GL_INVALID_VALUE: {
                    return "GL_INVALID_VALUE";
                }
                case GL_INVALID_OPERATION: {
                    return "GL_INVALID_OPERATION";
                }
                case GL_OUT_OF_MEMORY: {
                    return "GL_OUT_OF_MEMORY";
                }
                default: {
                    std::string msg="UNKNOWN ERROR: ";
                    msg.append(ConvNumberToString(gl_error));
                    return msg;
                }
            }
        }
    } // gl
} // namespace ks
