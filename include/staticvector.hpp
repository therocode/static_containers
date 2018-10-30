#pragma once
#include <cstddef>
#include <utility>
#include <type_traits>
#include <cstdint>
#include <array>
#include <containerstorage.hpp>

namespace spr
{
    class static_vector_trivial_base {};
    template <typename child>
    class static_vector_non_trivial_base
    {
        public:
            static_vector_non_trivial_base() = default;
            ~static_vector_non_trivial_base()
            {
                child* child_ptr = static_cast<child*>(this);
                for(size_t i = 0; i < child_ptr->size(); ++i)
                    child_ptr->m_data[i].destroy();
            }
    };

    template <typename data, size_t t_capacity>
    class static_vector: public std::conditional_t<std::is_trivially_destructible_v<data>, static_vector_trivial_base, static_vector_non_trivial_base<static_vector<data, t_capacity>>>
    {
        using base_class = std::conditional_t<std::is_trivially_destructible_v<data>, static_vector_trivial_base, static_vector_non_trivial_base<static_vector<data, t_capacity>>>;
        friend base_class::~base_class();

        public:
            using value_type = data;
            using iterator = value_type*;
            using const_iterator = const value_type*;
            using reference = value_type&;
            using const_reference = const value_type&;
            using pointer = value_type*;
            using const_pointer = const value_type*;

            static constexpr size_t capacity = t_capacity;
            constexpr static_vector():
                m_size(0)
            {
            }
            constexpr static_vector(size_t size):
                m_size(size)
            {
                for(size_t i = 0; i < m_size; ++i)
                    (*this)[i] = data{};
            }
            constexpr static_vector(size_t size, const data& data):
                m_size(size)
            {
                for(size_t i = 0; i < m_size; ++i)
                    (*this)[i] = data;
            }
            constexpr static_vector(std::initializer_list<data> data):
                m_size(data.size())
            {
                for(size_t i = 0; i < m_size; ++i)
                    m_data[i].set(*(data.begin() + i));
            }
            template <size_t size>
            constexpr static_vector(value_type const (&arr)[size]):
                m_size(arr.size())
            {
                for(size_t i = 0; i < size; ++i)
                    m_data[i].set(arr[i]);
            }
            //TBI COPY/MOVE  .... LEAKS RIGHT NOW
            constexpr void push_back(data new_entry)
            {
                m_data[m_size] = std::move(new_entry);
                ++m_size;
                //ASSERT(m_size <= t_capacity, "adding entry to full static vector of size " << t_capacity << "\n");
            }
            template <typename ...args>
            constexpr void emplace_back(args&&... args)
            {
                m_data[m_size] = data{std::forward<args>(args)...};
                ++m_size;
                //ASSERT(m_size <= t_capacity, "adding entry to full static vector of size " << t_capacity << "\n");
            }
            constexpr bool empty() const
            {
                return m_size == 0;
            }
            constexpr bool full() const
            {
                return m_size == t_capacity;
            }
            constexpr size_t size() const
            {
                return m_size;
            }
            constexpr const data& operator[](size_t index) const
            {
                return m_data[index].get();
            }
            constexpr data& operator[](size_t index)
            {
                return m_data[index].get();
            }
            constexpr const data& front() const
            {
                return m_data[0].get();
            }
            constexpr data& front()
            {
                return m_data[0].get();
            }
            constexpr const data& back() const
            {
                return m_data[m_size - 1].get();
            }
            constexpr data& back()
            {
                return m_data[m_size - 1].get();
            }
            constexpr const_iterator begin() const
            {
                return &front();
            }
            constexpr iterator begin()
            {
                return &front();
            }
            constexpr const_iterator end() const
            {
                return begin() + m_size;
            }
            constexpr iterator end()
            {
                return begin() + m_size;
            }
            constexpr iterator erase(const data* position)
            {
                size_t index = position - begin();

                //ASSERT(index < m_size, "trying to erase out of bounds or with bad iterator. iter: " << position << " index: " << index << "\n");

                (*this)[index].~data();

                for(size_t i = index; i < m_size - 1; ++i)
                {
                    (*this)[i] = std::move((*this)[i + 1]);
                }

                --m_size;

                return &(*this)[index];
            }
            constexpr void clear()
            {
                *this = {};
            }
            constexpr void pop_back()
            {
                //ASSERT(m_size > 0, "trying to pop_back an empty static vector");

                (*this)[m_size - 1].~data();
                --m_size;
            }
            constexpr iterator insert(const_iterator position, data value)
            {
                size_t target_index = position - begin();

                //ASSERT(target_index <= m_size, "trying to insert out of bounds or with bad iterator. iter: " << position << " index: " << target_index << "\n");

                if(m_size > 0)
                {
                    for(int64_t i = static_cast<int64_t>(m_size) - 1; i >= static_cast<int64_t>(target_index); --i)
                    {
                        (*this)[static_cast<size_t>(i + 1)] = std::move((*this)[static_cast<size_t>(i)]);
                    }
                }

                (*this)[target_index] = std::move(value);

                ++m_size;
                return &(*this)[target_index];
            }
            constexpr void resize(size_t new_size)
            {
                if(new_size < m_size)
                {
                    size_t first_erase = new_size;

                    for(size_t i = first_erase; i < m_size; ++i)
                        (*this)[i].~data();
                }

                m_size = new_size;
            }
        private:
            using storage_type = container_storage<value_type>;
            std::array<storage_type, t_capacity> m_data;
            size_t m_size;
    };

    template<typename data, size_t t_capacity>
    constexpr typename static_vector<data, t_capacity>::iterator erase_by_value(static_vector<data, t_capacity>& vec, const data& to_erase)
    {
        for(size_t i = 0; i < vec.size(); ++i)
        {
            if(vec[i] == to_erase)
                return vec.erase(vec.begin() + i);
        }

        return vec.end();
    }
}
