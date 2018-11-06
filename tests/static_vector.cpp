#include "common.hpp"
#include <stc/static_vector.hpp>

template <typename probe_type, size_t capacity>
void verify_contains(stc::static_vector<probe_type, capacity>& v, std::initializer_list<typename probe_type::value_type> contents)
{
    const auto& cv = v;

    size_t contents_size = contents.size();
    bool empty = contents_size == 0;

    REQUIRE(v.size() == contents_size);

    size_t i = 0;
    for(const auto& item : contents)
    {
        CHECK(v[i] == item);
        CHECK(cv[i] == item);
        CHECK(v.at(i) == item);
        CHECK(cv.at(i) == item);
        ++i;
    }
    CHECK_THROWS(v.at(i));
    CHECK_THROWS(cv.at(i));

    if(!empty)
    {
        const auto& first_item = *contents.begin();
        const auto& last_item = *(contents.begin() + contents_size - 1);

        CHECK(v.front() == first_item);
        CHECK(cv.front() == first_item);
        CHECK(v.back() == last_item);
        CHECK(cv.back() == last_item);
    }

    if(!empty)
    {
        CHECK(v.data() == &v[0]);
        CHECK(cv.data() == &cv[0]);
    }

    auto verify_increment_iteration = [] (auto v_begin, auto v_end, auto contents_begin, auto contents_end)
    {
        auto v_current = v_begin;
        auto contents_current = contents_begin;
        while(v_current != v_end)
        {
            CHECK(*v_current == *contents_current);
            ++v_current;
            ++contents_current;
        }
    };
    auto verify_random_access_iteration = [] (auto v_begin, auto contents_begin, size_t count)
    {
        for(size_t i = 0; i < count; ++i)
        {
            CHECK(*(v_begin + i) == *(contents_begin + i));
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

    CHECK(v.begin() + contents_size == v.end());
    CHECK(cv.begin() + contents_size == cv.end());
    CHECK(v.cbegin() + contents_size == v.cend());
    CHECK(v.rbegin() + contents_size == v.rend());
    CHECK(cv.rbegin() + contents_size == cv.rend());
    CHECK(v.crbegin() + contents_size == v.crend());
    

    CHECK(v.empty() == empty);
    CHECK(v.max_size() == std::vector<probe_type>().max_size());
    CHECK(v.capacity() == capacity);
    CHECK(v.full() == (contents_size == capacity));
}

TEST_CASE("static_vector()", "[static_vector]")
{
    using probe = pr::probe_t<int>;
    no_leak_verifier<probe> no_leak;
    
    verify_semantics<probe>(semantics_flags::NoConstruct, []()
    {
        stc::static_vector<probe, 100> numbers;

        verify_contains(numbers, {});
    });
}

TEST_CASE("explicit static_vector(size_type size, const value_type& data)", "[static_vector]")
{
    using probe = pr::probe_t<int>;
    no_leak_verifier<probe> no_leak;
    
    verify_semantics<probe>(semantics_flags::NoMove, []()
    {
        int value = 3;
        size_t size = 5;
        stc::static_vector<probe, 100> numbers(size, value);

        verify_contains(numbers, {3, 3, 3, 3, 3});
    });
}

TEST_CASE("explicit static_vector(size_type size)", "[static_vector]")
{
    using probe = pr::probe_t<int>;
    no_leak_verifier<probe> no_leak;
    
    verify_semantics<probe>(semantics_flags::NoCopy | semantics_flags::NoMove, []()
    {
        size_t size = 3;
        stc::static_vector<probe, 100> numbers(size);

        verify_contains(numbers, {0, 0, 0});
    });
}

TEST_CASE("static_vector(input_iter start, input_iter end)", "[static_vector]")
{
    using probe = pr::probe_t<int>;
    no_leak_verifier<probe> no_leak;
    
    std::array<int, 5> source{4,3,2,1,0};

    verify_semantics<probe>(semantics_flags::None, [&source]()
    {
        stc::static_vector<probe, 5> numbers(source.begin(), source.end());

        verify_contains(numbers, {4, 3, 2, 1, 0});
    });
}

TEST_CASE("static_vector(const static_vector& other)", "[static_vector]")
{
    using probe = pr::probe_t<int>;
    no_leak_verifier<probe> no_leak;
    
    stc::static_vector<probe, 5> source{4,3,2};

    verify_semantics<probe>(semantics_flags::None, [&source]()
    {
        stc::static_vector<probe, 5> numbers(source);

        verify_contains(numbers, {4, 3, 2});
        REQUIRE(numbers == source);
    });
}

TEST_CASE("static_vector(static_vector&& other)", "[static_vector]")
{
    using probe = pr::probe_t<int>;
    no_leak_verifier<probe> no_leak;
    
    stc::static_vector<probe, 5> source{4,3,2};

    verify_semantics<probe>(semantics_flags::NoCopy, [&source]()
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
