#ifndef KS_SG_UNIFORM_NODE_HPP
#define KS_SG_UNIFORM_NODE_HPP

// stl
#include <map>

// ks
#include <ks/gl/KsGLShaderProgram.hpp>

namespace ks
{
    namespace gl
    {
        // ============================================================= //

        class UniformBase
        {
        public:
            UniformBase(std::string name) :
                m_name(std::move(name))
            {

            }

            virtual ~UniformBase()
            {

            }

            virtual void GLSetUniform(gl::ShaderProgram* shader) const = 0;
            virtual UniformBase* Copy() const = 0;
            virtual void Sync() = 0;

            std::string const &GetName() const
            {
                return m_name;
            }

        protected:
            std::string m_name;
        };

        // ============================================================= //

        template<typename T>
        class Uniform final : public UniformBase
        {
        public:
            Uniform(std::string name, T data) :
                UniformBase(name),
                m_data(data),
                m_update(data)
            {

            }

            ~Uniform() {}

            void GLSetUniform(gl::ShaderProgram* shader) const
            {
                shader->GLSetUniform(m_name,m_data);
            }

            Uniform<T>* Copy() const
            {
                Uniform<T>* copy = new Uniform<T>(m_name,m_data);
                copy->m_update = this->m_update;

                return copy;
            }

            void Update(T update)
            {
                m_update = std::move(update);
            }

            void Sync()
            {
                m_data = m_update;
            }

            T GetData()
            {
                return m_update;
            }

        private:
            T m_data;
            T m_update;
        };

        // ============================================================= //

        template<typename T>
        class UniformArray final : public UniformBase
        {
        public:
            UniformArray(std::string name, std::vector<T> data) :
                UniformBase(name),
                m_size(data.size()),
                m_data(data),
                m_update(data),
                m_reupload(false)
            {

            }

            ~UniformArray() {}

            void GLSetUniform(gl::ShaderProgram* shader) const
            {
                shader->GLSetUniform(m_name,m_data);
            }

            UniformArray<T>* Copy() const
            {
                UniformArray<T>* copy = new UniformArray<T>(m_name,m_data);
                copy->m_update = this->m_update;
                copy->m_reupload = this->m_reupload;
                copy->m_list_update_idxs = this->m_list_update_idxs;

                return copy;
            }

            void Update(std::vector<T> update)
            {
                assert(update.size() == m_size);
                m_update = std::move(update);
                m_reupload = true;
            }

            void Update(T update, uint index, bool reupload=false)
            {
                m_update[index] = update;
                if(reupload) {
                    m_reupload = true;
                }
                else {
                    m_list_update_idxs.push_back(index);
                }
            }

            void Sync()
            {
                if(m_reupload) {
                    m_data = m_update;
                    m_reupload = false;
                }
                else {
                    for(uint idx : m_list_update_idxs) {
                        m_data[idx] = m_update[idx];
                    }
                    m_list_update_idxs.clear();
                }
            }

            std::vector<T> GetData()
            {
                return m_update;
            }

        private:
            uint const m_size;
            std::vector<T> m_data;
            std::vector<T> m_update;

            bool m_reupload;
            std::vector<uint> m_list_update_idxs;
        };

        // ============================================================= //

    } // gl
} // ks

#endif // KS_SG_UNIFORM_NODE_HPP
