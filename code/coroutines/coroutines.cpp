#include "catch.hpp"
#include <experimental/generator>
#include <iostream>
#include <string>

using namespace std::experimental;

///////////////////////////////
// the simplest coroutine
namespace explain
{
    struct suspend_never
    {
        bool await_ready() noexcept
        {
            return true;
        }

        void await_suspend(coroutine_handle<>) noexcept { }

        void await_resume() noexcept { }
    };
}

auto the_simplest_coroutine()
{
    co_await explain::suspend_never{};
}

TEST_CASE("the simplest coroutine")
{
    the_simplest_coroutine();
}


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