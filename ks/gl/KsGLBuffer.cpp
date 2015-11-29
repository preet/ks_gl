#include <ks/gl/KsGLBuffer.hpp>
#include <algorithm>

namespace ks
{
    namespace gl
    {
        // ============================================================= //

        Buffer::Update::Update(u8 options,
                               uint dst_byte_offset,
                               uint src_byte_offset,
                               size_t src_sz_bytes) :
            options(options),
            dst_byte_offset(dst_byte_offset),
            src_byte_offset(src_byte_offset),
            src_sz_bytes(src_sz_bytes)
        {}

        Buffer::Update::~Update() {}

        void const * Buffer::Update::GetData()
        {
            return nullptr;
        }

        // ============================================================= //

        Buffer::UpdateKeepData::UpdateKeepData(u8 options,
                                               uint dst_byte_offset,
                                               uint src_byte_offset,
                                               size_t src_sz_bytes,
                                               std::vector<u8>* data) :
            Buffer::Update(options,
                           dst_byte_offset,
                           src_byte_offset,
                           src_sz_bytes),
            data(data)
        {}

        Buffer::UpdateKeepData::~UpdateKeepData() {}

        void const * Buffer::UpdateKeepData::GetData()
        {
            return &((*data)[0]);
        }

        // ============================================================= //

        Buffer::UpdateFreeData::UpdateFreeData(u8 options,
                                               uint dst_byte_offset,
                                               uint src_byte_offset,
                                               size_t src_sz_bytes,
                                               std::vector<u8>* data) :
            Buffer::Update(options,
                           dst_byte_offset,
                           src_byte_offset,
                           src_sz_bytes),
            data(data)
        {}

        Buffer::UpdateFreeData::~UpdateFreeData()
        {
            delete data;
        }

        void const * Buffer::UpdateFreeData::GetData()
        {
            return &((*data)[0]);
        }

        // ============================================================= //

        Buffer::Buffer(Target target, Usage usage) :
            m_target(target),
            m_usage(usage)
        {

        }

        Buffer::~Buffer()
        {

        }

        std::string const &Buffer::GetDesc() const
        {
            return m_log_prefix;
        }

        void Buffer::SetDesc(std::string desc)
        {
            m_log_prefix = "Buffer: " + desc;
        }

        uint Buffer::GetSizeBytes() const
        {
            return m_lk_buffer_size;
        }

        std::vector<unique_ptr<Buffer::Update>> const & Buffer::GetUpdates() const
        {
            return m_list_updates;
        }

        bool Buffer::GLInit()
        {
            if(!(m_buffer_handle == 0)) {
                LOG.Error() << m_log_prefix
                            << "already have valid handle: "
                            << m_buffer_handle;
                return false;
            }

            glGenBuffers(1,&m_buffer_handle);
            KS_CHECK_GL_ERROR(m_log_prefix+"gen buffers");

            if(m_buffer_handle == 0) {
                LOG.Error() << m_log_prefix
                            << "failed to gen buffer";
                return false;
            }
            return true;
        }

        bool Buffer::GLBind()
        {
            if(m_buffer_handle == 0) {
                LOG.Error() << m_log_prefix
                            << "tried to bind buffer 0";
                return false;
            }

            glBindBuffer(static_cast<GLenum>(m_target),m_buffer_handle);
            KS_CHECK_GL_ERROR(m_log_prefix+"bind buffer");

            return true;
        }

        void Buffer::GLUnbind()
        {
            glBindBuffer(static_cast<GLenum>(m_target),0);
        }

        void Buffer::GLCleanUp()
        {
            // delete the buffer if we have a valid handle
            if(m_buffer_handle != 0) {
                glDeleteBuffers(1,&m_buffer_handle);
                m_buffer_handle = 0;
            }
        }

        void Buffer::GLSync()
        {
            for(auto& upd_uptr : m_list_updates)
            {

                Update& update = *upd_uptr;

                if((update.options & Update::ReUpload) == Update::ReUpload)
                {
                    glBufferData(static_cast<GLenum>(m_target),
                                 update.src_sz_bytes,
                                 update.GetData(),
                                 static_cast<GLenum>(m_usage));


                    KS_CHECK_GL_ERROR(m_log_prefix+"upload buffer");
                    m_lk_buffer_size = update.src_sz_bytes;
                }
                else
                {
                    glBufferSubData(static_cast<GLenum>(m_target),
                                    update.dst_byte_offset,
                                    update.src_sz_bytes,
                                    update.GetData());

                    KS_CHECK_GL_ERROR(m_log_prefix+"upload buffer subdata");
                }
            }

            m_list_updates.clear();
        }

        void Buffer::UpdateBuffer(unique_ptr<Update> update)
        {
            bool const is_reupload =
                    ((update->options & Update::ReUpload) == Update::ReUpload);

            if(is_reupload || (update->GetData()==nullptr))
            {
                // Erase all updates before this one
                m_list_updates.clear();
            }
//            else
//            {
//                // TODO we can create a minimal set of updates
//                // from a bunch of staggered smaller updates by
//                // merging overlapping ranges
//            }

            m_list_updates.push_back(std::move(update));
        }

        // ============================================================= //
    }
}
