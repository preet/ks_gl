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


#include <sstream>
#include <ks/gl/KsGLStateSet.hpp>
#include <ks/gl/KsGLImplementation.hpp>

namespace ks
{
    namespace gl
    {
        // Helpers for capturing state from the GL context
        namespace {

            template<typename T>
            void assignBooleanFromGL(StateSet::State<T> &state,GLenum gl_enum)
            {
                GLboolean temp;
                glGetBooleanv(gl_enum,&temp);
                state.valid = true;
                state.value = temp;
            }

            template<typename T>
            void assignIntegerFromGL(StateSet::State<T> &state, GLenum gl_enum)
            {
                GLint temp;
                glGetIntegerv(gl_enum,&temp);
                state.valid = true;
                state.value = temp;
            }

            template<typename T>
            void assignFloatFromGL(StateSet::State<T> &state, GLenum gl_enum)
            {
                GLfloat temp;
                glGetFloatv(gl_enum,&temp);
                state.valid = true;
                state.value = temp;
            }
        }

        void StateSet::CaptureState()
        {
            m_data.gl_vertex_attrib_array_enabled.resize(
                        Implementation::GetMaxVertexAttribs());

            m_data.list_texture_bindstates.resize(
                        Implementation::GetMaxTextureImageUnits());

            // blend
            assignBooleanFromGL(m_data.gl_blend,GL_BLEND);
            assignIntegerFromGL(m_data.gl_blend_src_rgb,GL_BLEND_SRC_RGB);
            assignIntegerFromGL(m_data.gl_blend_src_alpha,GL_BLEND_SRC_ALPHA);
            assignIntegerFromGL(m_data.gl_blend_dst_rgb,GL_BLEND_DST_RGB);
            assignIntegerFromGL(m_data.gl_blend_dst_alpha,GL_BLEND_DST_ALPHA);
            assignIntegerFromGL(m_data.gl_blend_equation_rgb,GL_BLEND_EQUATION_RGB);
            assignIntegerFromGL(m_data.gl_blend_equation_alpha,GL_BLEND_EQUATION_ALPHA);

            // depth
            assignBooleanFromGL(m_data.gl_depth_test,GL_DEPTH_TEST);
            assignBooleanFromGL(m_data.gl_depth_writemask,GL_DEPTH_WRITEMASK);
            assignIntegerFromGL(m_data.gl_depth_func,GL_DEPTH_FUNC);

            GLfloat depth_range[2];
            glGetFloatv(GL_DEPTH_RANGE,&(depth_range[0]));
            setState(m_data.gl_depth_range_near,depth_range[0]);
            setState(m_data.gl_depth_range_near,depth_range[1]);

            // stencil
            assignIntegerFromGL(m_data.gl_stencil_writemask,GL_STENCIL_WRITEMASK);
            assignIntegerFromGL(m_data.gl_stencil_back_writemask,GL_STENCIL_BACK_WRITEMASK);
            assignBooleanFromGL(m_data.gl_stencil_test,GL_STENCIL_TEST);
            assignIntegerFromGL(m_data.gl_stencil_func,GL_STENCIL_FUNC);
            assignIntegerFromGL(m_data.gl_stencil_value_mask,GL_STENCIL_VALUE_MASK);
            assignIntegerFromGL(m_data.gl_stencil_ref,GL_STENCIL_REF);
            assignIntegerFromGL(m_data.gl_stencil_back_func,GL_STENCIL_BACK_FUNC);
            assignIntegerFromGL(m_data.gl_stencil_back_value_mask,GL_STENCIL_BACK_VALUE_MASK);
            assignIntegerFromGL(m_data.gl_stencil_back_ref,GL_STENCIL_BACK_REF);

            assignIntegerFromGL(m_data.gl_stencil_fail,GL_STENCIL_FAIL);
            assignIntegerFromGL(m_data.gl_stencil_pass_depth_pass,GL_STENCIL_PASS_DEPTH_PASS);
            assignIntegerFromGL(m_data.gl_stencil_pass_depth_fail,GL_STENCIL_PASS_DEPTH_FAIL);
            assignIntegerFromGL(m_data.gl_stencil_back_fail,GL_STENCIL_BACK_FAIL);
            assignIntegerFromGL(m_data.gl_stencil_back_pass_depth_pass,GL_STENCIL_BACK_PASS_DEPTH_PASS);
            assignIntegerFromGL(m_data.gl_stencil_back_pass_depth_fail,GL_STENCIL_BACK_PASS_DEPTH_FAIL);

            // cull
            assignBooleanFromGL(m_data.gl_cull_face,GL_CULL_FACE);
            assignIntegerFromGL(m_data.gl_cull_face_mode,GL_CULL_FACE_MODE);

            // texture pack
            assignIntegerFromGL(m_data.gl_pack_alignment,GL_PACK_ALIGNMENT);
            assignIntegerFromGL(m_data.gl_unpack_alignment,GL_UNPACK_ALIGNMENT);

            // polygon offset
            assignBooleanFromGL(m_data.gl_polygon_offset_fill,GL_POLYGON_OFFSET_FILL);
            assignFloatFromGL(m_data.gl_polygon_offset_factor,GL_POLYGON_OFFSET_FACTOR);
            assignFloatFromGL(m_data.gl_polygon_offset_units,GL_POLYGON_OFFSET_UNITS);

            // clear
            GLfloat clc[4];
            glGetFloatv(GL_COLOR_CLEAR_VALUE,&(clc[0]));
            setState(m_data.gl_color_clear_value,Color{clc[0],clc[1],clc[2],clc[3]});

            assignFloatFromGL(m_data.gl_depth_clear_value,GL_DEPTH_CLEAR_VALUE);
            assignIntegerFromGL(m_data.gl_stencil_clear_value,GL_STENCIL_CLEAR_VALUE);

            KS_CHECK_GL_ERROR(m_log_prefix+"capture general state");

            // vertex attributes
            for(size_t i=0; i < m_data.gl_vertex_attrib_array_enabled.size(); i++) {
                GLint is_enabled;
                glGetVertexAttribiv(i,GL_VERTEX_ATTRIB_ARRAY_ENABLED,&is_enabled);
                m_data.gl_vertex_attrib_array_enabled[i].value = (is_enabled == 0) ? false : true;
                m_data.gl_vertex_attrib_array_enabled[i].valid = true;
            }
            KS_CHECK_GL_ERROR(m_log_prefix+"capture enabled vx attribs");

//            LOG.Info() << m_log_prefix << "GL_VERTEX_ATTRIB_ARRAY_ENABLED:";
//            std::string list_vx_attr_enabled;
//            for(size_t i=0; i < m_data.gl_vertex_attrib_array_enabled.size(); i++) {
//                list_vx_attr_enabled.append(
//                            ConvNumberToString(i) +
//                            "(" + ConvBoolToString(
//                                m_data.gl_vertex_attrib_array_enabled[i].value,true) + "), ");
//            }
//            LOG.Info() << m_log_prefix << list_vx_attr_enabled;
        }

        void StateSet::SetStateInvalid()
        {
            m_data = Data();

            // Set implementation specific resource limits
            m_data.gl_vertex_attrib_array_enabled.resize(
                        Implementation::GetMaxVertexAttribs());

            m_data.list_texture_bindstates.resize(
                        Implementation::GetMaxTextureImageUnits());
        }


        // ============================================================= //

        void StateSet::SetFrameBuffer(GLint fb_handle)
        {
            if(compareState(m_data.gl_framebuffer_binding,fb_handle)) {
                return;
            }

            glBindFramebuffer(GL_FRAMEBUFFER,fb_handle);
            KS_CHECK_GL_ERROR(m_log_prefix+"set framebuffer");
            setState(m_data.gl_framebuffer_binding,fb_handle);
        }

        void StateSet::SetVertexAttributeEnabled(GLuint location,bool enabled)
        {
            assert((m_data.gl_vertex_attrib_array_enabled.size() > 0) &&
                   (location < m_data.gl_vertex_attrib_array_enabled.size()));

            if(compareState(m_data.gl_vertex_attrib_array_enabled[location],enabled)) {
                return;
            }

            if(enabled) {
                glEnableVertexAttribArray(location);
            }
            else {
                glDisableVertexAttribArray(location);
            }

            KS_CHECK_GL_ERROR(m_log_prefix+"set vertex attrib enabled: "+
                                   ConvNumberToString(location)+": "+
                                   ConvBoolToString(enabled));

            setState(m_data.gl_vertex_attrib_array_enabled[location],enabled);
        }

        void StateSet::SetActiveTexUnitAndBind(GLint unit,GLint handle,GLenum target,uint64_t uid)
        {
            assert((m_data.list_texture_bindstates.size() > 0) &&
                   (GLuint(unit) < m_data.list_texture_bindstates.size()));

            bool unit_already_set =
                    compareState(m_data.gl_active_texture,unit);

            bool handle_already_bound =
                    (m_data.list_texture_bindstates[unit].valid &&
                     m_data.list_texture_bindstates[unit].uid == uid);

            if(!unit_already_set) {
                glActiveTexture(GL_TEXTURE0 + unit);
                KS_CHECK_GL_ERROR(m_log_prefix+"set active tex unit");
                setState(m_data.gl_active_texture,unit);
            }

            if(!handle_already_bound) {
                glBindTexture(target,handle);
                KS_CHECK_GL_ERROR(m_log_prefix+"bind texture");
                m_data.list_texture_bindstates[unit].uid = uid;
                m_data.list_texture_bindstates[unit].valid = true;
            }
        }

        void StateSet::SetBlend(GLboolean enabled)
        {
            if(compareState(m_data.gl_blend,enabled)) {
                return;
            }

            if(enabled == GL_TRUE) {
                glEnable(GL_BLEND);
            }
            else {
                glDisable(GL_BLEND);
            }
            KS_CHECK_GL_ERROR(m_log_prefix+"set blending");

            setState(m_data.gl_blend,enabled);
        }

        void StateSet::SetBlendFunction(GLenum srcRGB,GLenum dstRGB,GLenum srcAlpha,GLenum dstAlpha)
        {
            bool const same_state =
                    compareState(m_data.gl_blend_src_rgb,srcRGB) &&
                    compareState(m_data.gl_blend_dst_rgb,dstRGB) &&
                    compareState(m_data.gl_blend_src_alpha,srcAlpha) &&
                    compareState(m_data.gl_blend_dst_alpha,dstAlpha);

            if(!same_state) {
                glBlendFuncSeparate(srcRGB,dstRGB,srcAlpha,dstAlpha);
                KS_CHECK_GL_ERROR(m_log_prefix+"set blend func");
                setState(m_data.gl_blend_src_rgb,srcRGB);
                setState(m_data.gl_blend_dst_rgb,dstRGB);
                setState(m_data.gl_blend_src_alpha,srcAlpha);
                setState(m_data.gl_blend_dst_alpha,dstAlpha);
            }
        }

        void StateSet::SetBlendEquation(GLenum modeRGB,GLenum modeAlpha)
        {
            bool const same_state =
                    compareState(m_data.gl_blend_equation_rgb,modeRGB) &&
                    compareState(m_data.gl_blend_equation_alpha,modeAlpha);

            if(!same_state) {
                glBlendEquationSeparate(modeRGB,modeAlpha);
                KS_CHECK_GL_ERROR(m_log_prefix+"set blend equation");
                setState(m_data.gl_blend_equation_rgb,modeRGB);
                setState(m_data.gl_blend_equation_alpha,modeAlpha);
            }
        }


        void StateSet::SetDepthTest(GLboolean enabled)
        {
            if(compareState(m_data.gl_depth_test,enabled)) {
                return;
            }

            if(enabled == GL_TRUE) {
                glEnable(GL_DEPTH_TEST);
            }
            else {
                glDisable(GL_DEPTH_TEST);
            }
            KS_CHECK_GL_ERROR(m_log_prefix+"set depth test");

            setState(m_data.gl_depth_test,enabled);
        }

        void StateSet::SetDepthMask(GLboolean enabled)
        {
            if(compareState(m_data.gl_depth_writemask,enabled)) {
                return;
            }

            glDepthMask(enabled);
            KS_CHECK_GL_ERROR(m_log_prefix+"set depth writemask");

            setState(m_data.gl_depth_writemask,enabled);
        }

        void StateSet::SetDepthFunction(GLenum func)
        {
            if(compareState(m_data.gl_depth_func,func)) {
                return;
            }

            glDepthFunc(func);
            KS_CHECK_GL_ERROR(m_log_prefix+"set depth writemask");

            setState(m_data.gl_depth_func,func);
        }

        void StateSet::SetDepthRange(GLfloat near,GLfloat far)
        {
            bool const same_state =
                    compareState(m_data.gl_depth_range_near,near) &&
                    compareState(m_data.gl_depth_range_far,far);

            if(!same_state) {
                glDepthRangef(near,far);
                KS_CHECK_GL_ERROR(m_log_prefix+"set depth range");
                setState(m_data.gl_depth_range_near,near);
                setState(m_data.gl_depth_range_far,far);
            }
        }

        void StateSet::SetStencilTest(GLboolean status)
        {
            if(compareState(m_data.gl_stencil_test,status)) {
                return;
            }

            if(status == GL_TRUE) {
                glEnable(GL_STENCIL_TEST);
            }
            else {
                glDisable(GL_STENCIL_TEST);
            }
            KS_CHECK_GL_ERROR(m_log_prefix+"set stencil test");

            setState(m_data.gl_stencil_test,status);
        }

        void StateSet::SetStencilMask(GLenum face,GLuint mask)
        {
            bool front  = ((face == GL_FRONT) || (face == GL_FRONT_AND_BACK));
            bool back  = ((face == GL_BACK) || (face == GL_FRONT_AND_BACK));

            if(front) {
                if(!compareState(m_data.gl_stencil_writemask,mask)) {
                    glStencilMaskSeparate(face,mask);
                    KS_CHECK_GL_ERROR(m_log_prefix+"glStencilMaskSeparate front");
                    setState(m_data.gl_stencil_writemask,mask);
                }
            }

            if(back) {
                if(!compareState(m_data.gl_stencil_back_writemask,mask)) {
                    glStencilMaskSeparate(face,mask);
                    KS_CHECK_GL_ERROR(m_log_prefix+"glStencilMaskSeparate front");
                    setState(m_data.gl_stencil_back_writemask,mask);
                }
            }
        }

        void StateSet::SetStencilFunction(GLenum face,GLenum func,GLint ref,GLuint mask)
        {
            bool front  = ((face == GL_FRONT) || (face == GL_FRONT_AND_BACK));
            bool back  = ((face == GL_BACK) || (face == GL_FRONT_AND_BACK));

            if(front) {
                bool const same_state =
                        compareState(m_data.gl_stencil_func,func) &&
                        compareState(m_data.gl_stencil_value_mask,mask) &&
                        compareState(m_data.gl_stencil_ref,ref);

                if(!same_state) {
                    glStencilFuncSeparate(face,func,ref,mask);
                    KS_CHECK_GL_ERROR(m_log_prefix+"glStencilFuncSeparate front");
                    setState(m_data.gl_stencil_func,func);
                    setState(m_data.gl_stencil_value_mask,mask);
                    setState(m_data.gl_stencil_ref,ref);
                }
            }

            if(back) {
                bool const same_state =
                        compareState(m_data.gl_stencil_back_func,func) &&
                        compareState(m_data.gl_stencil_back_value_mask,mask) &&
                        compareState(m_data.gl_stencil_back_ref,ref);

                if(!same_state) {
                    glStencilFuncSeparate(face,func,ref,mask);
                    KS_CHECK_GL_ERROR(m_log_prefix+"glStencilFuncSeparate back");
                    setState(m_data.gl_stencil_back_func,func);
                    setState(m_data.gl_stencil_back_value_mask,mask);
                    setState(m_data.gl_stencil_back_ref,ref);
                }
            }
        }

        void StateSet::SetStencilOperation(GLenum face,GLenum sfail,GLenum dpfail,GLenum dppass)
        {
            bool front  = ((face == GL_FRONT) || (face == GL_FRONT_AND_BACK));
            bool back  = ((face == GL_BACK) || (face == GL_FRONT_AND_BACK));

            if(front) {
                bool const same_state =
                        compareState(m_data.gl_stencil_fail,sfail) &&
                        compareState(m_data.gl_stencil_pass_depth_pass,dppass) &&
                        compareState(m_data.gl_stencil_pass_depth_fail,dpfail);

                if(!same_state) {
                    glStencilOpSeparate(face,sfail,dpfail,dppass);
                    KS_CHECK_GL_ERROR(m_log_prefix+"glStencilOpSeparate front");
                    setState(m_data.gl_stencil_fail,sfail);
                    setState(m_data.gl_stencil_pass_depth_pass,dppass);
                    setState(m_data.gl_stencil_pass_depth_fail,dpfail);
                }
            }

            if(back) {
                bool const same_state =
                        compareState(m_data.gl_stencil_back_fail,sfail) &&
                        compareState(m_data.gl_stencil_back_pass_depth_pass,dppass) &&
                        compareState(m_data.gl_stencil_back_pass_depth_fail,dpfail);

                if(!same_state) {
                    glStencilOpSeparate(face,sfail,dpfail,dppass);
                    KS_CHECK_GL_ERROR(m_log_prefix+"glStencilOpSeparate back");
                    setState(m_data.gl_stencil_back_fail,sfail);
                    setState(m_data.gl_stencil_back_pass_depth_pass,dppass);
                    setState(m_data.gl_stencil_back_pass_depth_fail,dpfail);
                }
            }
        }

        void StateSet::SetFaceCulling(GLboolean enabled)
        {
            if(compareState(m_data.gl_cull_face,enabled)) {
                return;
            }

            if(enabled == GL_TRUE) {
                glEnable(GL_CULL_FACE);
            }
            else {
                glDisable(GL_CULL_FACE);
            }
            KS_CHECK_GL_ERROR(m_log_prefix+"set face culling");

            setState(m_data.gl_cull_face,enabled);
        }

        void StateSet::SetFaceCullingMode(GLenum mode)
        {
            if(compareState(m_data.gl_cull_face_mode,mode)) {
                return;
            }

            glCullFace(mode);
            KS_CHECK_GL_ERROR(m_log_prefix+"set face culling mode");
            setState(m_data.gl_cull_face_mode,mode);
        }

        void StateSet::SetPixelUnpackAlignment(GLint alignment)
        {
            if(compareState(m_data.gl_unpack_alignment,alignment)) {
                return;
            }

            glPixelStorei(GL_UNPACK_ALIGNMENT,alignment);
            KS_CHECK_GL_ERROR(m_log_prefix+"set pixel unpack alignment");
            setState(m_data.gl_unpack_alignment,alignment);
        }

        void StateSet::SetPixelPackAlignment(GLint alignment)
        {
            if(compareState(m_data.gl_pack_alignment,alignment)) {
                return;
            }

            glPixelStorei(GL_PACK_ALIGNMENT,alignment);
            KS_CHECK_GL_ERROR(m_log_prefix+"set pixel pack alignment");
            setState(m_data.gl_pack_alignment,alignment);
        }

        void StateSet::SetPolygonOffsetFill(GLboolean enabled)
        {
            if(compareState(m_data.gl_polygon_offset_fill,enabled)) {
                return;
            }

            if(enabled == GL_TRUE) {
                glEnable(GL_POLYGON_OFFSET_FILL);
            }
            else {
                glDisable(GL_POLYGON_OFFSET_FILL);
            }
            KS_CHECK_GL_ERROR(m_log_prefix+"set polygon offset fill");

            setState(m_data.gl_polygon_offset_fill,enabled);
        }

        void StateSet::SetPolygonOffset(GLfloat factor,GLfloat units)
        {
            bool const same_state =
                    compareState(m_data.gl_polygon_offset_factor,factor) &&
                    compareState(m_data.gl_polygon_offset_units,units);

            if(!same_state) {
                glPolygonOffset(factor,units);
                KS_CHECK_GL_ERROR(m_log_prefix+"set polygon offset");
                setState(m_data.gl_polygon_offset_factor,factor);
                setState(m_data.gl_polygon_offset_units,units);
            }
        }

        void StateSet::SetClearColor(GLfloat r,GLfloat g,GLfloat b,GLfloat a)
        {
            Color color{r,g,b,a};

            if(compareState(m_data.gl_color_clear_value,color)) {
                return;
            }

            glClearColor(r,g,b,a);
            KS_CHECK_GL_ERROR(m_log_prefix+"set clear color");
            setState(m_data.gl_color_clear_value,color);
        }

        void StateSet::SetClearDepth(GLfloat depth)
        {
            if(compareState(m_data.gl_depth_clear_value,depth)) {
                return;
            }

            glClearDepthf(depth);
            KS_CHECK_GL_ERROR(m_log_prefix+"set clear depth");
            setState(m_data.gl_depth_clear_value,depth);
        }

        void StateSet::SetClearStencil(GLint s)
        {
            if(compareState(m_data.gl_stencil_clear_value,s)) {
                return;
            }

            glClearStencil(s);
            KS_CHECK_GL_ERROR(m_log_prefix+"set clear stencil");
            setState(m_data.gl_stencil_clear_value,s);
        }
    }
} // namespace ks
