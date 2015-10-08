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


#ifndef KS_GL_RESOURCE_HPP
#define KS_GL_RESOURCE_HPP

#include <mutex>
#include <ks/KsGlobal.hpp>

namespace ks
{
	namespace gl
	{
        class Resource
		{
		public:
            Resource();
            Resource(Resource const &other) = delete;
            Resource(Resource &&other) = delete;
            virtual ~Resource();

            Resource & operator = (Resource const &) = delete;
            Resource & operator = (Resource &&) = delete;

            bool AddReference();
            void RemoveReference();

            Id GetResourceId() const;
            uint GetRefCount() const;

		protected:
			virtual bool GLInit()=0;
			virtual void GLCleanUp()=0;

		private:
            Id m_res_id;
            uint m_ref_count;

            static std::mutex s_id_mutex;
            static Id s_id_counter;
            static Id genResourceId();
		};
		
	} // gl

} // ks


#endif // KS_GL_RESOURCE_HPP


