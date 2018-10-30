#pragma once
#include <cstddef>
#include <utility>
#include <type_traits>
#include <cstdint>
#include <array>
#include <iterator>
#include <stc/container_storage.hpp>

namespace stc
{
    class static_vector_trivial_base {};
    template <typename child>
    class static_vector_non_literal_base
    {
        public:
            static_vector_non_literal_base() = default;
            ~static_vector_non_literal_base()
            {
                child* child_ptr = static_cast<child*>(this);

                child_ptr->destroy();
            }
            static_vector_non_literal_base(child&& other)
            {
                child* child_ptr = static_cast<child*>(this);

                for(typename child::reference item : other)
                    child_ptr->emplace_back(std::move(item));

                other.clear();
            }
            static_vector_non_literal_base& operator=(child&& other)
            {
                child* child_ptr = static_cast<child*>(this);

                child_ptr->clear();

                for(typename child::reference item : other)
                    child_ptr->emplace_back(item);

                other.clear();

                return this;
            }
    };

    template <typename t_data, size_t t_capacity>
    class static_vector: public std::conditional_t<std::is_trivially_destructible_v<t_data>, static_vector_trivial_base, static_vector_non_literal_base<static_vector<t_data, t_capacity>>>
    {
        public:
            using value_type = t_data;
            using reference = value_type&;
            using const_reference = const value_type&;
            using pointer = value_type*;
            using const_pointer = const value_type*;
            using size_type = size_t;
            using difference_type = std::ptrdiff_t;

        private:
            using storage_type = container_storage<value_type>;
            using base_class = std::conditional_t<std::is_trivially_destructible_v<value_type>, static_vector_trivial_base, static_vector_non_literal_base<static_vector<value_type, t_capacity>>>;
            friend base_class;

        public:
            template <typename iter_value_type, typename iter_storage_type>
            struct iterator_t
            {
                using difference_type = std::ptrdiff_t;
                using value_type = iter_value_type;
                using pointer = value_type*;
                using reference = value_type&;
                using iterator_category = std::random_access_iterator_tag;

                constexpr iterator_t& operator++ ()
                {
                    ++target;
                    return *this;
                }

                constexpr iterator_t operator++ (int)
                {
                    iterator_t tmp = *this;
                    operator++();
                    return tmp;
                }

                constexpr iterator_t& operator-- ()
                {
                    --target;
                    return *this;
                }

                constexpr iterator_t operator-- (int)
                {
                    iterator_t tmp = *this;
                    operator--();
                    return tmp;
                }

                constexpr iterator_t& operator+=(difference_type diff)
                {
                    target += diff;
                    return *this;
                }

                constexpr iterator_t operator+(difference_type diff)
                {
                    iterator_t tmp = *this;
                    tmp.target += diff;
                    return tmp;
                }

                friend constexpr iterator_t operator+(difference_type diff, const iterator_t& iter)
                {
                    return iter + diff;
                }

                constexpr iterator_t& operator-=(difference_type diff)
                {
                    target -= diff;
                    return *this;
                }

                constexpr difference_type operator-(iterator_t  iter) const
                {
                    return target - iter.target;
                }

                constexpr iterator_t operator-(difference_type diff)
                {
                    iterator_t tmp = *this;
                    tmp.target -= diff;
                    return tmp;
                }

                friend constexpr iterator_t operator-(difference_type diff, const iterator_t& iter)
                {
                    return iter - diff;
                }

                constexpr iter_value_type& operator *() const
                {
                    return target->get();
                }

                constexpr iter_value_type* operator->() const
                {
                    return &target->get();
                }

                constexpr iter_value_type& operator[](size_type index) const
                {
                    return (target + index)->get();
                }

                constexpr bool operator==(const iterator_t& other) const
                {
                    return target == other.target;
                }

                constexpr bool operator!=(const iterator_t& other) const
                {
                    return !(*this == other);
                }

                constexpr bool operator<(const iterator_t& other) const
                {
                    return target < other.target;
                }

                constexpr bool operator>(const iterator_t& other) const
                {
                    return target > other.target;
                }

                constexpr bool operator<=(const iterator_t& other) const
                {
                    return target <= other.target;
                }

                constexpr bool operator>=(const iterator_t& other) const
                {
                    return target >= other.target;
                }

                template<typename U = iter_value_type, typename = typename std::enable_if_t<!std::is_const_v<U>>>
                constexpr operator iterator_t<const U, const iter_storage_type>()
                {
                    return {target};
                }

                iter_storage_type* target = nullptr;
            };

            using iterator = iterator_t<value_type, storage_type>;
            using const_iterator = iterator_t<const value_type, const storage_type>;
            using reverse_iterator = std::reverse_iterator<iterator>;
            using const_reverse_iterator = std::reverse_iterator<const_iterator>;

            static constexpr size_type capacity = t_capacity;

            constexpr static_vector():
                m_size(0)
            {
            }
            constexpr static_vector(const static_vector& other): m_size(0)
            {
                for(const_reference item : other)
                    emplace_back(item);
            }
            constexpr static_vector& operator=(const static_vector& other)
            {
                clear();

                for(const_reference item : other)
                    emplace_back(item);

                return *this;
            }
            constexpr static_vector(size_type size):
                m_size(size)
            {
                for(size_type i = 0; i < m_size; ++i)
                    (*this)[i] = value_type{};
            }
            constexpr static_vector(size_type size, const value_type& data):
                m_size(size)
            {
                for(size_type i = 0; i < m_size; ++i)
                    (*this)[i] = data;
            }
            constexpr static_vector(std::initializer_list<value_type> data):
                m_size(data.size())
            {
                for(size_type i = 0; i < m_size; ++i)
                    m_storage[i].set(*(data.begin() + i));
            }
            template <size_type size>
            constexpr static_vector(value_type (&&arr)[size]):
                m_size(arr.size())
            {
                for(size_type i = 0; i < size; ++i)
                    m_storage[i].set(std::forward<value_type>(arr[i]));
            }
            constexpr void push_back(value_type new_entry)
            {
                m_storage[m_size] = std::move(new_entry);
                ++m_size;
                //ASSERT(m_size <= t_capacity, "adding entry to full static vector of size " << t_capacity << "\n");
            }
            template <typename ...t_args>
            constexpr void emplace_back(t_args&&... args)
            {
                m_storage[m_size] = value_type{std::forward<t_args>(args)...};
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
            constexpr size_type size() const
            {
                return m_size;
            }
            constexpr const value_type& operator[](size_type index) const
            {
                return m_storage[index].get();
            }
            constexpr value_type& operator[](size_type index)
            {
                return m_storage[index].get();
            }
            constexpr const value_type& front() const
            {
                return m_storage[0].get();
            }
            constexpr value_type& front()
            {
                return m_storage[0].get();
            }
            constexpr const value_type& back() const
            {
                return m_storage[m_size - 1].get();
            }
            constexpr value_type& back()
            {
                return m_storage[m_size - 1].get();
            }
            constexpr const_iterator begin() const
            {
                return const_iterator{m_storage.data()};
            }
            constexpr iterator begin()
            {
                return iterator{m_storage.data()};
            }
            constexpr const_iterator end() const
            {
                return begin() + m_size;
            }
            constexpr iterator end()
            {
                return begin() + m_size;
            }
            constexpr iterator erase(iterator position)
            {
                size_type index = position - begin();

                //ASSERT(index < m_size, "trying to erase out of bounds or with bad iterator. iter: " << position << " index: " << index << "\n");

                m_storage[index].destroy();

                for(size_type i = index; i < m_size - 1; ++i)
                {
                    (*this)[i] = std::move((*this)[i + 1]);
                }

                --m_size;

                return begin() + index;
            }
            constexpr void clear()
            {
                destroy();
                m_size = 0;
            }
            constexpr void pop_back()
            {
                //ASSERT(m_size > 0, "trying to pop_back an empty static vector");

                m_storage[m_size - 1].destroy();
                --m_size;
            }
            constexpr iterator insert(const_iterator position, value_type value)
            {
                size_type target_index = position - begin();

                //ASSERT(target_index <= m_size, "trying to insert out of bounds or with bad iterator. iter: " << position << " index: " << target_index << "\n");

                if(m_size > 0)
                {
                    for(int64_t i = static_cast<int64_t>(m_size) - 1; i >= static_cast<int64_t>(target_index); --i)
                    {
                        (*this)[static_cast<size_type>(i + 1)] = std::move((*this)[static_cast<size_type>(i)]);
                    }
                }

                (*this)[target_index] = std::move(value);

                ++m_size;
                return &(*this)[target_index];
            }
            constexpr void resize(size_type new_size)
            {
                if(new_size < m_size)
                {
                    size_type first_erase = new_size;

                    for(size_type i = first_erase; i < m_size; ++i)
                        (*this)[i].destroy();
                }

                m_size = new_size;
            }
        private:
            constexpr void destroy()
            {
                size_type s = size();
                for(size_type i = 0; i < s; ++i)
                    m_storage[i].destroy();
            }
            std::array<storage_type, t_capacity> m_storage;
            size_type m_size;
    };
}
