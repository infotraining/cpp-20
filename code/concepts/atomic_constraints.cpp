#include "catch.hpp"
#include <iostream>

using namespace std::literals;

//---------------------------
// atomic constraints

template <typename T> constexpr bool is_meowable = true;
template <typename T> constexpr bool is_cat = true;

template <typename T>
concept Meowable = is_meowable<T>;

template <typename T>
concept BadMeowableCat = is_meowable<T> && is_cat<T>;

template <typename T>
concept GoodMeowableCat = Meowable<T> && is_cat<T>;

template <Meowable T>
void f1(T);

template<BadMeowableCat T>
void f1(T);

template <Meowable T>
void f2(T)
{
    std::cout << "f2() requires Meowable T\n";
}

template <GoodMeowableCat T>
void f2(T)
{
    std::cout << "f2 requires GoodMeowableCat\n";
}

TEST_CASE("atomic constraints - 1")
{
    //f1(0); // error, ambiguous:
             // the is_meowable<T> in Meowable and BadMeowableCat forms distinct
             // atomic constraints that are not identical (and so do not subsume each other)

    f2(0); // OK, calls #4, more constrained than #3
           // GoodMeowableCat got its is_meowable<T> from Meowable
}


template <size_t N> constexpr bool Atomic = true;

template <size_t N> concept Constraint = Atomic<N>;

template <size_t N> concept Add1 = Constraint<N + 1>;
template <size_t N> concept AddOne = Constraint<N + 1>;

template <size_t M> void f()
    requires Add1<2 * M>
{
    std::cout << "f<M>() with constraint Add1<2 * M + 1>\n";
}

template <size_t N> int f()
    requires AddOne<2 * N> && true
{
    std::cout << "f<M>() with constraint AddOne<2 * M + 1>\n";
    return N;
}

TEST_CASE("atomic constraint - 2")
{
    auto x = f<0>();
    REQUIRE(x == 0);
}

////////////////////////////////////
//

template <unsigned N> struct WrapN;

template <unsigned N> using Add1T = WrapN<N+1>;
template <unsigned N> using AddOneT = WrapN<N+1>;

template <unsigned M> void g(Add1T<2 * M>*);
template <unsigned M> void g(AddOneT<2 * M>*)
{}

TEST_CASE("atomic constraints - 3")
{
    g<0>(nullptr);
}

///////////////////////////////////////////
// Partial ordering

template <typename T>
concept Decrementable = requires(T t) { --t; };

template <typename T>
concept ReverseIterator = Decrementable<T> && requires(T t) { *t; };

// ReverseIterator subsumes Decrementable, but not the other way around

template <Decrementable T>
std::string fun(T)
{
    return "f<Decrementable T>(T)";
}

template <ReverseIterator T>
std::string fun(T)
{
    return "f<ReverseIterator>(T)";
}

TEST_CASE("partial ordering in function calls - 1")
{
    REQUIRE(fun(0) == "f<Decrementable T>(T)"s);
    REQUIRE(fun(reinterpret_cast<int*>(0)) == "f<ReverseIterator>(T)"s);
}

template <typename T>
std::string gc(T)
{
    return "gc<T>(T)";
}

template <Decrementable T>
std::string gc(T)
{
    return "gc<Decrementable>(T)";
}

TEST_CASE("partial ordering in function calls - 2")
{
    REQUIRE(gc(true) == "gc<T>(T)"s); // bool does not satisfy Decrementable, selects #3
    REQUIRE(gc(0) == "gc<Decrementable>(T)"s);  // int satisfies Decrementable, selects #4 because it is more constrained
}