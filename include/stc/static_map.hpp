#pragma once
#include <cstdlib>
#include <array>
#include <stc/container_storage.hpp>

namespace stc
{
template <typename t_key_type, typename t_mapped_type, size_t t_capacity>
class static_map
{
    public:
        using key_type = t_key_type;
        using mapped_type = t_mapped_type;

        struct pair
        {
            key_type first;
            mapped_type second;
        };


        using size_type = size_t;
        using value_type = pair;
        using reference = value_type&;
        using const_reference = const value_type&;
        using pointer = value_type*;
        using const_pointer = const value_type*;
        static constexpr size_type capacity = t_capacity;
    private:
        using concrete_type = static_map<key_type, mapped_type, capacity>;
        using slot_type = optional_container_storage<value_type>;
    public:

        template<typename value_type>
        struct iterator_t
        {
            constexpr iterator_t& operator++ ()
            {
                do
                {
                    ++target;
                }while(target < end && !target->has_value);

                return *this;
            }

            constexpr value_type& operator *() const
            {
                return **target;
            }

            constexpr value_type* operator->() const
            {
                return &target->get();
            }

            constexpr bool operator==(const iterator_t& other) const
            {
                return target == other.target && end == other.end;
            }

            constexpr bool operator!=(const iterator_t& other) const
            {
                return !(*this == other);
            }

            slot_type* target = nullptr;
            slot_type* end = nullptr;
        };

        using iterator = iterator_t<value_type>;
        using const_iterator = iterator_t<const value_type>;

        constexpr static_map() = default;
        template <size_t size>
        constexpr static_map(value_type const (&arr)[size])
        {
            for(const value_type& v: arr)
                insert(v);
        }

        constexpr std::pair<iterator, bool> insert(value_type entry)
        {
            iterator existing = find(entry.first);

            if(existing != end())
            {
                return {existing, false};
            }
            else
            {
                size_t target = find_next_empty_slot();

                if(target != m_storage.size())
                {
                    slot_type& slot = m_storage[target];
                    slot = slot_type{std::move(entry)};
                    ++m_size;
                    return {{&slot, &(*m_storage.end())}, true};
                }
                else
                {
                    //full. assert?
                    return {end(), false};
                }
            }
        }

        constexpr mapped_type& operator[] (key_type key)
        {
            iterator existing = find(key);


            if(existing != end())
            {
                return existing->second;
            }
            else
            {
                size_t target = find_next_empty_slot();

                if(target == m_storage.size())
                {
                    //full. assert?
                }

                slot_type& slot = m_storage[target];
                slot = slot_type{{key, mapped_type{}}};
                ++m_size;

                return slot.get().second;
            }
        }

        constexpr const mapped_type& operator[] (const key_type& key) const
        {
            return const_cast<concrete_type&>(*this)[key];
        }

        constexpr mapped_type& at(const key_type& key)
        {
            return const_cast<mapped_type&>(const_cast<const concrete_type&>(*this).at(key));
        }

        constexpr const mapped_type& at(const key_type& key) const
        {
            const_iterator found = find(key);
            return found->second;
        }

        constexpr iterator erase(const key_type& key)
        {
            iterator found = find(key);

            if(found != end())
            {
                size_t index = std::distance(m_storage.data(), found.target); 

                m_storage[index] = {};
                --m_size;

                return ++found;
            }

            return found;
        }

        constexpr iterator find(const key_type& key)
        {
            iterator end_iter = end();

            for(iterator iter = begin(); iter != end_iter; ++iter)
            {
                if(iter->first == key)
                {
                    return iter;
                }
            }

            return end_iter;
        }

        constexpr const_iterator find(const key_type& key) const
        {
            iterator found = const_cast<concrete_type*>(this)->find(key);
            return const_iterator{found.target, found.end};
        }

        constexpr bool contains(const key_type& key) const
        {
            return find(key) != end();
        }

        constexpr size_t count(const key_type& key) const
        {
            return static_cast<size_t>(contains(key));
        }

        constexpr void clear()
        {
            m_storage.fill({});
            m_size = 0;
        }

        constexpr size_t size() const
        {
            return m_size;
        }

        constexpr bool empty() const
        {
            return m_size == 0;
        }

        constexpr bool full() const
        {
            return m_size == m_storage.size();
        }

        constexpr iterator begin()
        {
            iterator result;
            result.target = m_storage.end();
            result.end = m_storage.end();

            for(auto& entry : m_storage)
            {
                if(entry.has_value)
                {
                    result.target = &entry;
                    break;
                }
            }

            return result;
        }

        constexpr const_iterator begin() const
        {
            iterator iter = const_cast<concrete_type*>(this)->begin();
            return const_iterator{iter.target, iter.end};
        }

        constexpr iterator end()
        {
            iterator result;
            result.target = m_storage.end();
            result.end = m_storage.end();

            return result;
        }

        constexpr const_iterator end() const
        {
            iterator iter = const_cast<concrete_type*>(this)->end();
            return const_iterator{iter.target, iter.end};
        }
    private:
        constexpr size_t find_next_empty_slot() const
        {
            for(size_t i = 0; i < m_storage.size(); ++i)
            {
                if(!m_storage[i].has_value)
                {
                    return i;
                }
            }

            return m_storage.size();
        }

        size_t m_size = 0;
        std::array<slot_type, capacity> m_storage;
};

template<typename key_type, typename value_type, size_t capacity>
constexpr auto make_static_map(typename static_map<key_type, value_type, capacity>::value_type const (&arr)[capacity])
{
    return stc::static_map<key_type, value_type, capacity>{arr};
}
}
