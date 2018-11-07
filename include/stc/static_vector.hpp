#pragma once
#include <cstddef>
#include <utility>
#include <type_traits>
#include <cstdint>
#include <array>
#include <limits>
#include <iterator>
#include <stdexcept>
#include <stc/common.hpp>

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

            template <typename iter_value_type, typename iter_storage_type>
            struct iterator_t
            {
                using difference_type = std::ptrdiff_t;
                using value_type = iter_value_type;
                using pointer = value_type*;
                using reference = value_type&;
                using iterator_category = std::random_access_iterator_tag;

                iterator_t& operator++()
                {
                    ++target;
                    return *this;
                }

                iterator_t operator++(int)
                {
                    iterator_t tmp = *this;
                    operator++();
                    return tmp;
                }

                iterator_t& operator--()
                {
                    --target;
                    return *this;
                }

                iterator_t operator--(int)
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

                iterator_t operator+(difference_type diff) const
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

                iterator_t operator-(difference_type diff) const
                {
                    iterator_t tmp = *this;
                    tmp.target -= diff;
                    return tmp;
                }

                friend iterator_t operator-(difference_type diff, const iterator_t& iter)
                {
                    return iter - diff;
                }

                iter_value_type& operator*() const
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

        private:
            using storage_type = container_storage<value_type>;
        public:
            using iterator = iterator_t<value_type, storage_type>;
            using const_iterator = iterator_t<const value_type, const storage_type>;
            using reverse_iterator = std::reverse_iterator<iterator>;
            using const_reverse_iterator = std::reverse_iterator<const_iterator>;

            //constructors
            static_vector():
                m_size(0)
            {
            }
            explicit static_vector(size_type size, const value_type& data):
                m_size(size)
            {
                for(size_type i = 0; i < m_size; ++i)
                {
                    m_storage[i].set(data);
                }
            }
            explicit static_vector(size_type size):
                m_size(size)
            {
                for(size_type i = 0; i < m_size; ++i)
                {
                    m_storage[i].set();
                }
            }
            template<typename input_iter, typename std::enable_if_t<is_input_iterator_v<input_iter>>* = nullptr>
            static_vector(input_iter start, input_iter end):
                m_size(0)
            {
                for(auto current = start; current != end; ++current)
                    push_back(*current);
            }
            static_vector(const static_vector& other): m_size(0)
            {
                for(const_reference item : other)
                    emplace_back(item);
            }
            static_vector(static_vector&& other): m_size(0)
            {
                for(reference item : other)
                    emplace_back(std::move(item));

                other.clear();
            }
            static_vector(std::initializer_list<value_type> data):
                m_size(data.size())
            {
                for(size_type i = 0; i < m_size; ++i)
                    m_storage[i].set(*(data.begin() + i));
            }
            //TBI with a struct tag to disambiguate from std::initializer_list constructor maybe
            //template <size_type size>
            //static_vector(value_type (&&arr)[size]):
            //    m_size(arr.size())
            //{
            //    for(size_type i = 0; i < size; ++i)
            //        m_storage[i].set(std::forward<value_type>(arr[i]));
            //}
            //destructor
            ~static_vector()
            {
                destroy();
            }
            //assignment
            static_vector& operator=(const static_vector& other)
            {
                clear();

                for(const_reference item : other)
                    emplace_back(item);

                return *this;
            }
            static_vector& operator=(static_vector&& other)
            {
                clear();

                for(reference item : other)
                    emplace_back(std::move(item));

                other.clear();

                return *this;
            }
            static_vector& operator=(std::initializer_list<value_type> data)
            {
                destroy();

                m_size = data.size();

                for(size_type i = 0; i < m_size; ++i)
                    m_storage[i].set(*(data.begin() + i));

                return *this;
            }
            //assign
            void assign(size_type count, const value_type& value)
            {
                destroy();

                m_size = count;

                for(size_type i = 0; i < m_size; ++i)
                    m_storage[i].set(value);
            }
            template<typename input_iter, typename std::enable_if_t<is_input_iterator_v<input_iter>>* = nullptr>
            void assign(input_iter start, input_iter end)
            {
                destroy();

                size_type i = 0;
                for(auto current = start; current != end; ++current, ++i)
                {
                    m_storage[i].set(*current);
                }

                m_size = i;
            }
            void assign(std::initializer_list<value_type> data)
            {
                destroy();

                m_size = data.size();

                auto current = data.begin();
                for(size_type i = 0; i < m_size; ++i, ++current)
                    m_storage[i].set(*current);
            }
            //element access
            reference at(size_type index)
            {
                if(index >= m_size)
                {
                    throw std::out_of_range("accessing out of static_vector bounds");
                }
                return (*this)[index];
            }
            const_reference at(size_type index) const
            {
                if(index >= m_size)
                {
                    throw std::out_of_range("accessing out of static_vector bounds");
                }
                return (*this)[index];
            }
            reference operator[](size_type index)
            {
                return m_storage[index].get();
            }
            const_reference operator[](size_type index) const
            {
                return m_storage[index].get();
            }
            reference front()
            {
                return m_storage[0].get();
            }
            const_reference front() const
            {
                return m_storage[0].get();
            }
            reference back()
            {
                return m_storage[m_size - 1].get();
            }
            const_reference back() const
            {
                return m_storage[m_size - 1].get();
            }
            value_type* data() noexcept
            {
                return &(*this)[0];
            }
            const value_type* data() const noexcept
            {
                return &(*this)[0];
            }
            //iterators
            iterator begin()
            {
                return iterator{m_storage.data()};
            }
            const_iterator begin() const
            {
                return const_iterator{m_storage.data()};
            }
            const_iterator cbegin() const
            {
                return const_iterator{m_storage.data()};
            }
            iterator end()
            {
                return begin() + m_size;
            }
            const_iterator end() const
            {
                return begin() + m_size;
            }
            const_iterator cend() const
            {
                return begin() + m_size;
            }
            reverse_iterator rbegin()
            {
                return reverse_iterator{end()};
            }
            const_reverse_iterator rbegin() const
            {
                return const_reverse_iterator{end()};
            }
            const_reverse_iterator crbegin() const
            {
                return const_reverse_iterator{end()};
            }
            reverse_iterator rend()
            {
                return reverse_iterator{begin()};
            }
            const_reverse_iterator rend() const
            {
                return const_reverse_iterator{begin()};
            }
            const_reverse_iterator crend() const
            {
                return const_reverse_iterator{begin()};
            }
            //capacity
            bool empty() const
            {
                return m_size == 0;
            }
            size_type size() const
            {
                return m_size;
            }
            constexpr size_type max_size() const
            {
                return std::numeric_limits<size_type>::max() / static_cast<size_type>(sizeof(value_type));
            }
            constexpr static size_type capacity()
            {
                return t_capacity;
            }
            bool full() const
            {
                return m_size == t_capacity;
            }
            //modifiers
            void clear()
            {
                destroy();
                m_size = 0;
            }
            iterator insert(const_iterator cposition, const value_type& value)
            {
                //size_type target_index = index_of(cposition);
                //ASSERT(target_index <= m_size, "trying to insert out of bounds or with bad iterator. iter: " << position << " index: " << target_index << "\n");
                m_storage[m_size].set(value);

                iterator position = non_const(cposition);
                move_segment_down(end(), end() + 1, position);

                ++m_size;
                return position;
            }
            iterator insert(const_iterator cposition, value_type&& value)
            {
                //size_type target_index = index_of(position);
                //ASSERT(target_index <= m_size, "trying to insert out of bounds or with bad iterator. iter: " << position << " index: " << target_index << "\n");
                m_storage[m_size].set(std::forward<value_type>(value));

                iterator position = non_const(cposition);
                move_segment_down(end(), end() + 1, position);

                ++m_size;
                return position;
            }
            iterator insert(const_iterator cposition, size_type count, const value_type& value)
            {
                size_t end_index = m_size + count;

                for(size_type i = m_size; i < end_index; ++i)
                {
                    m_storage[i].set(value);
                }

                iterator position = non_const(cposition);
                move_segment_down(end(), end() + count, position);

                m_size += count;
                return position;
            }
            template <typename input_iter, typename std::enable_if_t<is_input_iterator_v<input_iter>>* = nullptr>
            iterator insert(const_iterator cposition, input_iter first, input_iter last)
            {
                auto current = first;
                size_type i = m_size;
                for(; current != last; ++i, ++current)
                {
                    m_storage[i].set(*current);
                }
                size_type count = i - m_size;

                iterator position = non_const(cposition);
                move_segment_down(end(), end() + count, position);

                m_size += count;
                return position;
            }
            iterator insert(const_iterator cposition, std::initializer_list<value_type> ilist)
            {
                return insert(cposition, ilist.begin(), ilist.end());
            }
            template <typename... Args>
            iterator emplace(const_iterator cposition, Args&&... args)
            {
                m_storage[m_size].set(std::forward<Args...>(args)...);

                iterator position = non_const(cposition);
                move_segment_down(end(), end() + 1, position);

                ++m_size;
                return position;
            }
            iterator erase(const_iterator cposition)
            {
                //size_type index = index_of(cposition);
                //ASSERT(index < m_size, "trying to erase out of bounds or with bad iterator. iter: " << position << " index: " << index << "\n");

                return erase(cposition, cposition + 1);
            }
            iterator erase(const_iterator cerase_start, const_iterator cerase_end)
            {
                //size_type index = index_of(cposition);
                //ASSERT(index < m_size, "trying to erase out of bounds or with bad iterator. iter: " << position << " index: " << index << "\n");

                if(cerase_start == cerase_end)
                    return non_const(cerase_start);

                iterator erase_start = non_const(cerase_start);
                iterator erase_end = non_const(cerase_end);
                size_type erase_count = erase_end - erase_start;

                std::rotate(erase_start, erase_start + erase_count, end());

                m_size -= erase_count;

                size_type end_index = m_size + erase_count;
                for(size_type i = m_size; i < end_index; ++i)
                {
                    m_storage[i].destroy();
                }

                return erase_start;
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
                m_storage[index].set(std::forward<Args>(args)...);
                //ASSERT(m_size <= t_capacity, "adding entry to full static vector of size " << t_capacity << "\n");
                return m_storage[index].get();
            }
            void pop_back()
            {
                //ASSERT(m_size > 0, "trying to pop_back an empty static vector");

                m_storage[m_size - 1].destroy();
                --m_size;
            }
            void resize(size_type new_size)
            {
                if(new_size < m_size)
                {
                    size_type first_erase = new_size;

                    for(size_type i = first_erase; i < m_size; ++i)
                        m_storage[i].destroy();
                }
                else
                {
                    for(size_type i = m_size; i < new_size; ++i)
                        m_storage[i].set();
                }

                m_size = new_size;
            }
            void resize(size_type new_size, const value_type& value)
            {
                if(new_size < m_size)
                {
                    size_type first_erase = new_size;

                    for(size_type i = first_erase; i < m_size; ++i)
                        m_storage[i].destroy();
                }
                else
                {
                    for(size_type i = m_size; i < new_size; ++i)
                        m_storage[i].set(value);
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
            size_type index_of(const_iterator iter) const
            {
                return iter - begin();
            }
            iterator non_const(const_iterator iter)
            {
                return begin() + index_of(iter);
            }
            void move_segment_down(iterator start, iterator end, iterator destination)
            {
                std::rotate(destination, start, end);
            }

            std::array<storage_type, t_capacity> m_storage;
            size_type m_size;
    };

    template <typename t_data, size_t t_a_capacity, size_t t_b_capacity>
    bool operator==(static_vector<t_data, t_a_capacity> a, static_vector<t_data, t_b_capacity> b)
    {
        size_t a_size = a.size();
        size_t b_size = b.size();

        if(a_size != b_size)
            return false;

        for(size_t i = 0; i < a_size; ++i)
        {
            if(!(a[i] == b[i]))
                return false;
        }

        return true;
    }
}
