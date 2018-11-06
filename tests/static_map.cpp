#include <catch/catch.hpp>
#include <stc/static_map.hpp>

//TEST_CASE("static_map initial state", "[containers]")
//{
//    stc::static_map<int, int, 50> numbers;
//
//    REQUIRE(numbers.empty());
//    REQUIRE(!numbers.full());
//    REQUIRE(numbers.size() == 0);
//    REQUIRE(numbers.capacity == 50);
//}
//
//TEST_CASE("static_map basic storage and retrieval", "[containers]")
//{
//    stc::static_map<int, int, 50> numbers;
//
//    numbers[0] = 2;
//
//    REQUIRE(numbers.size() == 1);
//
//    auto first_insert = numbers.insert({4, 7});
//
//    REQUIRE(numbers.size() == 2);
//    REQUIRE(first_insert.second);
//    REQUIRE(first_insert.first->first == 4);
//    REQUIRE(first_insert.first->second == 7);
//
//    REQUIRE(numbers.at(0) == 2);
//    REQUIRE(numbers.at(4) == 7);
//    REQUIRE(numbers[0] == 2);
//    REQUIRE(numbers[4] == 7);
//
//    numbers[0] = 45;
//
//    REQUIRE(numbers.size() == 2);
//    REQUIRE(numbers.at(0) == 45);
//
//    auto fail_insert = numbers.insert({4, 4});
//
//    REQUIRE(numbers.size() == 2);
//    REQUIRE(!fail_insert.second);
//}
//
//TEST_CASE("static_map count and contains", "[containers]")
//{
//    stc::static_map<int, int, 50> numbers;
//
//    numbers[7] = 23;
//
//    REQUIRE(numbers.count(0) == 0);
//    REQUIRE(numbers.count(7) == 1);
//    REQUIRE(!numbers.contains(10));
//    REQUIRE(numbers.contains(7));
//}
//
//TEST_CASE("static_map find, begin and end", "[containers]")
//{
//    stc::static_map<int, int, 50> numbers;
//
//    REQUIRE(numbers.begin() == numbers.end());
//
//    numbers = {{{1, 2}, {2, 3}}};
//
//    REQUIRE(numbers.begin() != numbers.end());
//    
//    auto iter = numbers.begin();
//    REQUIRE(iter != numbers.end());
//    REQUIRE(iter->first == 1);
//    REQUIRE(iter->second == 2);
//    ++iter;
//    REQUIRE(iter != numbers.end());
//    REQUIRE(iter->first == 2);
//    REQUIRE(iter->second == 3);
//    ++iter;
//    REQUIRE(iter == numbers.end());
//
//    auto found = numbers.find(2);
//
//    REQUIRE(found != numbers.end());
//    REQUIRE(found->first == 2);
//    REQUIRE(found->second == 3);
//
//    auto not_found = numbers.find(23);
//    REQUIRE(not_found == numbers.end());
//}
//
//TEST_CASE("static_map clear", "[containers]")
//{
//    stc::static_map<int, int, 50> numbers = {{{2, 3}, {5, 8}}};
//
//    REQUIRE(numbers.size() == 2);
//    REQUIRE(!numbers.empty());
//
//    numbers.clear();
//
//    REQUIRE(numbers.size() == 0);
//    REQUIRE(numbers.empty());
//}
//
//TEST_CASE("static_map full", "[containers]")
//{
//    stc::static_map<int, int, 3> numbers;
//
//    REQUIRE(!numbers.full());
//    numbers[1] = 23;
//    REQUIRE(!numbers.full());
//    numbers[5] = 231;
//    REQUIRE(!numbers.full());
//    numbers[111] = 234;
//    REQUIRE(numbers.full());
//}
