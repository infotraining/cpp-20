#include "catch.hpp"
#include <type_traits>
#include <functional>
#include <iostream>
#include <iterator>
#include <map>
#include <ranges>

using namespace std::literals;

template <typename T>
concept Hashable = requires(T a) {
    { std::hash<T>{}(a) } -> std::convertible_to<std::size_t>;
};

template <Hashable T>
size_t calculate_hash(const T& obj)
{
    return std::hash<T>{}(obj);
}

struct Dummy {};

TEST_CASE("hash concept")
{
    REQUIRE(calculate_hash(2) == std::hash<int>{}(2));
    //calculate_hash(Dummy{});
}



////////////////////////////////
// requires

template <typename T>
concept Indexable = requires(T obj, size_t index) {
    typename T::value_type;
    { obj[index] } -> std::convertible_to<typename T::value_type&>;
};

struct Array
{
    using value_type = int;

    int values[4] = {1, 2, 3, 4};

    const int& operator[](int index) const
    {
        return values[index];
    }

    int& operator[](int index)
    {
        return values[index];
    }
};

TEST_CASE("Indexable concept")
{
    static_assert(Indexable<Array>);
}

template<typename T> concept IllFormed =
requires {
    new int[-(int)sizeof(T)]; // ill-formed, no diagnostic required - GCC compiles!!!
};

TEST_CASE("ill-formed")
{
    REQUIRE_FALSE(IllFormed<int>);
}

/////////////////////////////////////////
// type requirements

template<typename T, typename T::type = 0> struct S;
template<typename T> using Ref = T&;

template<typename T> 
concept C = requires {
    typename T::inner; // required nested member name
    typename S<T>; // required class template specialization
    typename Ref<T>; // required alias template substitution, fails if T is void
};

struct TypeRequirements
{
    using inner = int;
    using type = int;
};

TEST_CASE("type requirements")
{
    static_assert(C<TypeRequirements>);
}

/////////////////////////////////////////
// compound requirements

namespace CompoundRequirements
{
    template <typename T>
    concept Indexable = requires (T obj, size_t n) {
        { obj[n] } -> std::same_as<typename T::reference>;
        { obj.at(n) } -> std::same_as<typename T::reference>;
        { obj.size() } noexcept -> std::convertible_to<size_t>;
        { obj.~T() } noexcept;
    };
}

TEST_CASE("Compound requirements")
{
    static_assert(CompoundRequirements::Indexable<std::vector<int>>);
    static_assert(CompoundRequirements::Indexable<std::vector<bool>>);
    static_assert(CompoundRequirements::Indexable<std::array<int, 10>>);
    //static_assert(CompoundRequirements::Indexable<std::map<int, std::string>>);
}

/////////////////////////////////////////
// nested requirements

template <typename T>
concept SameSizeAsPointer = requires(T obj) {    
    requires sizeof(obj) == sizeof(intptr_t);
};

template <SameSizeAsPointer T>
void use(T obj)
{    
}

TEST_CASE("nested requirements")
{
    use(4l);
    use(3.14);
}

template <typename T>
concept HeapAllocable = requires (T obj, size_t n) {
    requires std::same_as<T*, decltype(new T)>; // nested
    std::same_as<T*, decltype(new T[n])>; // nested
    //{ new T[n] } -> std::same_as<T*>;
    { delete new T }; // compound
    { delete new T[n] }; // compound    
};

struct X
{
private:
//    ~X() = default;
};

TEST_CASE("heap allocable")
{
    static_assert(HeapAllocable<int>);
    static_assert(HeapAllocable<X>);
}

/////////////////////
//
template <typename T>
concept AccumulativeRange = requires (T&& c) {
    std::ranges::begin(c); // simple requirement
    std::ranges::end(c); // simple requirement
    typename std::ranges::range_value_t<T>; // type requirement
    requires requires(std::ranges::range_value_t<T> x) { x + x; };
};

template <AccumulativeRange Rng>
auto sum(const Rng& data)
{
    return std::accumulate(std::begin(data), std::end(data), std::ranges::range_value_t<Rng>{});
}

TEST_CASE("sum")
{
    using namespace std::literals;

    static_assert(AccumulativeRange<std::vector<int>>);

    std::vector vec{1, 2, 3};
    REQUIRE(sum(vec) == 6);

    int data[] = {1, 2, 3};
    static_assert(AccumulativeRange<decltype(data)>);
    REQUIRE(sum(data) == 6);

    auto words = { "one"s, "two"s, "three"s };
    REQUIRE(sum(words) == "onetwothree"s);

    // auto tokens = { "one", "two", "three" };
    // REQUIRE(sum(tokens) == "onetwothree"s);
}

template <typename T>
concept LeanPointer = requires(T ptr) {
    *ptr;
    requires sizeof(ptr) == sizeof(intptr_t);
};

TEST_CASE("lean pointer")
{    
    static_assert(LeanPointer<int*>);
    static_assert(LeanPointer<std::unique_ptr<int>>);     
    //static_assert(LeanPointer<std::shared_ptr<int>>);
}

//////////////////////////////////////////
// PrintableRange

template <typename T>
concept PrintableRange = std::ranges::range<T> && requires {
    std::cout << std::declval<std::ranges::range_value_t<T>>();
};

void print(PrintableRange auto const& rng)
{
    for(const auto& item : rng)
        std::cout << item << " ";
    std::cout << "\n";
}

TEST_CASE("printing range")
{
    print(std::vector{1, 2, 3});

    //print(std::map<int, std::string>{ {1, "one"}, {2, "two"} });
}

std::unsigned_integral auto get_id()
{
    static size_t id_gen{};

    return ++id_gen;
}

TEST_CASE("auto + concept")
{
    std::unsigned_integral auto id = get_id();

    std::convertible_to<uint64_t> auto id64 = get_id();
}

/////////////////////////////
// Constraints

template <typename T>
concept Incrementable = std::incrementable<T>;

template <typename T>
concept Decrementable = requires (T obj) {
    --obj;
    obj--;
};

template <Incrementable T>
    requires Decrementable<T>
void foo(T)
{}

template <Incrementable T>
    requires Decrementable<T>
void foo(T); // redeclaration

// template <typename T>
//     requires Incrementable<T> && Decrementable<T>
// void foo(T)
// {}

template <Incrementable T>
    requires Decrementable<T>
void bar(T) {}

template <Decrementable T>
    requires Incrementable<T>
void bar(T) {}


TEST_CASE("constraints")
{
    foo(std::vector{1, 2, 3}.begin());
    //bar(std::vector{1, 2, 3}.begin());
}

