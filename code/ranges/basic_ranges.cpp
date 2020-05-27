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
#include "catch.hpp"

namespace rngs = std::ranges;

void print_range(rngs::range auto&& rng , std::string_view prefix)
{
    std::cout << prefix << ": [ ";
    for(auto&& item : rng)
    {
        std::cout << item << " ";
    }
    std::cout << "]\n";
}

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

auto create_data(size_t size)
{
    std::vector<int> data(size);

    std::mt19937 rnd_gen{42};
    std::uniform_int_distribution<> distr(-100, 100);
    
    rngs::generate(data, [&]{ return distr(rnd_gen);});

    return data;
}

TEST_CASE("sorting")
{
    std::cout << "\n-----------------------\n";

    std::vector<int> vec = create_data(20);

    SECTION("ascending")
    {
        rngs::sort(vec);

        print_range(vec, "vec");    
    }

    SECTION("descending")
    {
        rngs::sort(vec, std::greater{});

        print_range(vec, "vec");
    }
}

TEST_CASE("filter")
{
    std::cout << "\n-----------------------\n";

    std::vector<int> vec = create_data(20);
    print_range(vec, "vec");

    SECTION("using predicate")
    {
        auto gt_than_zero = vec | rngs::views::filter([](int item) { return item > 0; });

        print_range(gt_than_zero, "gt_than_zero");
    }    
}

TEST_CASE("top 3")
{
    std::cout << "\n-----------------------\n";

    std::vector<int> vec = create_data(20);   
    
    rngs::sort(vec);        
    auto top_3 = vec | rngs::views::reverse | rngs::views::take(3);

    print_range(vec, "sorted vec: ");
    print_range(top_3, "3 largest: ");
}