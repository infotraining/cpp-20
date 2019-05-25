#include "catch.hpp"
#include <experimental/generator>
#include <numeric>

using namespace std::experimental;

generator<int> gen()
{
    for (int i = 0;; i++)
    {
        co_yield i;
    }
}

generator<int> multiply(generator<int>& seq, int factor)
{
    for (auto value : seq)
        co_yield value* factor;
}

generator<int> add(generator<int>& seq, int offset)
{
    for (auto value : seq)
    {
        co_yield value + offset;
    }
}

generator<int> take_until(generator<int>& g, int sentinel)
{
    for (auto value : g)
    {
        if (value == sentinel)
            break;
        co_yield value;
    }
}

TEST_CASE("chain of generators")
{
    auto g = gen();
    auto t = take_until(g, 10);
    auto m = multiply(t, 10);
    auto a = add(m, 15);

	auto result = std::accumulate(a.begin(), a.end(), 0);

	REQUIRE(result == 600);
}