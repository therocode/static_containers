#pragma once
#include <cstddef>
#include <utility>
#include <type_traits>
#include <cstdint>
#include <array>
#include <containerstorage.hpp>

namespace spr
{
    class StaticVectorTrivialBase {};
    template <typename Child>
    class StaticVectorNonTrivialBase
    {
        public:
            StaticVectorNonTrivialBase() = default;
            ~StaticVectorNonTrivialBase()
            {
                Child* child_ptr = static_cast<Child*>(this);
                for(size_t i = 0; i < child_ptr->size(); ++i)
                    child_ptr->mData[i].destroy();
            }
    };

    template <typename Data, size_t tCapacity>
    class StaticVector: public std::conditional_t<std::is_trivially_destructible_v<Data>, StaticVectorTrivialBase, StaticVectorNonTrivialBase<StaticVector<Data, tCapacity>>>
    {
        using BaseClass = std::conditional_t<std::is_trivially_destructible_v<Data>, StaticVectorTrivialBase, StaticVectorNonTrivialBase<StaticVector<Data, tCapacity>>>;
        friend BaseClass::~BaseClass();

        public:
            using value_type = Data;
            using iterator = value_type*;
            using const_iterator = const value_type*;
            using reference = value_type&;
            using const_reference = const value_type&;
            using pointer = value_type*;
            using const_pointer = const value_type*;

            static constexpr size_t capacity = tCapacity;
            constexpr StaticVector():
                mSize(0)
            {
            }
            constexpr StaticVector(size_t size):
                mSize(size)
            {
                for(size_t i = 0; i < mSize; ++i)
                    (*this)[i] = Data{};
            }
            constexpr StaticVector(size_t size, const Data& data):
                mSize(size)
            {
                for(size_t i = 0; i < mSize; ++i)
                    (*this)[i] = data;
            }
            constexpr StaticVector(std::initializer_list<Data> data):
                mSize(data.size())
            {
                for(size_t i = 0; i < mSize; ++i)
                    mData[i].set(*(data.begin() + i));
            }
            template <size_t size>
            constexpr StaticVector(value_type const (&arr)[size]):
                mSize(arr.size())
            {
                for(size_t i = 0; i < size; ++i)
                    mData[i].set(arr[i]);
            }
            //TBI COPY/MOVE  .... LEAKS RIGHT NOW
            constexpr void pushBack(Data newEntry)
            {
                mData[mSize] = std::move(newEntry);
                ++mSize;
                //ASSERT(mSize <= tCapacity, "adding entry to full static vector of size " << tCapacity << "\n");
            }
            template <typename ...Args>
            constexpr void emplaceBack(Args&&... args)
            {
                mData[mSize] = Data(std::forward<Args>(args)...);
                ++mSize;
                //ASSERT(mSize <= tCapacity, "adding entry to full static vector of size " << tCapacity << "\n");
            }
            constexpr bool empty() const
            {
                return mSize == 0;
            }
            constexpr bool full() const
            {
                return mSize == tCapacity;
            }
            constexpr size_t size() const
            {
                return mSize;
            }
            constexpr const Data& operator[](size_t index) const
            {
                return mData[index].get();
            }
            constexpr Data& operator[](size_t index)
            {
                return mData[index].get();
            }
            constexpr const Data& front() const
            {
                return mData[0].get();
            }
            constexpr Data& front()
            {
                return mData[0].get();
            }
            constexpr const Data& back() const
            {
                return mData[mSize - 1].get();
            }
            constexpr Data& back()
            {
                return mData[mSize - 1].get();
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
                return begin() + mSize;
            }
            constexpr iterator end()
            {
                return begin() + mSize;
            }
            constexpr iterator erase(const Data* position)
            {
                size_t index = position - begin();

                //ASSERT(index < mSize, "trying to erase out of bounds or with bad iterator. iter: " << position << " index: " << index << "\n");

                (*this)[index].~Data();

                for(size_t i = index; i < mSize - 1; ++i)
                {
                    (*this)[i] = std::move((*this)[i + 1]);
                }

                --mSize;

                return &(*this)[index];
            }
            constexpr void clear()
            {
                *this = {};
            }
            constexpr void popBack()
            {
                //ASSERT(mSize > 0, "trying to popBack an empty static vector");

                (*this)[mSize - 1].~Data();
                --mSize;
            }
            constexpr iterator insert(const_iterator position, Data value)
            {
                size_t targetIndex = position - begin();

                //ASSERT(targetIndex <= mSize, "trying to insert out of bounds or with bad iterator. iter: " << position << " index: " << targetIndex << "\n");

                if(mSize > 0)
                {
                    for(int64_t i = static_cast<int64_t>(mSize) - 1; i >= static_cast<int64_t>(targetIndex); --i)
                    {
                        (*this)[static_cast<size_t>(i + 1)] = std::move((*this)[static_cast<size_t>(i)]);
                    }
                }

                (*this)[targetIndex] = std::move(value);

                ++mSize;
                return &(*this)[targetIndex];
            }
            constexpr void resize(size_t newSize)
            {
                if(newSize < mSize)
                {
                    size_t firstErase = newSize;

                    for(size_t i = firstErase; i < mSize; ++i)
                        (*this)[i].~Data();
                }

                mSize = newSize;
            }
        private:
            using storage_type = container_storage<value_type>;
            std::array<storage_type, tCapacity> mData;
            size_t mSize;
    };

    template<typename Data, size_t tCapacity>
    constexpr typename StaticVector<Data, tCapacity>::iterator eraseByValue(StaticVector<Data, tCapacity>& vec, const Data& toErase)
    {
        for(size_t i = 0; i < vec.size(); ++i)
        {
            if(vec[i] == toErase)
                return vec.erase(vec.begin() + i);
        }

        return vec.end();
    }
}
