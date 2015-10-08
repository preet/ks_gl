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

// stl
#include <algorithm>

// ks
#include <ks/gl/KsGLImplementation.hpp>
#include <ks/gl/KsGLShaderProgram.hpp>

namespace ks
{
    namespace gl
    {
        ShaderProgram::ShaderProgram(std::string source_vsh,
                                     std::string source_fsh,
                                     std::string glsl_version) :
            m_source_vsh(std::move(source_vsh)),
            m_source_fsh(std::move(source_fsh)),
            m_glsl_version(std::move(glsl_version)),
            m_handle_prog(0),
            m_handle_vsh(0),
            m_handle_fsh(0),
            m_init(false),
            m_impl_max_attrs(false)
        {
            m_log_prefix = "ShaderProgram: ";

            if(m_glsl_version.empty()) {
                #ifdef KS_ENV_GL_ES
                    m_glsl_version = std::string("#version 100\n");
                #elif KS_ENV_GL_DESKTOP
                    m_glsl_version = std::string("#version 120\n");
                #else
                    #error KS_ENV_GL version unspecified (ES or DESKTOP)
                #endif
            }
        }

        ShaderProgram::~ShaderProgram()
        {
            // empty
        }

        GLint ShaderProgram::GetAttributeLocation(std::string const &name) const
        {
            auto it = std::find_if(
                        m_list_attributes.begin(),
                        m_list_attributes.end(),
                        [&name](VxAttrDesc const &attr) -> bool {
                            return(attr.name == name);
                        });

            return (it != m_list_attributes.end()) ? it->location : -1;
        }

        GLint ShaderProgram::GetUniformLocation(std::string const &name) const
        {
            auto it = std::find_if(
                        m_list_uniforms.begin(),
                        m_list_uniforms.end(),
                        [&name](UniformDesc const &uniform) -> bool {
                            return(uniform.name == name);
                        });

            return (it != m_list_uniforms.end()) ? it->location : -1;
        }

        std::string const & ShaderProgram::GetDesc() const
        {
            return m_desc;
        }

        GLuint ShaderProgram::GetHandle() const
        {
            return m_handle_prog;
        }

        bool ShaderProgram::IsInit() const
        {
            return m_init;
        }

        void ShaderProgram::SetDesc(std::string desc)
        {
            m_desc = std::move(desc);
            m_log_prefix = "ShaderProgram: " +m_desc+": ";
        }

        void ShaderProgram::PrintAttributes() const
        {
            LOG.Info() << m_log_prefix << "Attributes:";
            for(auto &attr : m_list_attributes) {
                LOG.Info() << m_log_prefix << attr.name << ": " << attr.location;
            }
        }

        void ShaderProgram::PrintUniforms() const
        {
            LOG.Info() << m_log_prefix << "Uniforms:";
            for(auto &unif : m_list_uniforms) {
                LOG.Info() << m_log_prefix << unif.name << ": " << unif.location;
            }
        }

        bool ShaderProgram::GLInit()
        {
            if(m_init) {
                LOG.Error() << m_log_prefix << "already init!";
                return false;
            }

            // Get the max number of vertex attributes
            // supported by this GL implementation
            m_impl_max_attrs = Implementation::GetMaxVertexAttribs();

            // create vertex shader object
            m_handle_vsh = glCreateShader(GL_VERTEX_SHADER);
            if(m_handle_vsh == 0) {
                // this error may indicate the gl context
                // hasn't been made available yet
                LOG.Error() << m_log_prefix
                           << "could not create vertex shader object";
                return false;
            }
            // compile vertex shader
            if(!loadShaderFromSource(m_source_vsh,m_handle_vsh)) {
                return false;
            }

            // create fragment shader object
            m_handle_fsh = glCreateShader(GL_FRAGMENT_SHADER);
            if(m_handle_fsh == 0) {
                // this error may indicate the gl context
                // hasn't been made available yet
                LOG.Error() << m_log_prefix
                           << "could not create fragment shader object";
                return false;
            }
            // compile fragment shader
            if(!loadShaderFromSource(m_source_fsh,m_handle_fsh)) {
                return false;
            }

            // create the shader program
            m_handle_prog = glCreateProgram();
            if(m_handle_prog==0) {
                LOG.Error() << m_log_prefix
                           << "failed to create program";
                return false;
            }

            // attach vertex and fragment shaders
            glAttachShader(m_handle_prog,m_handle_vsh);
            KS_CHECK_GL_ERROR(m_log_prefix+"attach vert shader");

            glAttachShader(m_handle_prog,m_handle_fsh);
            KS_CHECK_GL_ERROR(m_log_prefix+"attach frag shader");

            // link
            glLinkProgram(m_handle_prog);
            KS_CHECK_GL_ERROR(m_log_prefix+"link");

            // check link status
            GLint link_status;
            glGetProgramiv(m_handle_prog,GL_LINK_STATUS,&link_status);
            if(link_status == GL_FALSE) {
                // check why link failed
                std::string err_msg;
                GLint info_log_length;
                glGetProgramiv(m_handle_prog,GL_INFO_LOG_LENGTH,&info_log_length);
                if(info_log_length==0) {
                    err_msg = "no info log avail";
                }
                else {
                    std::vector<char> info_log(info_log_length);
                    glGetProgramInfoLog(m_handle_prog,info_log_length,NULL,&(info_log[0]));
                    err_msg = std::string(&(info_log[0]));
                }
                LOG.Error() << m_log_prefix
                           << "failed to link: " << err_msg;

                glDeleteProgram(m_handle_prog);
                KS_CHECK_GL_ERROR(m_log_prefix+"delete after failed link");
                return false;
            }

            // get the list of attribute and uniform locations
            // defined in the shader
            if(!(this->getAttributes()) || !(this->getUniforms())) {
                return false;
            }

            // detach and delete the vertex and fragment
            // shader objects from the program

            // WARN:
            // Usually you should detach and delete shaders
            // once they have been linked to a shader program,
            // but some drivers (Nvidia tegra, apparently?)
            // create issues when this is done. If this define
            // isn't set, shaders will stay attached to their
            // shader program until glDeleteProgram is called.
            // Attached shaders may take up some additional
            // memory on the gpu.

            // If we don't detach/delete here, its done during
            // the GLCleanUp() call instead.

            // refs:
            // http://code.google.com/p/android/issues/detail?id=61832
            // http://stackoverflow.com/questions/9113154/proper-way-to-delete-glsl-shader

            #ifdef KS_ENV_GL_QUICK_DELETE_SHADERS
                glDetachShader(m_handle_prog,m_handle_vsh);
                glDetachShader(m_handle_prog,m_handle_fsh);
                KS_CHECK_GL_ERROR(m_log_prefix+"detach shaders");

                glDeleteShader(m_handle_vsh);
                glDeleteShader(m_handle_fsh);
                KS_CHECK_GL_ERROR(m_log_prefix+"delete shaders");

                m_handle_vsh=0;
                m_handle_fsh=0;
            #endif

            //
            m_init = true;
            return true;
        }

        void ShaderProgram::GLEnable(StateSet * state_set)
        {
            glUseProgram(m_handle_prog);
            KS_CHECK_GL_ERROR(m_log_prefix+"enable");

            // enable associated vertex attribute arrays
            // and update the state set accordingly
            for(size_t i=0; i < m_list_attribs_used.size(); i++) {
                state_set->SetVertexAttributeEnabled(i,m_list_attribs_used[i]);
            }
        }

        void ShaderProgram::GLDisable()
        {
            // Might not be a good idea to call this unnecessarily;
            // ref: http://stackoverflow.com/questions/13546461/what-does-gluseprogram0-do
            #ifdef KS_DEBUG_GL
            LOG.Warn() << m_log_prefix << "called disable";
            #endif

            glUseProgram(0);
            KS_CHECK_GL_ERROR(m_log_prefix+"disable");
        }

        void ShaderProgram::GLCleanUp()
        {
            // explicitly mark the shader objects associated
            // with this program for deletion if they haven't
            // already been done so -- they will be cleaned
            // cleaned up when glDeleteProgram is called
            if(m_handle_vsh && m_handle_fsh) {
                glDeleteShader(m_handle_vsh);
                glDeleteShader(m_handle_fsh);
                KS_CHECK_GL_ERROR(m_log_prefix+"delete shaders");
            }

            // delete the shader program
            glDeleteProgram(m_handle_prog);
            KS_CHECK_GL_ERROR(m_log_prefix+"delete shader prog");

            m_init = false;
        }

        // ============================================================= //

        void ShaderProgram::GLSetUniform(std::string const &name,GLint i)
        {
            glUniform1iv(GetUniformLocation(name),1,&i);
            KS_CHECK_GL_ERROR(m_log_prefix+"set uniform1iv "+name);
        }

        void ShaderProgram::GLSetUniform(std::string const &name,GLfloat f)
        {
            glUniform1fv(GetUniformLocation(name),1,&f);
//            glUniform1f(GetUniformLocation(name),f);
            KS_CHECK_GL_ERROR(m_log_prefix+"set uniform1fv "+name);
        }

        void ShaderProgram::GLSetUniform(std::string const &name,glm::vec2 const &v2)
        {
            glUniform2fv(GetUniformLocation(name),1,glm::value_ptr(v2));
            KS_CHECK_GL_ERROR(m_log_prefix+"set uniform2fv "+name);
        }

        void ShaderProgram::GLSetUniform(std::string const &name,glm::vec3 const &v3)
        {
            glUniform3fv(GetUniformLocation(name),1,glm::value_ptr(v3));
            KS_CHECK_GL_ERROR(m_log_prefix+"set uniform3fv "+name);
        }

        void ShaderProgram::GLSetUniform(std::string const &name,glm::vec4 const &v4)
        {
            glUniform4fv(GetUniformLocation(name),1,glm::value_ptr(v4));
            KS_CHECK_GL_ERROR(m_log_prefix+"set uniform4fv "+name);
        }

        void ShaderProgram::GLSetUniform(std::string const &name,glm::mat4 const &m4)
        {
            glUniformMatrix4fv(GetUniformLocation(name),1,false,glm::value_ptr(m4));
            KS_CHECK_GL_ERROR(m_log_prefix+"set uniform mat4fv "+name);
        }



        void ShaderProgram::GLSetUniform(std::string const &name,std::vector<GLint> const &ia)
        {
            glUniform1iv(GetUniformLocation(name),ia.size(),&(ia[0]));
            KS_CHECK_GL_ERROR(m_log_prefix+"set uniform1iv array "+name);
        }

        void ShaderProgram::GLSetUniform(std::string const &name,std::vector<GLfloat> const &fa)
        {
            glUniform1fv(GetUniformLocation(name),fa.size(),&(fa[0]));
            KS_CHECK_GL_ERROR(m_log_prefix+"set uniform1fv array "+name);
        }

        void ShaderProgram::GLSetUniform(std::string const &name,std::vector<glm::vec2> const &v2a)
        {
            glUniform2fv(GetUniformLocation(name),v2a.size(),glm::value_ptr(v2a[0]));
            KS_CHECK_GL_ERROR(m_log_prefix+"set uniform2fv array "+name);
        }

        void ShaderProgram::GLSetUniform(std::string const &name,std::vector<glm::vec3> const &v3a)
        {
            glUniform3fv(GetUniformLocation(name),v3a.size(),glm::value_ptr(v3a[0]));
            KS_CHECK_GL_ERROR(m_log_prefix+"set uniform3fv array "+name);
        }

        void ShaderProgram::GLSetUniform(std::string const &name,std::vector<glm::vec4> const &v4a)
        {
            glUniform4fv(GetUniformLocation(name),v4a.size(),glm::value_ptr(v4a[0]));
            KS_CHECK_GL_ERROR(m_log_prefix+"set uniform4fv array "+name);
        }

        void ShaderProgram::GLSetUniform(std::string const &name,std::vector<glm::mat4> const &m4a)
        {
            glUniformMatrix4fv(GetUniformLocation(name),m4a.size(),false,glm::value_ptr(m4a[0]));
            KS_CHECK_GL_ERROR(m_log_prefix+"set uniform mat4fv array"+name);
        }

        // ============================================================= //

        bool ShaderProgram::loadShaderFromSource(std::string const &shader_source,
                                                 GLuint const shader_handle)
        {
            // add version to source
            std::string source = shader_source;
            source.insert(0,m_glsl_version);

            // set the GLSL source code for this shader
            char const * source_data = source.c_str();
            glShaderSource(shader_handle,1,&source_data,NULL);
            KS_CHECK_GL_ERROR(m_log_prefix+"set glsl source");

            // compile
            glCompileShader(shader_handle);
            KS_CHECK_GL_ERROR(m_log_prefix+"compile glsl");

            // check compile status
            GLint compile_status;
            glGetShaderiv(shader_handle,GL_COMPILE_STATUS,&compile_status);
            if(compile_status == GL_FALSE) {
                // find out why compilation failed
                std::string err_msg;
                GLint info_log_length;
                glGetShaderiv(shader_handle,GL_INFO_LOG_LENGTH,&info_log_length);
                if(info_log_length==0) {
                    // NOTE: DRIVER BUG: Adreno
                    // Adreno drivers may always return an INFO LOG LENGTH
                    // of 0 regardless of an actual log being available, so
                    // try querying anyway:
                    info_log_length = 4096;
                }

                GLsizei bytes_written=0;
                std::vector<char> info_log(info_log_length);
                glGetShaderInfoLog(shader_handle,
                                   info_log_length,
                                   &bytes_written,
                                   &(info_log[0]));

                err_msg = std::string(&(info_log[0]));

                LOG.Error() << m_log_prefix
                          << "Failed to compile glsl: log size: " << bytes_written;

                LOG.Error() << m_log_prefix
                          << "Failed to compile glsl: LOG: \n" << err_msg;

                assert(compile_status != GL_FALSE);

                glDeleteShader(shader_handle);
                KS_CHECK_GL_ERROR(m_log_prefix+"delete after failed load");
                return false;
            }

            return true;
        }

        bool ShaderProgram::getAttributes()
        {
            // get the number of active attributes
            GLint attrib_count;
            glGetProgramiv(m_handle_prog,
                           GL_ACTIVE_ATTRIBUTES,
                           &attrib_count);

            // get attribute max name length
            GLint max_attrib_length;
            glGetProgramiv(m_handle_prog,
                           GL_ACTIVE_ATTRIBUTE_MAX_LENGTH,
                           &max_attrib_length);

            // get attribute location, name and type
            for(GLint i=0; i < attrib_count; i++) {
                GLsizei attrib_length;
                GLint attrib_sz;
                GLenum attrib_type;
                std::vector<char> attrib_name(max_attrib_length);

                // get attrib desc
                glGetActiveAttrib(m_handle_prog,i,
                                  max_attrib_length,
                                  &attrib_length,
                                  &attrib_sz,
                                  &attrib_type,
                                  &(attrib_name[0]));

                std::string name(&(attrib_name[0]),attrib_length);

                // get attrib location
                // NOTE: This is *NOT* the same as 'i'; the
                // index passed to glGetActiveAttrib and the
                // location associated with said index are different
                GLint attrib_loc = glGetAttribLocation(m_handle_prog,
                                                       name.c_str());

                if(attrib_loc < 0) {
                    LOG.Error() << m_log_prefix
                               << "failed to find location for "
                                  "attribute: " << name;
                    return false;
                }

                m_list_attributes.push_back(
                    {name,attrib_loc,attrib_type});
            }
            KS_CHECK_GL_ERROR(m_log_prefix+"get attributes");

            // save used attrib locations for easy lookup
            m_list_attribs_used.clear();
            m_list_attribs_used.resize(m_impl_max_attrs,false);

            for(auto& attr : m_list_attributes) {
                m_list_attribs_used[attr.location] = true;
            }

            return true;
        }

        bool ShaderProgram::getUniforms()
        {
            // get the number of active uniforms
            GLint unif_count;
            glGetProgramiv(m_handle_prog,
                           GL_ACTIVE_UNIFORMS,
                           &unif_count);

            // get uniform max name length
            GLint max_unif_length;
            glGetProgramiv(m_handle_prog,
                           GL_ACTIVE_UNIFORM_MAX_LENGTH,
                           &max_unif_length);

            // get uniform location, name and type
            for(GLint i=0; i < unif_count; i++) {
                GLsizei unif_length;
                GLint unif_sz;
                GLenum unif_type;
                std::vector<char> unif_name(max_unif_length);

                glGetActiveUniform(m_handle_prog,i,
                                   max_unif_length,
                                   &unif_length,
                                   &unif_sz,
                                   &unif_type,
                                   &(unif_name[0]));

                std::string name(&(unif_name[0]),unif_length);

                // If this is a single uniform
                if(unif_sz == 1) {
                    // Get uniform location
                    // NOTE: This is *NOT* the same as 'i'; the
                    // index passed to glGetActiveUniform and the
                    // location associated with said index are different
                    GLint uniform_loc = glGetUniformLocation(m_handle_prog,
                                                             name.c_str());

                    if(uniform_loc < 0) {
                        LOG.Error() << m_log_prefix
                                   << "failed to find location for "
                                      "uniform: " << name;
                        return false;
                    }

                    m_list_uniforms.push_back(
                        {name,uniform_loc,unif_type});
                }
                else {
                    // If this uniform is an array, save all of its
                    // possible locations by individual index name
                    // TODO not sure if this is a good idea esp with
                    // lots of potential indices

                    // There is a potential device-specific bug here in how
                    // names for uniform arrays are defined.

                    // http://forum.imgtec.com/discussion/2290/potential-bug-in-glgetactiveuniform
                    // https://www.opengl.org/discussion_boards/showthread.php/181286-Uniform-array-names
                    // http://www.leadwerks.com/werkspace/blog/1/entry-812-madness-this-is-mobile/
                    // http://www.drawelements.com/uploads/news/de-market-analysis-2013__free_version.pdf

                    // Regardless of what name glGetActiveUniform returned,
                    // (according to the GLES2 spec p36,67 it should be the
                    // name with [0] appended), we manually verify which variant
                    // for the name is valid for the FIRST element:
                    //
                    // "uniform_array_name[0]"
                    // -or-
                    // "uniform_array_name"

                    std::string name_full;
                    std::string name_base;

                    size_t const arr_op_idx = name.find("[0]",0);
                    if(arr_op_idx == std::string::npos) {
                        name_base = name;
                        name_full = name_base + "[0]";
                    }
                    else {
                        name_base = name.substr(0,arr_op_idx);
                        name_full = name;
                    }

                    // Test "uniform_array_name[0]" first:
                    GLint first_loc = -1;
                    first_loc = glGetUniformLocation(m_handle_prog,
                                                     name_full.c_str());

                    if(first_loc < 0) {
                        // try without the array operator
                        first_loc = glGetUniformLocation(m_handle_prog,
                                                         name_base.c_str());

                        if(first_loc < 0) { // nothing we can do if this fails
                            LOG.Error() << m_log_prefix
                                      << "failed to find location for uniform array,"
                                         "tried: " << name_full << " and " << name_base;
                            return false;
                        }
                    }

                    // save the first location under both names
                    // for convenience
                    {
                        m_list_uniforms.push_back(
                            {name_full,first_loc,unif_type});

                        m_list_uniforms.push_back(
                            {name_base,first_loc,unif_type});
                    }

                    // save the rest of the indices
                    for(GLint n=1; n < unif_sz; n++) {
                        std::string name_index = name_base + "[" +
                                ConvNumberToString(n) +"]";

                        GLint uniform_loc = glGetUniformLocation(m_handle_prog,
                                                                 name_index.c_str());

                        if(uniform_loc < 0) {
                            LOG.Error() << m_log_prefix
                                       << "failed to find location for "
                                          "uniform: " << name_index;
                            return false;
                        }

                        m_list_uniforms.push_back(
                            {name_index,uniform_loc,unif_type});
                    }
                }
            }
            KS_CHECK_GL_ERROR(m_log_prefix+"get uniforms");

            return true;
        }
    } // gl
} // ks
