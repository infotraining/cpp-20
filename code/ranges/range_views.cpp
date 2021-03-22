#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include "catch.hpp"
#include "helpers.hpp"
#include <ranges>
#include <sstream>

#define MSVC_COMPILER (defined(_MSC_VER))
#define GCC_COMPILER (defined(__GNUC__) && !defined(__clang__))

namespace rng = std::ranges;

TEST_CASE("views")
{
    SECTION("all - all elements range")
    {
        std::vector<int> vec = helpers::create_numeric_dataset(20);

        for (int n : std::views::all(vec) | std::views::take(5))
            std::cout << n << " ";
        std::cout << "\n";
    }

#if GCC_COMPILER
    SECTION("iota - integer sequence range")
    {
        SECTION("bound version")
        {
            for (const int i : rng::iota_view(1, 10)) // all integers in range [1;10)
                std::cout << i << "; ";
            std::cout << "\n";

            auto numbers = std::views::iota(1, 10);

            rng::equal(numbers, std::vector{1, 2, 3, 4, 5, 6, 7, 8, 9});
        }

        SECTION("compare to for loop")
        {
            const size_t n = 1'000;

            BENCHMARK("for loop with i")
            {
                size_t result{};
                for (size_t i = 1; i < n; ++i)
                    result += i;
                return result;
            };

            BENCHMARK("iota - bounded")
            {
                size_t result{};
                for (const size_t i : std::views::iota(1u, n))
                    result += i;
                return result;
            };

            BENCHMARK("iota - unbounded")
            {
                size_t result{};
                for (const size_t i : std::views::iota(1u) | std::views::take(n))
                    result += i;
                return result;
            };
        };
    }
    #endif

    SECTION("istream_view<T> - view for an input stream")
    {
        using namespace std::literals;

        std::string text = "1_one 2_two   3_three  \n4_four      5_five";
  
        SECTION("range-based for on a stream")
        {
            std::istringstream text_stream{text};
            auto text_view = rng::istream_view<std::string>(text_stream);
            for (const auto& word : text_view)
                std::cout << word << " ";
            std::cout << "\n";
        }

#if MSVC_COMPILER 
        SECTION("filtering a stream")
        {
            std::istringstream text_stream{text};
            auto evens = rng::istream_view<std::string>(text_stream)
                | std::views::transform([](const std::string& token) { return std::stoi(token); })
                | std::views::filter([](int n){ return n % 2 == 0; });

            REQUIRE(rng::equal(evens, std::vector{2, 4}));
        }
#endif
    }
}

TEST_CASE("filter - basics")
{
    std::cout << "\n-----------------------\n";

    std::vector<int> vec = helpers::create_numeric_dataset(20);

    helpers::print_range(vec, "vec");

    SECTION("views::all - all elements")
    {
        for (int n : std::views::all(vec) | std::views::take(5))
            std::cout << n << " ";
        std::cout << "\n";
    }

    SECTION("iota")

    SECTION("using predicate")
    {
        auto gt_than_zero = vec | rng::views::filter([](int item) { return item > 0; });

        helpers::print_range(gt_than_zero, "gt_than_zero");
    }

    SECTION("top 3 that are even")
    {
        std::cout << "\n-----------------------\n";

        rng::sort(vec);
        auto top_3_evens = vec | rng::views::filter([](int n) { return n % 2 == 0; })
            | rng::views::reverse | rng::views::take(3);

        helpers::print_range(vec, "sorted vec: ");
        helpers::print_range(top_3_evens, "3 largest: ");
    }
}

// TEST_CASE("filter the dictionary")
// {
//     std::vector<std::string> dictionary = helpers::load_dictionary();
//     REQUIRE(dictionary.size() == 119789U);
// }
