#include "catch.hpp"
#include <experimental/generator>
#include <iostream>
#include <string>

using namespace std::experimental;

generator<char> hello()
{
    co_yield 'H';
    co_yield 'e';
    co_yield 'l';
    co_yield 'l';
    co_yield 'o';
}

TEST_CASE("simple generator")
{
    for (const auto& token : hello())
    {
        std::cout << token << '\n';
    }
}