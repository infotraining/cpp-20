#include "catch.hpp"
#include <iostream>
#include <string>
#include <type_traits>

using namespace std;

// clang-format off
template <typename T>
concept bool EqualityComparable()
{
    return requires(T a, T b)
    {
        { a == b } -> bool;
        { a != b } -> bool;
    };
}
// clang-format on

void are_equal(EqualityComparable const& a, EqualityComparable const& b)
{
    if (a == b)
        cout << a << " equals to " << b << "\n";
    else
        cout << a << " does not equal to " << b << "\n";
}

//-----------------------------------------------------------------

template <typename T>
concept Integral = std::is_integral_v<T>;

template <typename T>
concept FloatingPoint = std::is_floating_point_v<T>;

Integral calculate(Integral x, Integral factor)
{
    cout << "Integral calculate(Integral " << x << ", Integral " << factor << ")\n";
    return x * factor;
}

FloatingPoint calculate(FloatingPoint x, FloatingPoint factor)
{
    cout << "FloatingPoint calculate(FloatingPoint " << x << ", FloatingPoint " << factor << ")\n";
    return x * factor;
}

struct Data
{
    int value;
};

TEST_CASE("basic concepts")
{
    are_equal(1, 2);

    SECTION("EqualityComparable concept")
    {
        are_equal("text"s, "test"s);

        //are_equal(Data{1}, Data{2});
    }
    
    SECTION("Integral & FloatingPoint concepts - SFINAE")
    {
        REQUIRE(calculate(2, 4) == 8);  
        REQUIRE(calculate(2.0, 4.0) == Approx(8.0));

        //calculate(Data{2}, Data{4});
    }
}