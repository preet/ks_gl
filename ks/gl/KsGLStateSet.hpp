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


#ifndef KS_GL_STATE_SET_HPP
#define KS_GL_STATE_SET_HPP

// stl
#include <vector>
#include <unordered_set>

// ks
#include <ks/KsMiscUtils.hpp>
#include <ks/gl/KsGLDebug.hpp>

namespace ks
{
    namespace gl
    {
        class StateSet
        {
        public:
            template<typename T>
            struct State {
                bool valid{false};
                T value;
            };

            void CaptureState();

            void SetStateInvalid();

            void SetFrameBuffer(GLint fb_handle);

            void SetVertexAttributeEnabled(GLuint location,bool enabled);

            void SetActiveTexUnitAndBind(GLint unit,GLint handle,GLenum target,u64 uid);

            // blending
            void SetBlend(GLboolean enabled);
            void SetBlendFunction(GLenum srcRGB,GLenum dstRGB,GLenum srcAlpha,GLenum dstAlpha);
            void SetBlendEquation(GLenum modeRGB,GLenum modeAlpha);

            // depth
            void SetDepthTest(GLboolean enabled);
            void SetDepthMask(GLboolean enabled);
            void SetDepthFunction(GLenum func);
            void SetDepthRange(GLfloat near,GLfloat far);

            // stencil
            void SetStencilTest(GLboolean enabled);
            void SetStencilMask(GLenum face,GLuint mask);
            void SetStencilFunction(GLenum face,GLenum func,GLint ref,GLuint mask);
            void SetStencilOperation(GLenum face,GLenum sfail,GLenum dpfail,GLenum dppass);

            // culling
            void SetFaceCulling(GLboolean enabled);
            void SetFaceCullingMode(GLenum mode);

            // texture pixel packing
            void SetPixelUnpackAlignment(GLint alignment);
            void SetPixelPackAlignment(GLint alignment);

            // polygon offset
            void SetPolygonOffsetFill(GLboolean enabled);
            void SetPolygonOffset(GLfloat factor,GLfloat units);

            // clear
            void SetClearColor(GLfloat r,GLfloat g,GLfloat b,GLfloat a);
            void SetClearDepth(GLfloat depth);
            void SetClearStencil(GLint s);

            // ============================================================= //

            // Accessors
            // * can be called outside an active OpenGL context
            GLint GetPixelUnpackAlignment() const
            {
                return m_data.gl_unpack_alignment.value;
            }

            GLint GetPixelPackAlignment() const
            {
                return m_data.gl_pack_alignment.value;
            }

            State<GLint> GetCurrentFramebuffer() const
            {
                return m_data.gl_framebuffer_binding;
            }

            // ============================================================= //

        private:
            template<typename T>
            void setState(State<T> &state, T value)
            {
                state.valid = true;
                state.value = value;
            }

            template<typename T>
            bool compareState(State<T> &state, T value)
            {
                return (state.valid && (state.value == value));
            }

            struct Color
            {
                GLfloat r;
                GLfloat g;
                GLfloat b;
                GLfloat a;

                bool operator == (Color const &other) {
                    return (r == other.r &&
                            g == other.g &&
                            b == other.b &&
                            a == other.a);
                }
            };


            std::string m_log_prefix{"StateSet: "};

            // ============================================================= //

            // active state
            struct Data
            {
                State<GLint> gl_framebuffer_binding;

                // ============================================================= //

                std::vector<State<bool>> gl_vertex_attrib_array_enabled;

                // ============================================================= //

                // currently active texture unit
                State<GLint> gl_active_texture;

                // map of which texture ids are bound to
                // which texture unit

                // list_texture_bindstates
                // * List of texture units and which textures
                //   are bound to them. Textures are identified
                //   by 'uid' which is automatically generated
                //   with the ks::Texture class. These uids
                //   are NOT OpenGL texture handles.
                // * The list index denotes the corresponding
                //   texture unit; list_[0] == texture unit 0
                struct TextureBindingState {
                    bool valid;
                    uint64_t uid; // this is NOT an opengl texture handle
                    TextureBindingState() :
                        valid(false) {}
                };
                std::vector<TextureBindingState> list_texture_bindstates;

                // ============================================================= //

                // (blend)
                State<GLboolean> gl_blend;
                // (function)
                State<GLenum> gl_blend_src_rgb;
                State<GLenum> gl_blend_src_alpha;
                State<GLenum> gl_blend_dst_rgb;
                State<GLenum> gl_blend_dst_alpha;
                // (equation)
                State<GLenum> gl_blend_equation_rgb;
                State<GLenum> gl_blend_equation_alpha;

                // ============================================================= //

                // (test)
                State<GLboolean> gl_depth_test;
                // (mask)
                State<GLboolean> gl_depth_writemask;
                // (func)
                State<GLenum> gl_depth_func;
                // (range)
                State<GLfloat> gl_depth_range_near;
                State<GLfloat> gl_depth_range_far;

                // ============================================================= //

                // (mask)
                State<GLuint> gl_stencil_writemask;
                State<GLuint> gl_stencil_back_writemask;
                // (function)
                State<GLboolean> gl_stencil_test;
                State<GLenum> gl_stencil_func;
                State<GLuint> gl_stencil_value_mask;
                State<GLint> gl_stencil_ref;
                State<GLenum> gl_stencil_back_func;
                State<GLuint> gl_stencil_back_value_mask;
                State<GLint> gl_stencil_back_ref;
                // (ops)
                State<GLenum> gl_stencil_fail;
                State<GLenum> gl_stencil_pass_depth_pass;
                State<GLenum> gl_stencil_pass_depth_fail;
                State<GLenum> gl_stencil_back_fail;
                State<GLenum> gl_stencil_back_pass_depth_pass;
                State<GLenum> gl_stencil_back_pass_depth_fail;

                // ============================================================= //

                State<GLboolean> gl_cull_face;
                State<GLenum> gl_cull_face_mode;

                // ============================================================= //

                // GL_PACK_ALIGNMENT when data is read back (glReadPixels)
                // GL_UNPACK_ALIGNMENT when data is uploaded (ie glTexImage2D)
                State<GLint> gl_pack_alignment;
                State<GLint> gl_unpack_alignment;

                // ============================================================= //

                State<GLboolean> gl_polygon_offset_fill;
                State<GLfloat> gl_polygon_offset_factor;
                State<GLfloat> gl_polygon_offset_units;

                // ============================================================= //

                State<Color> gl_color_clear_value;
                State<GLfloat> gl_depth_clear_value;
                State<GLint> gl_stencil_clear_value;
            };

            Data m_data;
        };
    }
} // namespace ks

#endif // KS_GL_STATE_SET_HPP
