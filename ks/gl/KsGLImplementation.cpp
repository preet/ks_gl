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

#include <ks/KsLog.hpp>
#include <ks/gl/KsGLImplementation.hpp>
#include <mutex>

namespace ks
{
    namespace gl
    {
        namespace Implementation
        {
            namespace {
                std::mutex g_gl_mutex;

                // Implementation Dependent Values (p.152 GL ES 2 spec)
                bool g_gl_valid{false};
                std::string g_gl_vendor;
                std::string g_gl_renderer;
                std::string g_gl_version;
                std::string g_gl_shading_language_version;
                std::unordered_set<std::string> g_gl_extensions;

                GLint g_gl_max_texture_size;
                GLint g_gl_max_cube_map_texture_size;
                GLint g_gl_max_vertex_attribs;
                GLint g_gl_max_vertex_uniform_vectors;
                GLint g_gl_max_varying_vectors;
                GLint g_gl_max_combined_texture_image_units;
                GLint g_gl_max_vertex_texture_image_units;
                GLint g_gl_max_texture_image_units;
                GLint g_gl_max_fragment_uniform_vectors;
                GLint g_gl_max_renderbuffer_size;

                std::string g_log_prefix{"gl: Implementation: "};
            }

            void GLCapture()
            {
                std::lock_guard<std::mutex> lock(g_gl_mutex);

                if(g_gl_valid) {
                    return;
                }

                LOG.Info() << g_log_prefix << "Capturing Info...";

                g_gl_vendor.assign(reinterpret_cast<const char*>(
                                       glGetString(GL_VENDOR)));

                g_gl_renderer.assign(reinterpret_cast<const char*>(
                                       glGetString(GL_RENDERER)));

                g_gl_version.assign(reinterpret_cast<const char*>(
                                       glGetString(GL_VERSION)));

                g_gl_shading_language_version.assign(reinterpret_cast<const char*>(
                                       glGetString(GL_SHADING_LANGUAGE_VERSION)));

                // extensions are returned in a space separated list
                std::string list_extensions(
                            reinterpret_cast<const char*>(
                                glGetString(GL_EXTENSIONS)));

                std::stringstream ss(list_extensions);
                std::string extension;
                while(std::getline(ss,extension,' ')) {
                    g_gl_extensions.insert(extension);
                }

                glGetIntegerv(GL_MAX_TEXTURE_SIZE,&g_gl_max_texture_size);
                glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE,&g_gl_max_cube_map_texture_size);
                glGetIntegerv(GL_MAX_VERTEX_ATTRIBS,&g_gl_max_vertex_attribs);
                glGetIntegerv(GL_MAX_VERTEX_UNIFORM_VECTORS,&g_gl_max_vertex_uniform_vectors);
                glGetIntegerv(GL_MAX_VARYING_VECTORS,&g_gl_max_varying_vectors);
                glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS,&g_gl_max_combined_texture_image_units);
                glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS,&g_gl_max_vertex_texture_image_units);
                glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS,&g_gl_max_texture_image_units);
                glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_VECTORS,&g_gl_max_fragment_uniform_vectors);
                glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE,&g_gl_max_renderbuffer_size);

                KS_CHECK_GL_ERROR(g_log_prefix+"capture implementation info");

                LOG.Info() << g_log_prefix << g_gl_vendor;
                LOG.Info() << g_log_prefix << g_gl_renderer;
                LOG.Info() << g_log_prefix << g_gl_version;
                LOG.Info() << g_log_prefix << g_gl_shading_language_version;

                g_gl_valid = true;
            }

            std::string const & GetGLVendor()
            {
                std::lock_guard<std::mutex> lock(g_gl_mutex);
                return g_gl_vendor;
            }

            std::string const & GetGLRenderer()
            {
                std::lock_guard<std::mutex> lock(g_gl_mutex);
                return g_gl_renderer;
            }

            std::string const & GetGLVersion()
            {
                std::lock_guard<std::mutex> lock(g_gl_mutex);
                return g_gl_version;
            }

            std::string const & GetGLSLVersion()
            {
                std::lock_guard<std::mutex> lock(g_gl_mutex);
                return g_gl_shading_language_version;
            }

            bool GetGLExtensionExists(std::string const &ext)
            {
                std::lock_guard<std::mutex> lock(g_gl_mutex);
                return (g_gl_extensions.find(ext) != g_gl_extensions.end());
            }

            GLint GetMaxTextureSize()
            {
                std::lock_guard<std::mutex> lock(g_gl_mutex);
                return g_gl_max_texture_size;
            }

            GLint GetMaxCubeMapTextureSize()
            {
                std::lock_guard<std::mutex> lock(g_gl_mutex);
                return g_gl_max_cube_map_texture_size;
            }

            GLint GetMaxVertexAttribs()
            {
                std::lock_guard<std::mutex> lock(g_gl_mutex);
                return g_gl_max_vertex_attribs;
            }

            GLint GetMaxVertexUniformVectors()
            {
                std::lock_guard<std::mutex> lock(g_gl_mutex);
                return g_gl_max_vertex_uniform_vectors;
            }

            GLint GetMaxVaryingVectors()
            {
                std::lock_guard<std::mutex> lock(g_gl_mutex);
                return g_gl_max_varying_vectors;
            }

            GLint GetMaxCombinedTextureImageUnits()
            {
                std::lock_guard<std::mutex> lock(g_gl_mutex);
                return g_gl_max_combined_texture_image_units;
            }

            GLint GetMaxVertexTextureImageUnits()
            {
                std::lock_guard<std::mutex> lock(g_gl_mutex);
                return g_gl_max_vertex_texture_image_units;
            }

            GLint GetMaxTextureImageUnits()
            {
                std::lock_guard<std::mutex> lock(g_gl_mutex);
                return g_gl_max_texture_image_units;
            }

            GLint GetMaxFragmentUniformVectors()
            {
                std::lock_guard<std::mutex> lock(g_gl_mutex);
                return g_gl_max_fragment_uniform_vectors;
            }

            GLint GetMaxRenderBufferSize()
            {
                std::lock_guard<std::mutex> lock(g_gl_mutex);
                return g_gl_max_renderbuffer_size;
            }
        }
    }
}
