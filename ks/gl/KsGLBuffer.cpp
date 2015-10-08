#include <ks/gl/KsGLBuffer.hpp> 
#include <algorithm>

namespace ks
{
    namespace gl
    {
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

        std::vector<Buffer::Update> const & Buffer::GetUpdates() const
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
            if(m_list_updates.empty()) {
                return;
            }

            if(m_opt_retain_buffer)
            {

            }
            else
            {
                for(Update &update : m_list_updates)
                {
                    if((update.options & Update::ReUpload) == Update::ReUpload)
                    {
                        if(update.src_data) {
                            std::vector<u8> const &src_data = *(update.src_data);
                            glBufferData(static_cast<GLenum>(m_target),
                                         update.src_sz_bytes,
                                         &(src_data[update.src_byte_offset]),
                                         static_cast<GLenum>(m_usage));
                        }
                        else {
                            glBufferData(static_cast<GLenum>(m_target),
                                         update.src_sz_bytes,
                                         nullptr,
                                         static_cast<GLenum>(m_usage));
                        }
                        KS_CHECK_GL_ERROR(m_log_prefix+"upload buffer");
                        m_lk_buffer_size = update.src_sz_bytes;
                    }
                    else
                    {
                        std::vector<u8> const &src_data = *(update.src_data);

                        glBufferSubData(static_cast<GLenum>(m_target),
                                        update.dst_byte_offset,
                                        update.src_sz_bytes,
                                        &(src_data[update.src_byte_offset]));

                        KS_CHECK_GL_ERROR(m_log_prefix+"upload buffer subdata");
                    }

                    if((update.options & Update::KeepSrcData) == 0) {
                        delete update.src_data;
                    }
                }

                m_list_updates.clear();
            }
        }

        void Buffer::UpdateBuffer(Update const &update)
        {
            bool const is_reupload =
                    ((update.options & Update::ReUpload) == Update::ReUpload);

            if(is_reupload || (update.src_data==nullptr))
            {
                // Erase all updates before this one
                std::for_each(
                            m_list_updates.begin(),
                            m_list_updates.end(),
                            [](Update& upd) {
                                if((upd.options & Update::KeepSrcData) == 0) {
                                    delete upd.src_data;
                                }
                            });

                m_list_updates.clear();
            }
//            else
//            {
//                // TODO we can create a minimal set of updates
//                // from a bunch of staggered smaller updates by
//                // merging overlapping ranges
//            }

            m_list_updates.push_back(update);
        }

        // ============================================================= //
    }
}
