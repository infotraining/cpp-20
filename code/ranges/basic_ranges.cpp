#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include "helpers.hpp"
#include "catch.hpp"
#include <ranges>
#include <algorithm>
#include <iostream>
#include <string>
#include <functional>
#include <tuple>
#include <iterator>
#include <vector>
#include <string_view>
#include <random>

namespace rngs = std::ranges;

struct ToVectorFn
{    
};

template <rngs::range TRange>
auto operator|(TRange&& rng, ToVectorFn)
{
    using T = rngs::range_value_t<std::decay_t<TRange>>;
    return std::vector<T>{rng.begin(), rng.end()};
}

namespace range_conversion
{
    constexpr ToVectorFn to_vector;
}

template <rngs::range Range>
constexpr auto to_vector(Range&& r)
{
    using T = rngs::range_value_t<Range>;
    return std::vector<T>{r.begin(), r.end()};
}

TEST_CASE("sorting")
{
    std::cout << "\n-----------------------\n";

    std::vector<int> vec = helpers::create_numeric_dataset(20);

    SECTION("ascending")
    {
        rngs::sort(vec);

        helpers::print_range(vec, "vec");
    }

    SECTION("descending")
    {
        rngs::sort(vec, std::greater{});

        helpers::print_range(vec, "vec");
    }
}