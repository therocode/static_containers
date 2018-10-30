#pragma once
#include <type_traits>
#include <utility>

namespace stc
{

    template <typename value_type>
    union literal_container_storage_type
    {
        struct empty{};
        constexpr literal_container_storage_type(): uninitialized{}{}
        constexpr literal_container_storage_type(value_type v): value(v){}
        constexpr void set(value_type v)
        {
            *this = literal_container_storage_type{v};
        }

        empty uninitialized;
        value_type value;
    };

    template <typename value_type>
    union non_literal_container_storage_type
    {
        struct empty{};
        non_literal_container_storage_type(): uninitialized{}{}
        non_literal_container_storage_type(value_type v): value(std::move(v)){}
        ~non_literal_container_storage_type() {}
        void set(value_type v)
        {
            new (&value) value_type(std::move(v));
        }

        empty uninitialized;
        value_type value;
    };

    template<typename value_type>
    struct get_container_storage_type
    {
        using type = std::conditional_t<std::is_trivially_destructible_v<value_type>, literal_container_storage_type<value_type>, non_literal_container_storage_type<value_type>>;
    };
    template<typename value_type>
    using get_container_storage_type_t = typename get_container_storage_type<value_type>::type;


    struct optional_container_storage_trivial_base
    {
    };

    template <typename child_type>
    struct optional_container_storage_non_trivial_base
    {
        optional_container_storage_non_trivial_base() = default;
        ~optional_container_storage_non_trivial_base()
        {
            using value_type = typename child_type::value_type;
            auto* child_ptr = static_cast<child_type*>(this);
            if(child_ptr->has_value)
                child_ptr->value.value.~value_type();
        }
        optional_container_storage_non_trivial_base(optional_container_storage_non_trivial_base&& other)
        {
            using value_type = typename child_type::value_type;
            auto* child_ptr = static_cast<child_type*>(this);

            child_ptr->has_value = other.has_value;

            if(other.has_value)
            {
                child_ptr->value.set(std::move(other.value.value));
                other.value.value.~value_type();
                other.has_value = false;
            }
        }
        optional_container_storage_non_trivial_base& operator=(optional_container_storage_non_trivial_base&& other)
        {
            using value_type = typename child_type::value_type;
            auto* child_ptr = static_cast<child_type*>(this);

            if(child_ptr->has_value)
                child_ptr->value.value.~value_type();
            
            child_ptr->has_value = other.has_value;

            if(other.has_value)
            {
                child_ptr->value.set(std::move(other.value.value));
                other.value.value.~value_type();
                other.has_value = false;
            }

            return *this;
        }
    };

    template <typename value_type_in>
    struct optional_container_storage: std::conditional_t<std::is_trivially_destructible_v<value_type_in>, optional_container_storage_trivial_base, optional_container_storage_non_trivial_base<optional_container_storage<value_type_in>>>
    {
        using value_type = value_type_in;
        constexpr optional_container_storage(): value(), has_value(false) {}
        constexpr optional_container_storage(value_type v): value(std::move(v)), has_value(true) {}
        constexpr optional_container_storage(const optional_container_storage& other): value(), has_value(other.has_value)
        {
            if(other.has_value)
                value.set(other.value.value);
        }
        constexpr optional_container_storage& operator=(const optional_container_storage& other)
        {
            if(has_value)
                value.value.~value_type();
            
            has_value = other.has_value;

            if(other.has_value)
                value.set(other.value.value);

            return *this;
        }

        constexpr value_type& get()
        {
            return value.value;
        }

        const constexpr value_type& get() const
        {
            return value.value;
        }

        using container_storage_type = get_container_storage_type_t<value_type>;
        container_storage_type value;
        bool has_value;
    };

    template <typename value_type>
    struct container_storage
    {
        constexpr container_storage(): value() {}
        constexpr container_storage(value_type v): value(std::move(v)) {}

        constexpr void destroy()
        {
            value.value.~value_type();
        }

        constexpr void set(value_type v)
        {
            value.set(std::move(v));
        }

        constexpr value_type& get()
        {
            return value.value;
        }

        const constexpr value_type& get() const
        {
            return value.value;
        }

        using container_storage_type = get_container_storage_type_t<value_type>;
        container_storage_type value;
    };
}
