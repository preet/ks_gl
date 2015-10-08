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

#ifndef KS_GL_SHADER_PROGRAM_HPP
#define KS_GL_SHADER_PROGRAM_HPP

// stl
#include <string>
#include <memory>
#include <unordered_map>

// glm
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

// ks
#include <ks/KsMiscUtils.hpp>
#include <ks/gl/KsGLDebug.hpp>
#include <ks/gl/KsGLResource.hpp>
#include <ks/gl/KsGLStateSet.hpp>

namespace ks
{
    namespace gl
    {

        class ShaderProgram : public Resource
        {
        public:
            ShaderProgram(std::string source_vsh,
                          std::string source_fsh,
                          std::string glsl_version="");

            ~ShaderProgram();

            //
            bool GetAttributeLocation(std::string const &name,
                                      GLuint &location);

            bool GetUniformLocation(std::string const &name,
                                    GLuint &location);


            GLint GetAttributeLocation(std::string const &name) const;
            GLint GetUniformLocation(std::string const &name) const;

            std::string const & GetDesc() const;
            GLuint GetHandle() const;
            bool IsInit() const;

            void SetDesc(std::string desc);

            // debug
            void PrintAttributes() const;
            void PrintUniforms() const;

            //
            bool GLInit();
            void GLEnable(StateSet * state_set);
            void GLDisable();
            void GLCleanUp();

            // GLSetUniform
            void GLSetUniform(std::string const &name,GLint i);
            void GLSetUniform(std::string const &name,GLfloat f);
            void GLSetUniform(std::string const &name,glm::vec2 const &v2);
            void GLSetUniform(std::string const &name,glm::vec3 const &v3);
            void GLSetUniform(std::string const &name,glm::vec4 const &v4);
            void GLSetUniform(std::string const &name,glm::mat4 const &m4);

            void GLSetUniform(std::string const &name,std::vector<GLint> const &ia);
            void GLSetUniform(std::string const &name,std::vector<GLfloat> const &fa);
            void GLSetUniform(std::string const &name,std::vector<glm::vec2> const &v2a);
            void GLSetUniform(std::string const &name,std::vector<glm::vec3> const &v3a);
            void GLSetUniform(std::string const &name,std::vector<glm::vec4> const &v4a);
            void GLSetUniform(std::string const &name,std::vector<glm::mat4> const &m4a);

        private:
            struct VxAttrDesc {
                std::string name;
                GLint location;
                GLenum type;
            };
            struct UniformDesc {
                std::string name;
                GLint location;
                GLenum type;
            };

            bool loadShaderFromSource(std::string const &shader_source,
                                      GLuint const shader_handle);

            bool getAttributes();
            bool getUniforms();

            std::string m_desc;
            std::string const m_source_vsh;
            std::string const m_source_fsh;
            std::string m_glsl_version;

            GLuint m_handle_prog;
            GLuint m_handle_vsh;
            GLuint m_handle_fsh;
            bool m_init;

            std::string m_log_prefix;

            // attribute/uniform binding indexes by name
            uint m_impl_max_attrs;
            std::vector<VxAttrDesc>  m_list_attributes;
            std::vector<UniformDesc> m_list_uniforms;

            // list of which vertex attribute locations
            // are used in this shader
            std::vector<bool> m_list_attribs_used;
        };
    } // gl
} // ks

#endif // KS_GL_SHADER_PROGRAM_HPP
