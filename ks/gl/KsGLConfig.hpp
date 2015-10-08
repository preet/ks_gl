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

#ifndef KS_GL_CONFIG_HPP
#define KS_GL_CONFIG_HPP

#include <ks/KsConfig.hpp>


#if defined(KS_ENV_ANDROID)
    #define KS_ENV_GL_ES 1
#else
    #define KS_ENV_GL_DESKTOP 1
    #define KS_ENV_GL_LOAD_FUNCPTRS 1
#endif

#if defined(KS_ENV_GL_DESKTOP)
    #include <glad/glad.h>
#endif

#if defined(KS_ENV_GL_ES)
    #include <GLES2/gl2.h>
    #include <GLES2/gl2ext.h>
#endif


#endif // KS_GL_CONFIG_HPP
