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

#include <ks/gl/KsGLResource.hpp>

namespace ks
{  
    namespace gl
    {
        std::mutex Resource::s_id_mutex;

        // NOTE: Start at one since we reserve '0'
        // to refer to an invalid/uninitialized resource
        Id Resource::s_id_counter(1);

        Id Resource::genResourceId()
        {
            std::lock_guard<std::mutex> lock(s_id_mutex);
            Id id = s_id_counter;
            s_id_counter++;
            return id;
        }

        // ============================================================= //

        Resource::Resource() :
            m_res_id(0),
            m_ref_count(0)
        {
            // empty
        }

        Resource::~Resource()
        {
            // empty
        }

        bool Resource::AddReference()
        {
            m_ref_count++;

            if(this->GLInit()) {
                m_res_id = genResourceId();
                return true;
            }
            return false;
        }

        void Resource::RemoveReference()
        {
            if(m_ref_count > 0) {
                m_ref_count--;
            }
            if(m_ref_count==0) {
                this->GLCleanUp();
                m_res_id = 0;
            }
        }

        Id Resource::GetResourceId() const
        {
            return m_res_id;
        }

        uint Resource::GetRefCount() const
        {
            return m_ref_count;
        }
    } // gl
} // ks
