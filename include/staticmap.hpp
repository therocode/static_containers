#pragma once
#include <cstdlib>
#include <array>
#include <containerstorage.hpp>

namespace spr
{
template <typename KeyType, typename DataType, size_t Capacity>
class StaticMap
{
    public:
        struct pair
        {
            KeyType first;
            DataType second;
        };

        using ConcreteType = StaticMap<KeyType, DataType, Capacity>;
        using value_type = pair;
        using reference = value_type&;
        using const_reference = const value_type&;
        using pointer = value_type*;
        using const_pointer = const value_type*;

        using key_type = KeyType;
        using mapped_type = DataType;
    private:
        using slot_type = optional_container_storage<value_type>;
    public:

        template<typename ValueType>
        struct IteratorT
        {
            constexpr IteratorT& operator++ ()
            {
                do
                {
                    ++target;
                }while(target < end && !target->has_value);

                return *this;
            }

            constexpr ValueType& operator *() const
            {
                return **target;
            }

            constexpr ValueType* operator->() const
            {
                return &target->get();
            }

            constexpr bool operator==(const IteratorT& other) const
            {
                return target == other.target && end == other.end;
            }

            constexpr bool operator!=(const IteratorT& other) const
            {
                return !(*this == other);
            }

            slot_type* target = nullptr;
            slot_type* end = nullptr;
        };

        using iterator = IteratorT<value_type>;
        using const_iterator = IteratorT<const value_type>;

        constexpr StaticMap() = default;
        template <size_t size>
        constexpr StaticMap(value_type const (&arr)[size])
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
                size_t target = findNextEmptySlot();

                if(target != mStorage.size())
                {
                    slot_type& slot = mStorage[target];
                    slot = slot_type{std::move(entry)};
                    ++mSize;
                    return {{&slot, &(*mStorage.end())}, true};
                }
                else
                {
                    //full. assert?
                    return {end(), false};
                }
            }
        }

        constexpr DataType& operator[] (KeyType key)
        {
            iterator existing = find(key);


            if(existing != end())
            {
                return existing->second;
            }
            else
            {
                size_t target = findNextEmptySlot();

                if(target == mStorage.size())
                {
                    //full. assert?
                }

                slot_type& slot = mStorage[target];
                slot = slot_type{{key, DataType{}}};
                ++mSize;

                return slot.get().second;
            }
        }

        constexpr const DataType& operator[] (const KeyType& key) const
        {
            return const_cast<ConcreteType&>(*this)[key];
        }

        constexpr DataType& at(const KeyType& key)
        {
            return const_cast<DataType&>(const_cast<const ConcreteType&>(*this).at(key));
        }

        constexpr const DataType& at(const KeyType& key) const
        {
            const_iterator found = find(key);
            return found->second;
        }

        constexpr iterator erase(const KeyType& key)
        {
            iterator found = find(key);

            if(found != end())
            {
                size_t index = std::distance(mStorage.data(), found.target); 

                mStorage[index] = {};
                --mSize;

                return ++found;
            }

            return found;
        }

        constexpr iterator find(const KeyType& key)
        {
            iterator endIter = end();

            for(iterator iter = begin(); iter != endIter; ++iter)
            {
                if(iter->first == key)
                {
                    return iter;
                }
            }

            return endIter;
        }

        constexpr const_iterator find(const KeyType& key) const
        {
            iterator found = const_cast<ConcreteType*>(this)->find(key);
            return const_iterator{found.target, found.end};
        }

        constexpr bool contains(const KeyType& key) const
        {
            return find(key) != end();
        }

        constexpr size_t count(const KeyType& key) const
        {
            return static_cast<size_t>(contains(key));
        }

        constexpr void clear()
        {
            mStorage.fill({});
            mSize = 0;
        }

        constexpr size_t size() const
        {
            return mSize;
        }

        constexpr bool empty() const
        {
            return mSize == 0;
        }

        constexpr size_t capacity() const
        {
            return mStorage.size();
        }

        constexpr bool full() const
        {
            return mSize == mStorage.size();
        }

        constexpr iterator begin()
        {
            iterator result;
            result.target = mStorage.end();
            result.end = mStorage.end();

            for(auto& entry : mStorage)
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
            iterator iter = const_cast<ConcreteType*>(this)->begin();
            return const_iterator{iter.target, iter.end};
        }

        constexpr iterator end()
        {
            iterator result;
            result.target = mStorage.end();
            result.end = mStorage.end();

            return result;
        }

        constexpr const_iterator end() const
        {
            iterator iter = const_cast<ConcreteType*>(this)->end();
            return const_iterator{iter.target, iter.end};
        }
    private:
        constexpr size_t findNextEmptySlot() const
        {
            for(size_t i = 0; i < mStorage.size(); ++i)
            {
                if(!mStorage[i].has_value)
                {
                    return i;
                }
            }

            return mStorage.size();
        }

        size_t mSize = 0;
        std::array<slot_type, Capacity> mStorage;
};

template<typename key_type, typename value_type, size_t capacity>
constexpr auto makeStaticMap(typename StaticMap<key_type, value_type, capacity>::value_type const (&arr)[capacity])
{
    return spr::StaticMap<key_type, value_type, capacity>{arr};
}
}
