#include "catch.hpp"

#include <vector>
#include <list>
#include <span>

namespace aux
{
    template <typename T> 
    struct is_std_vector : std::false_type
    {};

    template <typename T>
    struct is_std_vector<std::vector<T>> : std::true_type
    {};
}

TEST_CASE("lambda expressions - template syntax")
{
    SECTION("before C++20")
    {
        auto get_head = [](auto& vec, size_t size) {
            
            // ensuring that vec is really std::vector<T>
            static_assert(aux::is_std_vector<std::decay_t<decltype(vec)>>::value, "it must be a std::vector");

            // extracting type of vector's elements
            using T = typename std::decay_t<decltype(vec)>::value_type;

            using ReturnT = std::tuple<T*, size_t>;

            if (!vec.empty() && (size <= vec.size()))
                return std::optional{ReturnT{vec.data(), size}};
            else 
                return std::optional<ReturnT>{};
        };

        std::vector vec = {1, 2, 3};

        auto [head_ptr, head_size] = get_head(vec, 2).value();

        REQUIRE(*head_ptr == 1);
        ++head_ptr;
        REQUIRE(*head_ptr == 2);
    }

    SECTION("since C++20")
    {
        auto get_head = []<typename T>(std::vector<T>& vec, size_t size) {
           using ReturnT = std::span<T>;

            if (!vec.empty() && (size <= vec.size()))
                return std::optional{ReturnT{vec.data(), size}};
            else 
                return std::optional<ReturnT>{};
        };

        std::vector vec = {1, 2, 3};
        
        auto head_span = get_head(vec, 2).value();

        REQUIRE(head_span[0] == 1);
        REQUIRE(head_span[1] == 2);
    }
}