#include <catch/catch.hpp>
#include <probe/probe.hpp>
#include <stc/static_vector.hpp>

#include <iostream>

namespace semantics_flags
{
    using type = int;
    constexpr type None = 0;
    constexpr type NoLeaks = 1;
    constexpr type NoConstruct = 1 << 1;
    constexpr type NoCopy = 1 << 2;
    constexpr type NoMove = 1 << 3;
}

template <typename probe, typename functor>
void verify_semantics(semantics_flags::type flags, functor f)
{
    probe::reset();

    f();

    auto counts = probe::last();

    if(flags & semantics_flags::NoLeaks)
        REQUIRE(probe::clean());
    if(flags & semantics_flags::NoConstruct)
        REQUIRE((counts.defco == 0 && counts.valco == 0 && counts.movco == 0 && counts.copco == 0));
    if(flags & semantics_flags::NoCopy)
        REQUIRE(counts.copies == 0);
    if(flags & semantics_flags::NoMove)
        REQUIRE(counts.moves == 0);
}

template <typename probe_type, size_t capacity>
void verify_contains(stc::static_vector<probe_type, capacity>& v, std::initializer_list<typename probe_type::value_type> contents)
{
    const auto& cv = v;

    size_t contents_size = contents.size();
    bool empty = contents_size == 0;

    size_t i = 0;
    for(const auto& item : contents)
    {
        REQUIRE(v[i] == item);
        REQUIRE(cv[i] == item);
        REQUIRE(v.at(i) == item);
        REQUIRE(cv.at(i) == item);
        ++i;
    }
    REQUIRE_THROWS(v.at(i));
    REQUIRE_THROWS(cv.at(i));

    if(!empty)
    {
        const auto& first_item = *contents.begin();
        const auto& last_item = *(contents.begin() + contents_size - 1);

        REQUIRE(v.front() == first_item);
        REQUIRE(cv.front() == first_item);
        REQUIRE(v.back() == last_item);
        REQUIRE(cv.back() == last_item);
    }
    else
    {
        REQUIRE_THROWS(v.front());
        REQUIRE_THROWS(cv.front());
        REQUIRE_THROWS(v.back());
        REQUIRE_THROWS(cv.back());
    }

    if(!empty)
    {
        REQUIRE(v.data() == &v[0]);
        REQUIRE(cv.data() == &cv[0]);
    }

    auto verify_increment_iteration = [] (auto v_begin, auto v_end, auto contents_begin, auto contents_end)
    {
        auto v_current = v_begin;
        auto contents_current = contents_begin;
        while(v_current != v_end)
        {
            REQUIRE(*v_current == *contents_current);
            ++v_current;
            ++contents_current;
        }
    };
    auto verify_random_access_iteration = [] (auto v_begin, auto contents_begin, size_t count)
    {
        for(size_t i = 0; i < count; ++i)
        {
            REQUIRE(*(v_begin + i) == *(contents_begin + i));
        }
    };

    verify_increment_iteration(v.begin(), v.end(), contents.begin(), contents.end());
    verify_increment_iteration(v.cbegin(), v.cend(), contents.begin(), contents.end());
    verify_increment_iteration(v.rbegin(), v.rend(), std::rbegin(contents), std::rend(contents));
    verify_increment_iteration(v.crbegin(), v.crend(), std::rbegin(contents), std::rend(contents));

    verify_random_access_iteration(v.begin(), contents.begin(), contents_size);
    verify_random_access_iteration(v.cbegin(), contents.begin(), contents_size);
    verify_random_access_iteration(v.rbegin(), std::rbegin(contents), contents_size);
    verify_random_access_iteration(v.crbegin(), std::rbegin(contents), contents_size);

    REQUIRE(v.begin() + contents_size == v.end());
    REQUIRE(cv.begin() + contents_size == cv.end());
    REQUIRE(v.cbegin() + contents_size == v.cend());
    REQUIRE(v.rbegin() + contents_size == v.rend());
    REQUIRE(cv.rbegin() + contents_size == cv.rend());
    REQUIRE(v.crbegin() + contents_size == v.crend());
    

    REQUIRE(v.empty() == empty);
    REQUIRE(v.size() == contents_size);
    REQUIRE(v.max_size() == std::vector<typename probe_type::value_type>().max_size());
    REQUIRE(v.capacity() == capacity);
    REQUIRE(v.full() == (contents_size == capacity));

    stc::static_vector<probe_type, capacity> ideal_vec(contents.begin(), contents.end());
    REQUIRE(v == ideal_vec);
}

TEST_CASE("static_vector()", "[static_vector]")
{
    using probe = pr::probe_t<int>;probe::reset();
    pr::probe_counts counts;
    {
        stc::static_vector<probe, 100> numbers;

        counts = probe::last();

        REQUIRE(numbers.size() == 0);
    }

    REQUIRE(counts.defco == 0);
    REQUIRE(probe::clean());
}

TEST_CASE("explicit static_vector(size_type size, const value_type& data)", "[static_vector]")
{
    using probe = pr::probe_t<int>;probe::reset();
    {
        int value = 3;
        size_t size = 5;
        stc::static_vector<probe, 100> numbers(size, value);

        REQUIRE(numbers.size() == size);
        REQUIRE(numbers[0] == value);
        REQUIRE(numbers[size - 1] == value);
    }

    REQUIRE(probe::clean());
}

TEST_CASE("explicit static_vector(size_type size)", "[static_vector]")
{
    using probe = pr::probe_t<int>;probe::reset();
    {
        size_t size = 3;
        stc::static_vector<probe, 100> numbers(size);

        REQUIRE(numbers.size() == size);
        REQUIRE(numbers[0] == 0);
        REQUIRE(numbers[size - 1] == 0);
    }

    REQUIRE(probe::clean());
}

TEST_CASE("static_vector(input_iter start, input_iter end)", "[static_vector]")
{
    using probe = pr::probe_t<int>;probe::reset();
    {
        std::array<int, 5> source{4,3,2,1,0};

        stc::static_vector<probe, 100> numbers(source.begin(), source.end());

        REQUIRE(numbers.size() == source.size());
        REQUIRE(numbers[0] == 4);
        REQUIRE(numbers[1] == 3);
        REQUIRE(numbers[2] == 2);
        REQUIRE(numbers[3] == 1);
        REQUIRE(numbers[4] == 0);
    }

    REQUIRE(probe::clean());
}

TEST_CASE("static_vector(const static_vector& other)", "[static_vector]")
{
    using probe = pr::probe_t<int>;probe::reset();
    {
        stc::static_vector<probe, 5> source{4,3,2};

        stc::static_vector<probe, 5> numbers(source);

        verify_contains(numbers, {4, 3, 2});
        REQUIRE(numbers == source);
    }

    REQUIRE(probe::clean());
}

TEST_CASE("static_vector(static_vector&& other)", "[static_vector]")
{
    using probe = pr::probe_t<int>;
    
    stc::static_vector<probe, 5> source{4,3,2};

    verify_semantics<probe>(semantics_flags::NoLeaks | semantics_flags::NoCopy, [&source]()
    {
        stc::static_vector<probe, 5> numbers(std::move(source));

        verify_contains(numbers, {4, 3, 2});
        verify_contains(source, {});
    });
}
//TEST_CASE("static_vector can be created filled with a size", "[containers]")
//{
//    stc::static_vector<int, 100> numbers(3, 1);
//
//    REQUIRE(numbers.size() == 3);
//    REQUIRE(numbers[0] == 1);
//    REQUIRE(numbers[1] == 1);
//    REQUIRE(numbers[2] == 1);
//}
//
//TEST_CASE("static_vector can be constructed from initializer_list", "[containers]")
//{
//    stc::static_vector<int, 50> numbers = {0, 1, 2, 3};
//
//    REQUIRE(numbers.size() == 4);
//    REQUIRE(numbers[0] == 0);
//    REQUIRE(numbers[1] == 1);
//    REQUIRE(numbers[2] == 2);
//    REQUIRE(numbers[3] == 3);
//}
//
//TEST_CASE("items added to static vectors increase the size", "[containers]")
//{
//    stc::static_vector<int, 100> numbers;
//
//    REQUIRE(numbers.size() == 0);
//    REQUIRE(numbers.empty());
//
//    numbers.push_back(12);
//
//    REQUIRE(numbers.size() == 1);
//    REQUIRE(!numbers.empty());
//
//    numbers.emplace_back(324);
//
//    REQUIRE(numbers.size() == 2);
//}
//
//TEST_CASE("items added to static vectors can make them full", "[containers]")
//{
//    stc::static_vector<int, 3> numbers;
//
//    REQUIRE(!numbers.full());
//    numbers.push_back(12);
//    REQUIRE(!numbers.full());
//    numbers.push_back(12);
//    REQUIRE(!numbers.full());
//    numbers.push_back(12);
//    REQUIRE(numbers.full());
//}
//
//TEST_CASE("items added to static vectors can be retrieved through random access", "[containers]")
//{
//    stc::static_vector<int, 50> numbers;
//
//    numbers.push_back(12);
//    REQUIRE(numbers[0] == 12);
//
//    numbers.emplace_back(321);
//
//    REQUIRE(numbers[0] == 12);
//    REQUIRE(numbers[1] == 321);
//
//    REQUIRE(numbers.front() == 12);
//    REQUIRE(((const stc::static_vector<int, 50>&)numbers).front() == 12);
//    REQUIRE(numbers.back() == 321);
//    REQUIRE(((const stc::static_vector<int, 50>&)numbers).back() == 321);
//}
//
//TEST_CASE("items added to static vectors can be retreived using iterators, including range based for", "[containers]")
//{
//    stc::static_vector<int, 50> numbers;
//
//    numbers.push_back(12);
//    REQUIRE(*numbers.begin() == 12);
//
//    numbers.emplace_back(321);
//
//    REQUIRE(*numbers.begin() == 12);
//    REQUIRE(*(numbers.begin() + 1) == 321);
//    REQUIRE(*(numbers.end() - 1) == 321);
//    REQUIRE(*(numbers.end() - 2) == 12);
//
//    size_t counter = 0;
//    for(int entry : numbers)
//    {
//        if(counter == 0)
//            REQUIRE(entry == 12);
//        else if(counter == 1)
//            REQUIRE(entry == 321);
//        ++counter;
//    }
//}
//
//TEST_CASE("items removed from static vectors reduce the size and move items accordingly", "[containers]")
//{
//    stc::static_vector<int, 50> numbers;
//
//    numbers.push_back(1);
//    numbers.push_back(2);
//    numbers.push_back(3);
//    numbers.push_back(4);
//    numbers.push_back(5);
//
//    auto past_erased = numbers.erase(numbers.begin() + 2);
//
//    REQUIRE(numbers[2] == 4);
//    REQUIRE(numbers[3] == 5);
//    REQUIRE(*past_erased == 4);
//
//    numbers.pop_back();
//
//    REQUIRE(numbers[2] == 4);
//    REQUIRE(numbers.size() == 3);
//}
//
//TEST_CASE("items can be added to static vectors at any index using 'insert'", "[containers]")
//{
//    stc::static_vector<int, 50> numbers;
//
//    numbers.insert(numbers.end(), 43);
//
//    REQUIRE(numbers.size() == 1);
//    REQUIRE(numbers[0] == 43);
//
//    auto added = numbers.insert(numbers.begin(), 42);
//
//    REQUIRE(numbers[0] == 42);
//    REQUIRE(numbers[1] == 43);
//    REQUIRE(*added == 42);
//}
//
//TEST_CASE("items added to static vectors have their constructors and destructors called appropriately", "[containers]")
//{
//}
//
//TEST_CASE("STL algorithms functions on static vector", "[containers]")
//{
//    stc::static_vector<int, 50> numbers = {4, 2, 3, 0, 2};
//
//    std::sort(numbers.begin(), numbers.end());
//
//    REQUIRE(numbers[0] == 0);
//    REQUIRE(numbers[1] == 2);
//    REQUIRE(numbers[2] == 2);
//    REQUIRE(numbers[3] == 3);
//    REQUIRE(numbers[4] == 4);
//}
