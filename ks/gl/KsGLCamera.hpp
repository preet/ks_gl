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

#ifndef KS_GL_CAMERA_HPP
#define KS_GL_CAMERA_HPP

// glm
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace ks
{
    namespace gl
    {
        template<typename FP>
        class Camera
        {
            static_assert(std::is_same<FP,float>::value ||
                          std::is_same<FP,double>::value,
                          "FP must be either a float or a double");

        public:

            using vec3 = glm::tvec3<FP,glm::highp>;
            using mat4 = glm::tmat4x4<FP,glm::highp>;

            enum class Mode
            {
                Orthographic,
                Perspective
            };

            Camera() :
                m_mode(Mode::Perspective),
                m_eye(vec3(7,0,0)),
                m_vpt(vec3(0,0,0)),
                m_up(vec3(0,0,1)),
                m_fovy_rads(glm::radians(40.0)),
                m_aspect_ratio(1.33),
                m_left(0),
                m_right(640),
                m_bottom(0),
                m_top(480),
                m_dist_near(0.01),
                m_dist_far(100.0)
            {

            }

            void SetMode(Mode mode)
            {
                m_mode = mode;
            }

            void SetViewMatrixAsLookAt(vec3 const &eye,
                                       vec3 const &vpt,
                                       vec3 const &up)
            {
                m_eye = eye;
                m_vpt = vpt;
                m_up = up;
                rebuildViewMatrix();
            }

            void SetProjMatrixAsOrtho(FP left,
                                      FP right,
                                      FP bottom,
                                      FP top,
                                      FP dist_near,
                                      FP dist_far)
            {
                m_mode = Mode::Orthographic;
                m_left = left;
                m_right = right;
                m_bottom = bottom;
                m_top = top;
                m_dist_near = dist_near;
                m_dist_far = dist_far;
                rebuildProjMatrix();
            }

            void SetProjMatrixAsOrtho(FP left,
                                      FP right,
                                      FP bottom,
                                      FP top)
            {
                m_mode = Mode::Orthographic;
                m_left = left;
                m_right = right;
                m_bottom = bottom;
                m_top = top;
                rebuildProjMatrix();
            }

            void SetProjMatrixAsPerspective(FP fovy_rads,
                                            FP aspect_ratio)
            {
                m_mode = Mode::Perspective;
                m_fovy_rads = fovy_rads;
                m_aspect_ratio = aspect_ratio;
                rebuildProjMatrix();
            }

            void SetProjMatrixAsPerspective(FP fovy_rads,
                                            FP aspect_ratio,
                                            FP dist_near,
                                            FP dist_far)
            {
                m_mode = Mode::Perspective;
                m_fovy_rads = fovy_rads;
                m_aspect_ratio = aspect_ratio;
                m_dist_near = dist_near;
                m_dist_far = dist_far;
                rebuildProjMatrix();
            }

            void SetFovYRads(FP fovy_rads)
            {
                m_fovy_rads = fovy_rads;
                rebuildProjMatrix();
            }

            void SetAspectRatio(FP aspect_ratio)
            {
                m_aspect_ratio = aspect_ratio;
                rebuildProjMatrix();
            }

            void SetNearAndFarDist(FP dist_near,
                                   FP dist_far)
            {
                m_dist_near = dist_near;
                m_dist_far = dist_far;
                rebuildProjMatrix();
            }

            mat4 const & GetViewMatrix() const
            {
                return m_view_matrix;
            }

            mat4 const & GetProjMatrix() const
            {
                return m_proj_matrix;
            }

            void GetViewMatrixAsLookAt(vec3 &eye,
                                       vec3 &vpt,
                                       vec3 &up) const
            {
                eye = m_eye;
                vpt = m_vpt;
                up = m_up;
            }

            void GetProjMatrixAsPerspective(FP &fovy_rads,
                                            FP &aspect_ratio,
                                            FP &dist_near,
                                            FP &dist_far) const
            {
                fovy_rads = m_fovy_rads;
                aspect_ratio = m_aspect_ratio;
                dist_near = m_dist_near;
                dist_far = m_dist_far;
            }

            vec3 const & GetEye() const
            {
                return m_eye;
            }

            vec3 const & GetViewPt() const
            {
                return m_vpt;
            }

            vec3 const & GetUp() const
            {
                return m_up;
            }

            vec3 GetViewDirection() const
            {
                return m_vpt-m_eye;
            }

            FP GetNearDist() const
            {
                return m_dist_near;
            }

            FP GetFarDist() const
            {
                return m_dist_far;
            }

            FP GetFovYRads() const
            {
                return m_fovy_rads;
            }

            FP GetAspectRatio() const
            {
                if(Mode::Orthographic) {
                    return ((m_right-m_left)/(m_top-m_bottom));
                }
                else {
                    return m_aspect_ratio;
                }
            }

            FP GetLeft() const
            {
                return m_left;
            }

            FP GetRight() const
            {
                return m_right;
            }

            FP GetBottom() const
            {
                return m_bottom;
            }

            FP GetTop() const
            {
                return m_top;
            }

        private:
            void rebuildViewMatrix()
            {
                m_view_matrix = glm::lookAt(m_eye,
                                            m_vpt,
                                            m_up);
            }

            void rebuildProjMatrix()
            {
                if(m_mode == Mode::Orthographic) {
                    m_proj_matrix =
                            glm::ortho(m_left,
                                       m_right,
                                       m_bottom,
                                       m_top,
                                       m_dist_near,
                                       m_dist_far);
                }
                else {
                    m_proj_matrix =
                            glm::perspective(m_fovy_rads,
                                             m_aspect_ratio,
                                             m_dist_near,
                                             m_dist_far);
                }
            }

            Mode m_mode;

            vec3 m_eye;
            vec3 m_vpt;
            vec3 m_up;

            FP m_fovy_rads;
            FP m_aspect_ratio;

            FP m_left;
            FP m_right;
            FP m_bottom;
            FP m_top;

            FP m_dist_near;
            FP m_dist_far;

            mat4 m_view_matrix;
            mat4 m_proj_matrix;
        };
    }
}

#endif // KS_GL_CAMERA_HPP
