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
    template <typename t_data, size_t t_capacity>
    class static_vector
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
        public:
            template <typename iter_value_type, typename iter_storage_type>
            struct iterator_t
            {
                using difference_type = std::ptrdiff_t;
                using value_type = iter_value_type;
                using pointer = value_type*;
                using reference = value_type&;
                using iterator_category = std::random_access_iterator_tag;

                iterator_t& operator++ ()
                {
                    ++target;
                    return *this;
                }

                iterator_t operator++ (int)
                {
                    iterator_t tmp = *this;
                    operator++();
                    return tmp;
                }

                iterator_t& operator-- ()
                {
                    --target;
                    return *this;
                }

                iterator_t operator-- (int)
                {
                    iterator_t tmp = *this;
                    operator--();
                    return tmp;
                }

                iterator_t& operator+=(difference_type diff)
                {
                    target += diff;
                    return *this;
                }

                iterator_t operator+(difference_type diff)
                {
                    iterator_t tmp = *this;
                    tmp.target += diff;
                    return tmp;
                }

                friend iterator_t operator+(difference_type diff, const iterator_t& iter)
                {
                    return iter + diff;
                }

                iterator_t& operator-=(difference_type diff)
                {
                    target -= diff;
                    return *this;
                }

                difference_type operator-(iterator_t  iter) const
                {
                    return target - iter.target;
                }

                iterator_t operator-(difference_type diff)
                {
                    iterator_t tmp = *this;
                    tmp.target -= diff;
                    return tmp;
                }

                friend iterator_t operator-(difference_type diff, const iterator_t& iter)
                {
                    return iter - diff;
                }

                iter_value_type& operator *() const
                {
                    return target->get();
                }

                iter_value_type* operator->() const
                {
                    return &target->get();
                }

                iter_value_type& operator[](size_type index) const
                {
                    return (target + index)->get();
                }

                bool operator==(const iterator_t& other) const
                {
                    return target == other.target;
                }

                bool operator!=(const iterator_t& other) const
                {
                    return !(*this == other);
                }

                bool operator<(const iterator_t& other) const
                {
                    return target < other.target;
                }

                bool operator>(const iterator_t& other) const
                {
                    return target > other.target;
                }

                bool operator<=(const iterator_t& other) const
                {
                    return target <= other.target;
                }

                bool operator>=(const iterator_t& other) const
                {
                    return target >= other.target;
                }

                template<typename U = iter_value_type, typename = typename std::enable_if_t<!std::is_const_v<U>>>
                operator iterator_t<const U, const iter_storage_type>()
                {
                    return {target};
                }

                iter_storage_type* target = nullptr;
            };

            using iterator = iterator_t<value_type, storage_type>;
            using const_iterator = iterator_t<const value_type, const storage_type>;
            using reverse_iterator = std::reverse_iterator<iterator>;
            using const_reverse_iterator = std::reverse_iterator<const_iterator>;

            constexpr static size_type capacity = t_capacity;

            static_vector():
                m_size(0)
            {
            }
            ~static_vector()
            {
                destroy();
            }
            static_vector(const static_vector& other): m_size(0)
            {
                for(const_reference item : other)
                    emplace_back(item);
            }
            static_vector& operator=(const static_vector& other)
            {
                clear();

                for(const_reference item : other)
                    emplace_back(item);

                return *this;
            }
            static_vector(static_vector&& other)
            {
                clear();

                for(reference item : other)
                    emplace_back(std::move(item));

                other.clear();
            }
            static_vector& operator=(static_vector&& other)
            {
                clear();

                for(reference item : other)
                    emplace_back(std::move(item));

                other.clear();

                return this;
            }
            static_vector(size_type size):
                m_size(size)
            {
            }
            static_vector(size_type size, const value_type& data):
                m_size(size)
            {
                for(size_type i = 0; i < m_size; ++i)
                    (*this)[i] = data;
            }
            static_vector(std::initializer_list<value_type> data):
                m_size(data.size())
            {
                for(size_type i = 0; i < m_size; ++i)
                    m_storage[i].set(*(data.begin() + i));
            }
            template <size_type size>
            static_vector(value_type (&&arr)[size]):
                m_size(arr.size())
            {
                for(size_type i = 0; i < size; ++i)
                    m_storage[i].set(std::forward<value_type>(arr[i]));
            }
            reference push_back(const value_type& new_entry)
            {
                size_t index = m_size++;
                m_storage[index].set(new_entry);
                //ASSERT(m_size <= t_capacity, "adding entry to full static vector of size " << t_capacity << "\n");
                return m_storage[index].get();
            }
            reference push_back(value_type&& new_entry)
            {
                size_t index = m_size++;
                m_storage[index].set(std::forward<value_type>(new_entry));
                //ASSERT(m_size <= t_capacity, "adding entry to full static vector of size " << t_capacity << "\n");
                return m_storage[index].get();
            }
            template <typename ...Args>
            reference emplace_back(Args&&... args)
            {
                size_t index = m_size++;
                m_storage[index].set(value_type{std::forward<Args>(args)...});
                //ASSERT(m_size <= t_capacity, "adding entry to full static vector of size " << t_capacity << "\n");
                return m_storage[index].get();
            }
            bool empty() const
            {
                return m_size == 0;
            }
            bool full() const
            {
                return m_size == t_capacity;
            }
            size_type size() const
            {
                return m_size;
            }
            const value_type& operator[](size_type index) const
            {
                return m_storage[index].get();
            }
            value_type& operator[](size_type index)
            {
                return m_storage[index].get();
            }
            const value_type& front() const
            {
                return m_storage[0].get();
            }
            value_type& front()
            {
                return m_storage[0].get();
            }
            const value_type& back() const
            {
                return m_storage[m_size - 1].get();
            }
            value_type& back()
            {
                return m_storage[m_size - 1].get();
            }
            const_iterator begin() const
            {
                return const_iterator{m_storage.data()};
            }
            iterator begin()
            {
                return iterator{m_storage.data()};
            }
            const_iterator end() const
            {
                return begin() + m_size;
            }
            iterator end()
            {
                return begin() + m_size;
            }
            iterator erase(const_iterator position)
            {
                size_type index = position - begin();

                //ASSERT(index < m_size, "trying to erase out of bounds or with bad iterator. iter: " << position << " index: " << index << "\n");

                for(size_type i = index; i < m_size - 1; ++i)
                {
                    (*this)[i] = std::move((*this)[i + 1]);
                }

                --m_size;

                m_storage[m_size].destroy();

                return begin() + index;
            }
            iterator erase(const_iterator startIn, const_iterator endIn)
            {
                iterator start = begin() + (startIn - begin());
                iterator end = begin() + (endIn - begin());

                if(start == end)
                    return start;

                size_type deletedCount = end - start;

                for(iterator current = start; current + deletedCount != end; ++current)
                {
                    *current = std::move(*(current + deletedCount));
                }
                
                for(iterator current = end - deletedCount; current != end; ++end)
                {
                    size_t index = current - start;
                    m_storage[index].destroy();
                }

                m_size -= deletedCount;

                return start;
            }
            void clear()
            {
                destroy();
                m_size = 0;
            }
            void pop_back()
            {
                //ASSERT(m_size > 0, "trying to pop_back an empty static vector");

                m_storage[m_size - 1].destroy();
                --m_size;
            }
            iterator insert(const_iterator position, value_type value)
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
                return begin() + target_index;
            }
            void resize(size_type new_size)
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
            void destroy()
            {
                size_type s = size();
                for(size_type i = 0; i < s; ++i)
                    m_storage[i].destroy();
            }
            std::array<storage_type, t_capacity> m_storage;
            size_type m_size;
    };
}
