#include "catch.hpp"
#include <iostream>
#include <string>
#include <type_traits>

using namespace std;

namespace cpts
{
	template<class From, class To>
	concept ConvertibleTo =
	std::is_convertible_v<From, To> &&
		requires(std::add_rvalue_reference_t<From>(& f)()) {
			static_cast<To>(f());
		};
}

// clang-format off
template<typename T>
concept EqualityComparable =
requires(T a, T b)
{
	{ a == b } -> cpts::ConvertibleTo<bool>;
	{ a != b } -> cpts::ConvertibleTo<bool>;
};
// clang-format on

template<EqualityComparable T>
void are_equal(T const& a, T const& b)
{
	if (a == b)
		cout << a << " equals to " << b << "\n";
	else
		cout << a << " does not equal to " << b << "\n";
}

//-----------------------------------------------------------------

template<typename T>
concept Integral = std::is_integral_v<T>;

template<typename T>
concept FloatingPoint = std::is_floating_point_v<T>;

template<Integral T>
T calculate(T x, T factor)
{
	cout << "Integral calculate(Integral " << x << ", Integral " << factor << ")\n";
	return x * factor;
}

template<FloatingPoint T>
T calculate(T x, T factor)
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

/////////////////////////////////////////////////////////////////////////
//

template<typename T>
concept Callable =
requires(T a)
{
	{ a() };
};

void call(Callable auto f)
{
	f();
}

void foo()
{
	std::cout << "foo()\n";
}

struct Foo
{
	void operator()() const
	{
		std::cout << "Foo::operator()\n";
	}
};

TEST_CASE("callable concept")
{
	call(foo);

	Foo f;
	call(f);

	call([]
	{ std::cout << "Lambda::operator()\n"; });

	//call("text");
}