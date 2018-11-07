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

    verify_semantics<probe>(semantics_flags::NoMove, [&source]()
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

TEST_CASE("static_vector(std::initializer_list<value_type> data)", "[static_vector]")
{
    using probe = pr::probe_t<int>;
    no_leak_verifier<probe> no_leak;
    
    verify_semantics<probe>(semantics_flags::None, []()
    {
        stc::static_vector<probe, 5> numbers({1, 3, 3, 7});

        verify_contains(numbers, {1, 3, 3, 7});
    });
}

TEST_CASE("static_vector& operator=(const static_vector& other)", "[static_vector]")
{
    using probe = pr::probe_t<int>;
    no_leak_verifier<probe> no_leak;

    stc::static_vector<probe, 5> source{4, 4, 4, 1};
    
    verify_semantics<probe>(semantics_flags::NoMove, [&source]()
    {
        stc::static_vector<probe, 5> numbers;
        auto return_ptr = &(numbers = source);

        verify_contains(numbers, {4, 4, 4, 1});
        REQUIRE(return_ptr == &numbers);
    });
}

TEST_CASE("static_vector& operator=(static_vector&& other)", "[static_vector]")
{
    using probe = pr::probe_t<int>;
    no_leak_verifier<probe> no_leak;

    stc::static_vector<probe, 5> source{1, 4, 2, 1};
    stc::static_vector<probe, 5> numbers{9, 3};
    
    verify_semantics<probe>(semantics_flags::NoCopy, [&source, &numbers]()
    {
        auto return_ptr = &(numbers = std::move(source));

        verify_contains(numbers, {1, 4, 2, 1});
        verify_contains(source, {});
        REQUIRE(return_ptr == &numbers);
    });
}

TEST_CASE("static_vector& operator=(std::initializer_list<value_type> data)", "[static_vector]")
{
    using probe = pr::probe_t<int>;
    no_leak_verifier<probe> no_leak;

    verify_semantics<probe>(semantics_flags::NoMove, []()
    {
        stc::static_vector<probe, 5> numbers{1, 2};
        auto return_ptr = &(numbers = {5, 7, 8});

        verify_contains(numbers, {5, 7, 8});
        REQUIRE(return_ptr == &numbers);
    });
}

TEST_CASE("void assign(size_type count, const value_type& value)", "[static_vector]")
{
    using probe = pr::probe_t<int>;
    no_leak_verifier<probe> no_leak;

    verify_semantics<probe>(semantics_flags::NoMove, []()
    {
        size_t size = 3;
        int value = 8;
        stc::static_vector<probe, 5> numbers{2, 4};
        numbers.assign(size, value);

        verify_contains(numbers, {8, 8, 8});
    });
}

TEST_CASE("void assign(input_iter start, input_iter end)", "[static_vector]")
{
    using probe = pr::probe_t<int>;
    no_leak_verifier<probe> no_leak;

    std::array<int, 4> source{9, 4, 9, 1};

    verify_semantics<probe>(semantics_flags::NoMove, [&source]()
    {
        stc::static_vector<probe, 5> numbers{2, 4};
        numbers.assign(source.begin(), source.end());

        verify_contains(numbers, {9, 4, 9, 1});
    });
}

TEST_CASE("void assign(std::initializer_list<value_type> data)", "[static_vector]")
{
    using probe = pr::probe_t<int>;
    no_leak_verifier<probe> no_leak;

    verify_semantics<probe>(semantics_flags::NoMove, []()
    {
        stc::static_vector<probe, 5> numbers{2, 4};
        numbers.assign({6, 7, 5});

        verify_contains(numbers, {6, 7, 5});
    });
}

TEST_CASE("reference at(size_type index)", "[static_vector]")
{
    using probe = pr::probe_t<int>;
    no_leak_verifier<probe> no_leak;

    stc::static_vector<probe, 5> numbers{2, 4};

    verify_semantics<probe>(semantics_flags::NoCopy, [&numbers]()
    {
        numbers.at(0) = 8;
        numbers.at(1) = 1;
    });

    verify_contains(numbers, {8, 1});
}

TEST_CASE("reference operator[](size_type index)", "[static_vector]")
{
    using probe = pr::probe_t<int>;
    no_leak_verifier<probe> no_leak;

    stc::static_vector<probe, 5> numbers{8, 0, 5};

    verify_semantics<probe>(semantics_flags::NoCopy, [&numbers]()
    {
        numbers[0] = 7;
        numbers[1] = 2;
    });

    verify_contains(numbers, {7, 2, 5});
}

TEST_CASE("reference front()", "[static_vector]")
{
    using probe = pr::probe_t<int>;
    no_leak_verifier<probe> no_leak;

    stc::static_vector<probe, 5> numbers{8, 0, 5};

    verify_semantics<probe>(semantics_flags::NoCopy, [&numbers]()
    {
        numbers.front() = 1;
    });

    verify_contains(numbers, {1, 0, 5});
}

TEST_CASE("reference back()", "[static_vector]")
{
    using probe = pr::probe_t<int>;
    no_leak_verifier<probe> no_leak;

    stc::static_vector<probe, 5> numbers{8, 0, 5};

    verify_semantics<probe>(semantics_flags::NoCopy, [&numbers]()
    {
        numbers.back() = 1;
    });

    verify_contains(numbers, {8, 0, 1});
}

TEST_CASE("value_type* data() noexcept", "[static_vector]")
{
    using probe = pr::probe_t<int>;
    no_leak_verifier<probe> no_leak;

    stc::static_vector<probe, 5> numbers{8, 0, 5};

    verify_semantics<probe>(semantics_flags::NoCopy, [&numbers]()
    {
        auto data_ptr = numbers.data();
        data_ptr[0] = 1;
        data_ptr[1] = 2;
        data_ptr[2] = 3;
    });

    verify_contains(numbers, {1, 2, 3});
}

TEST_CASE("iterator begin()", "[static_vector]")
{
    using probe = pr::probe_t<int>;
    no_leak_verifier<probe> no_leak;

    stc::static_vector<probe, 5> numbers{7, 9, 8};

    verify_semantics<probe>(semantics_flags::NoCopy, [&numbers]()
    {
        *numbers.begin() = 5;
        *(numbers.begin() + 1) = 7;
    });

    verify_contains(numbers, {5, 7, 8});
}

TEST_CASE("iterator end()", "[static_vector]")
{
    using probe = pr::probe_t<int>;
    no_leak_verifier<probe> no_leak;

    stc::static_vector<probe, 5> numbers{8, 9, 1};

    verify_semantics<probe>(semantics_flags::NoCopy, [&numbers]()
    {
        *(numbers.end() - 1) = 5;
        *(numbers.end() - 2) = 7;
    });

    verify_contains(numbers, {8, 7, 5});
}

TEST_CASE("reverse_iterator rbegin()", "[static_vector]")
{
    using probe = pr::probe_t<int>;
    no_leak_verifier<probe> no_leak;

    stc::static_vector<probe, 5> numbers{8, 9, 1};

    verify_semantics<probe>(semantics_flags::NoCopy, [&numbers]()
    {
        *numbers.rbegin() = 5;
        *(numbers.rbegin() + 1) = 7;
    });

    verify_contains(numbers, {8, 7, 5});
}

TEST_CASE("iterator rend()", "[static_vector]")
{
    using probe = pr::probe_t<int>;
    no_leak_verifier<probe> no_leak;

    stc::static_vector<probe, 5> numbers{8, 9, 1};

    verify_semantics<probe>(semantics_flags::NoCopy, [&numbers]()
    {
        *(numbers.rend() - 1) = 4;
        *(numbers.rend() - 2) = 6;
    });

    verify_contains(numbers, {4, 6, 1});
}

TEST_CASE("void clear()", "[static_vector]")
{
    using probe = pr::probe_t<int>;
    no_leak_verifier<probe> no_leak;

    stc::static_vector<probe, 5> numbers{8, 9, 1};

    verify_semantics<probe>(semantics_flags::NoConstruct, [&numbers]()
    {
        numbers.clear();
    });

    verify_contains(numbers, {});
}

TEST_CASE("iterator insert(const_iterator cposition, const value_type& value)", "[static_vector]")
{
    using probe = pr::probe_t<int>;
    no_leak_verifier<probe> no_leak;

    stc::static_vector<probe, 5> numbers{1, 2};

    verify_semantics<probe>(semantics_flags::None, [&numbers]()
    {
        auto ret1 = numbers.insert(numbers.begin(), 3);
        auto ret2 = numbers.insert(numbers.end(), 6);

        REQUIRE(ret1 == numbers.begin());
        REQUIRE(ret2 == numbers.begin() + 3);
    });

    verify_contains(numbers, {3, 1, 2, 6});
}

TEST_CASE("iterator insert(const_iterator cposition, value_type&& value)", "[static_vector]")
{
    using probe = pr::probe_t<int>;
    no_leak_verifier<probe> no_leak;

    stc::static_vector<probe, 5> numbers{1, 2};

    verify_semantics<probe>(semantics_flags::None, [&numbers]()
    {
        auto ret1 = numbers.insert(numbers.begin(), 3);
        auto ret2 = numbers.insert(numbers.end(), 6);

        REQUIRE(ret1 == numbers.begin());
        REQUIRE(ret2 == numbers.begin() + 3);
    });

    verify_contains(numbers, {3, 1, 2, 6});
}

TEST_CASE("iterator insert(const_iterator cposition, size_type count, const value_type& value)", "[static_vector]")
{
    using probe = pr::probe_t<int>;
    no_leak_verifier<probe> no_leak;

    stc::static_vector<probe, 5> numbers{1};

    verify_semantics<probe>(semantics_flags::None, [&numbers]()
    {
        size_t count = 2;
        auto ret1 = numbers.insert(numbers.begin(), count, 3);
        auto ret2 = numbers.insert(numbers.end(), count, 6);

        REQUIRE(ret1 == numbers.begin());
        REQUIRE(ret2 == numbers.begin() + 3);
    });

    verify_contains(numbers, {3, 3, 1, 6, 6});
}

TEST_CASE("iterator insert(const_iterator cposition, input_iter first, input_iter last)", "[static_vector]")
{
    using probe = pr::probe_t<int>;
    no_leak_verifier<probe> no_leak;

    stc::static_vector<probe, 5> numbers{1};
    std::array<int, 2> source1{4,3};
    std::array<int, 2> source2{7,8};

    verify_semantics<probe>(semantics_flags::None, [&numbers, &source1, &source2]()
    {
        auto ret1 = numbers.insert(numbers.begin(), source1.begin(), source1.end());
        auto ret2 = numbers.insert(numbers.end(), source2.begin(), source2.end());

        REQUIRE(ret1 == numbers.begin());
        REQUIRE(ret2 == numbers.begin() + 3);
    });

    verify_contains(numbers, {4, 3, 1, 7, 8});
}

TEST_CASE("iterator insert(const_iterator cposition, std::initializer_list<value_type> ilist)", "[static_vector]")
{
    using probe = pr::probe_t<int>;
    no_leak_verifier<probe> no_leak;

    stc::static_vector<probe, 5> numbers{1};

    verify_semantics<probe>(semantics_flags::None, [&numbers]()
    {
        auto ret1 = numbers.insert(numbers.begin(), {6, 7});
        auto ret2 = numbers.insert(numbers.end(), {2, 4});

        REQUIRE(ret1 == numbers.begin());
        REQUIRE(ret2 == numbers.begin() + 3);
    });

    verify_contains(numbers, {6, 7, 1, 2, 4});
}

TEST_CASE("iterator emplace(const_iterator cposition, Args&&... args)", "[static_vector]")
{
    using probe = pr::probe_t<int>;
    no_leak_verifier<probe> no_leak;

    stc::static_vector<probe, 5> numbers{1};

    verify_semantics<probe>(semantics_flags::NoCopy, [&numbers]()
    {
        auto ret1 = numbers.emplace(numbers.begin(), 0);
        auto ret2 = numbers.emplace(numbers.begin() + 1, 9);
        auto ret3 = numbers.emplace(numbers.end(), 7);

        REQUIRE(ret1 == numbers.begin());
        REQUIRE(ret2 == numbers.begin() + 1);
        REQUIRE(ret3 == numbers.begin() + 3);
    });

    verify_contains(numbers, {0, 9, 1, 7});
}

TEST_CASE("iterator erase(const_iterator cposition)", "[static_vector]")
{
    using probe = pr::probe_t<int>;
    no_leak_verifier<probe> no_leak;

    stc::static_vector<probe, 5> numbers{0, 1, 2, 3, 4};

    verify_semantics<probe>(semantics_flags::None, [&numbers]()
    {
        auto ret1 = numbers.erase(numbers.begin());
        auto ret2 = numbers.erase(numbers.begin() + 1);

        REQUIRE(ret1 == numbers.begin());
        REQUIRE(ret2 == numbers.begin() + 1);
    });

    verify_contains(numbers, {1, 3, 4});
}

TEST_CASE("iterator erase(const_iterator cerase_start, const_iterator cerase_end)", "[static_vector]")
{
    using probe = pr::probe_t<int>;
    no_leak_verifier<probe> no_leak;

    stc::static_vector<probe, 5> numbers{0, 1, 2, 3, 4};

    verify_semantics<probe>(semantics_flags::None, [&numbers]()
    {
        auto ret1 = numbers.erase(numbers.begin(), numbers.begin() + 2);
        auto ret2 = numbers.erase(numbers.begin() + 1, numbers.end());

        REQUIRE(ret1 == numbers.begin());
        REQUIRE(ret2 == numbers.begin() + 1);
    });

    verify_contains(numbers, {2});
}

TEST_CASE("reference push_back(const value_type& new_entry)", "[static_vector]")
{
    using probe = pr::probe_t<int>;
    no_leak_verifier<probe> no_leak;

    stc::static_vector<probe, 5> numbers{};

    probe val1 = 8;
    probe val2 = 6;
    probe val3 = 4;

    verify_semantics<probe>(semantics_flags::NoMove, [&numbers, &val1, &val2, &val3]()
    {
        auto& ret1 = numbers.push_back(val1);
        auto& ret2 = numbers.push_back(val2);
        auto& ret3 = numbers.push_back(val3);

        REQUIRE(ret1 == 8);
        REQUIRE(ret2 == 6);
        REQUIRE(ret3 == 4);
    });

    verify_contains(numbers, {8, 6, 4});
}

TEST_CASE("reference push_back(value_type&& new_entry)", "[static_vector]")
{
    using probe = pr::probe_t<int>;
    no_leak_verifier<probe> no_leak;

    stc::static_vector<probe, 5> numbers{};

    verify_semantics<probe>(semantics_flags::NoCopy, [&numbers]()
    {
        auto& ret1 = numbers.push_back(3);
        auto& ret2 = numbers.push_back(2);
        auto& ret3 = numbers.push_back(1);

        REQUIRE(ret1 == 3);
        REQUIRE(ret2 == 2);
        REQUIRE(ret3 == 1);
    });

    verify_contains(numbers, {3, 2, 1});
}

TEST_CASE("reference emplace_back(Args&&... args", "[static_vector]")
{
    using probe = pr::probe_t<int>;
    no_leak_verifier<probe> no_leak;

    stc::static_vector<probe, 5> numbers{};

    verify_semantics<probe>(semantics_flags::NoCopy, [&numbers]()
    {
        auto& ret1 = numbers.emplace_back(4);
        auto& ret2 = numbers.emplace_back(3);
        auto& ret3 = numbers.emplace_back(2);

        REQUIRE(ret1 == 4);
        REQUIRE(ret2 == 3);
        REQUIRE(ret3 == 2);
    });

    verify_contains(numbers, {4, 3, 2});
}

TEST_CASE("void pop_back()", "[static_vector]")
{
    using probe = pr::probe_t<int>;
    no_leak_verifier<probe> no_leak;

    stc::static_vector<probe, 5> numbers{5, 6, 7};

    verify_semantics<probe>(semantics_flags::NoConstruct, [&numbers]()
    {
        numbers.pop_back();
        numbers.pop_back();
    });

    verify_contains(numbers, {5});
}

TEST_CASE("void resize(size_type new_size)", "[static_vector]")
{
    using probe = pr::probe_t<int>;
    no_leak_verifier<probe> no_leak;

    stc::static_vector<probe, 5> numbers{5, 6, 7};

    verify_semantics<probe>(semantics_flags::NoConstruct, [&numbers]()
    {
        numbers.resize(3);
    });

    verify_contains(numbers, {5, 6, 7});

    verify_semantics<probe>(semantics_flags::NoConstruct, [&numbers]()
    {
        numbers.resize(1);
    });

    verify_contains(numbers, {5});

    verify_semantics<probe>(semantics_flags::NoCopy | semantics_flags::NoMove, [&numbers]()
    {
        numbers.resize(5);
    });

    verify_contains(numbers, {5, 0, 0, 0, 0});
}

TEST_CASE("void resize(size_type new_size, const value_type& value)", "[static_vector]")
{
    using probe = pr::probe_t<int>;
    no_leak_verifier<probe> no_leak;

    stc::static_vector<probe, 5> numbers{5, 6, 7};

    probe value = 3;

    verify_semantics<probe>(semantics_flags::NoConstruct, [&numbers, &value]()
    {
        numbers.resize(3, value);
    });

    verify_contains(numbers, {5, 6, 7});

    value = 2;

    verify_semantics<probe>(semantics_flags::NoConstruct, [&numbers, &value]()
    {
        numbers.resize(1, value);
    });

    verify_contains(numbers, {5});

    value = 9;

    verify_semantics<probe>(semantics_flags::NoMove, [&numbers, &value]()
    {
        numbers.resize(5, value);
    });

    verify_contains(numbers, {5, 9, 9, 9, 9});
}
