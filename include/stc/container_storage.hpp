#pragma once
#include <type_traits>
#include <utility>

namespace stc
{
    template <typename value_type_in>
    struct container_storage
    {
        using value_type = value_type_in;

        void destroy()
        {
            get().~value_type();
        }

        void set(value_type&& v)
        {
            new(&m_data) value_type(std::forward<value_type>(v));
        }

        void set(const value_type& v)
        {
            new(&m_data) value_type(v);
        }

        template<typename ...Args>
        void set(Args&&... args) 
        {
            new(&m_data) value_type(std::forward<Args>(args)...);
        }

        value_type& get()
        {
            return *std::launder(reinterpret_cast<value_type*>(&m_data));
        }

        const value_type& get() const
        {
            return *std::launder(reinterpret_cast<const value_type*>(&m_data));
        }

        typename std::aligned_storage<sizeof(value_type), alignof(value_type)>::type m_data;
    };

    template <typename value_type_in>
    struct optional_container_storage
    {
        using value_type = value_type_in;

        optional_container_storage(): has_value(false) {}
        optional_container_storage(value_type&& v): has_value(true)
        {
            value.set(std::forward<value_type>(v));
        }
        optional_container_storage(const value_type& v): has_value(true)
        {
            value.set(v);
        }
        template <typename ...Args>
        optional_container_storage(Args&&... args)
        {
            value.set(std::forward<Args>(args)...);
        }
        ~optional_container_storage()
        {
            if(has_value)
                value.destroy();
        }
        optional_container_storage(const optional_container_storage& other): has_value(other.has_value)
        {
            if(other.has_value)
                value.set(other.get());
        }
        optional_container_storage& operator=(const optional_container_storage& other)
        {
            if(&other == this)
                return *this;

            if(has_value)
                value.destroy();
            
            has_value = other.has_value;

            if(other.has_value)
                value.set(other.get());

            return *this;
        }
        optional_container_storage(optional_container_storage&& other)
        {
            has_value = other.has_value;

            if(other.has_value)
            {
                value.set(std::move(other.get()));
                other.value.destroy();
                other.has_value = false;
            }
        }
        optional_container_storage& operator=(optional_container_storage&& other)
        {
            if(has_value)
                value.destroy();
            
            has_value = other.has_value;

            if(other.has_value)
            {
                value.set(std::move(other.get()));
                other.value.destroy();
                other.has_value = false;
            }

            return *this;
        }

        value_type& get()
        {
            return value.get();
        }

        const value_type& get() const
        {
            return value.get();
        }

        void set(value_type&& v)
        {
            *this = optional_container_storage(std::forward<value_type>(v));
        }

        void set(const value_type& v)
        {
            *this = optional_container_storage(std::forward<value_type>(v));
        }

        template<typename ...Args>
        void set(Args&&... args) 
        {
            *this = optional_container_storage(std::forward<Args>(args)...);
        }

        container_storage<value_type> value;
        bool has_value;
    };
}
