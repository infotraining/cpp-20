#include "catch.hpp"
#include <algorithm>
#include <tuple>
#include <optional>
#include <iostream>

using namespace std::literals;

template <size_t N>
constexpr auto create_array()
{
    std::array<int, N> arr{};
    std::iota(arr.begin(), arr.end(), 1);

    return arr;
}

TEST_CASE("constexpr algorithms")
{
    constexpr auto data = create_array<10>();    

    constexpr auto squares = [&data] {  
        std::array results = data;
        std::transform(begin(results), end(results), begin(results), [](int x) { return x * x;});
        return results;
    }();
    
    static_assert(*std::upper_bound(begin(squares), end(squares), 50) == 64);        

    constexpr auto sum = std::accumulate(begin(squares), end(squares), 0);

    static_assert(sum == 385);        
}

// constexpr auto tokenize(std::string_view text)
// {
//     std::vector<std::string_view> tokens;
//     return 
// }

// TEST_CASE("constexpr tokenizing")
// {
//     constexpr auto text = "one two,three"sv;

//     constexpr std::array tokens = tokenize(text);

//     REQUIRE(tokens == std::array{"one"sv, "two"sv, "three"sv);
// }