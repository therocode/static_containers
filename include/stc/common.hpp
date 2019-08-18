#pragma once
#include <type_traits>
#include <utility>
#include <new>

namespace stc
{
    template<typename T>
    struct is_input_iterator
    {
        private:
            typedef std::true_type yes;
            typedef std::false_type no;
    
            template<typename U>
            static auto can_increment(int) -> decltype(++std::declval<U&>() == std::declval<U&>(), yes());
            template<typename>
            static no can_increment(...);
    
            template<typename U>
            static auto can_inequality_compare(int) -> decltype(std::declval<U&>() != std::declval<U&>(), yes());
            template<typename>
            static no can_inequality_compare(...);
    
            template<typename U>
            static auto can_dereference(int) -> decltype(*std::declval<U&>(), yes());
            template<typename>
            static no can_dereference(...);
    
        public:
    
            static constexpr bool value =
                std::is_same_v<decltype(can_increment<T>(0)), yes> &&
                std::is_same_v<decltype(can_inequality_compare<T>(0)), yes> &&
                std::is_same_v<decltype(can_dereference<T>(0)), yes>;
    };
    
    template <typename T>
    inline constexpr bool is_input_iterator_v = is_input_iterator<T>::value;


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
            value.set(std::move(v));
        }
        optional_container_storage(const value_type& v): has_value(true)
        {
            value.set(v);
        }
        template <typename ...Args>
        optional_container_storage(Args&&... args): has_value(true)
        {
            value.set(std::forward<Args>(args)...);
        }
        ~optional_container_storage()
        {
            if(has_value)
                value.destroy();
        }
        optional_container_storage(const optional_container_storage& other): has_value(false)
        {
            *this = other;
        }
        optional_container_storage& operator=(const optional_container_storage& other)
        {
            if(&other == this)
                return *this;

            if(has_value)
                value.destroy();
            
            has_value = other.has_value;
            if(other.has_value)
            {
                value = other.value;
            }

            return *this;
        }
        optional_container_storage(optional_container_storage&& other): has_value(false)
        {
            *this = std::move(other);
        }
        optional_container_storage& operator=(optional_container_storage&& other)
        {
            if(has_value)
                value.destroy();
            
            has_value = other.has_value;

            if(other.has_value)
            {
                value = std::move(other.value);
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

        container_storage<value_type> value;
        bool has_value;
    };
}
