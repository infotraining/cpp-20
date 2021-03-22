#include "catch.hpp"

#include <complex>
#include <concepts>
#include <iostream>
#include <set>
#include <unordered_set>
#include <vector>

TEST_CASE("templated lambda")
{
    auto plus = [](auto x, auto y)
    {
        return x + y;
    };

    auto result = plus.operator()<int, double>(1, 2.14);

    auto print_vec_size = []<typename T>(const std::vector<T>& vec)
    { std::cout << vec.size() << "\n"; };

    print_vec_size(std::vector{1, 2, 3});
}

template <typename T>
void foo(const T& item)
{
    puts(__PRETTY_FUNCTION__);
}

void foo(int)
{
    puts(__PRETTY_FUNCTION__);
}

TEST_CASE("foo overloading")
{
    int x = 10;
    const int y = 10;
    const int& ref_x = x;

    foo(x);
    foo(y);
    foo(ref_x);
}

// concepts
template <typename T>
concept Container = requires(T coll)
{
    typename T::value_type;
    { *coll.begin() } -> std::convertible_to<typename T::value_type>;
};

template <typename T>
requires Container<T>
struct Foo
{
};

template <Container C>
struct Bar
{
};

TEST_CASE("concepts")
{
    Foo<std::vector<int>> foo;
    Bar<std::vector<int>> bar;
}

template <typename Coll, typename T>
concept SupportsPushBack = requires(Coll c, T v)
{
    c.push_back(v);
};

template <typename Coll, typename T>
requires // defines constraint
    requires(Coll c, T v) { c.push_back(v); } // defines requirements
auto add(Coll& coll, const T& value)
{
    return coll.push_back(value);
}

template <typename Coll, typename T>
void add_to(Coll& coll, const T& value)
{
    if constexpr (requires { coll.push_back(value); })
    {
        coll.push_back(value);
    }
    else
    {
        coll.insert(value);
    }
}

class Dummy
{
};

TEST_CASE("push_backs")
{
    std::vector<int> vec;
    add(vec, 1);
    add_to(vec, 2);

    std::set<int> set_int;
    //add(set_int, 2);
    add_to(set_int, 1);

    std::vector words{"abc", "defff"};
    static_assert(std::is_same_v<decltype(words), std::vector<const char*>>);
}

TEST_CASE("weird stuff with aggregate initialization with braces")
{
    std::vector<std::complex<double>> vec{{1, 2}};
    std::array<std::complex<double>, 4> arr{{1, 2}};

    REQUIRE_FALSE(vec[0] == arr[0]);

    //std::array arr1(1, 2, 3);
}

template <auto Msg>
struct Message
{
    void print() const
    {
        std::cout << Msg << "\n";
    }
};

TEST_CASE("non-type template params")
{
    Message<'a'> m1;
    m1.print();

    Message<42>{}.print();

    //Message<3.14>{}.print();

    static const char* txt = "Hello";
    Message<&txt>{}.print();

    enum class Status
    {
        zero,
        one,
        two,
        none = zero
    };

    std::is_same_v<Message<Status::zero>, Message<Status::none>>;
}

template <std::size_t N>
struct Str
{
    char chars[N];
};

template <size_t N>
std::ostream& operator<<(std::ostream& out, const Str<N>& str)
{
    std::cout << str.chars;
    return out;
}

template <std::size_t N>
Str(const char (&)[N]) -> Str<N>;

TEST_CASE("string as template param")
{
    Message<Str{"Hello"}> msg;
    msg.print();

    Message<Str{"Hello"}> msg2;
    msg2.print();

    static_assert(std::is_same_v<decltype(msg), decltype(msg2)>);
}

template <typename T>
concept Hashable = requires(T obj)
{
    { obj.hash_value() } -> std::convertible_to<size_t>;
};

namespace std
{
    template <Hashable T>
    struct hash<T>
    {
        size_t operator()(const T& item) const
        {
            return item.hash_value();
        }
    };
}

struct Id
{
    int value;

    bool operator==(const Id&) const = default;

    size_t hash_value() const
    {
        return std::hash<int>{}(value);
    }
};

TEST_CASE("templates in practise")
{
    std::unordered_set<Id> ids;
    ids.insert(Id{42});
}

template <typename T>
class MyColl
{
public:
    void print() const { std::cout << "T\n"; }
};

template <std::ranges::input_range T>
class MyColl<T>
{
public:
    void print() const { std::cout << "range T\n"; }
};

template <std::ranges::random_access_range T>
class MyColl<T>
{
public:
    void print() const { std::cout << "random-access T\n"; }
};

TEST_CASE("partial specialization with concepts")
{
    //std::vector<int> vec;
    MyColl<std::vector<int>>{}.print();
    MyColl<int>{}.print();
    MyColl<std::set<int>>{}.print();
}

//void print(std::integral auto... items)
//{
//    std::cout << "integers: ";
//    (..., (std::cout << items << " "));
//    std::cout << "\n";
//}

template <typename... T>
requires(...&& requires(T) { std::cout << std::declval<T>(); }) void print(T... items)
{
    std::cout << "objects: ";
    (..., (std::cout << items << " "));
    std::cout << "\n";
}

TEST_CASE("print for integrals")
{
    print(1, 42u, 1000L, short{32});

    print(std::string{"abc"}, "def", 3.14);
}

template <typename T>
void insert_twice(std::vector<std::remove_cvref_t<T>>& coll, T&& value)
{
    coll.push_back(value);
    coll.push_back(std::forward<T>(value));
}

TEST_CASE("insert twice")
{
    std::string s = "a";
    std::vector<std::string> words;
    insert_twice(words, s);
    insert_twice(words, s);
    REQUIRE(words == std::vector<std::string>{"a", "a", "a", "a"});
}

////////////////////////////////////////////
// perfect call function template

template <typename Func, typename... Args>
decltype(auto) call(Func f, Args&&... args)
{
    decltype(auto) result = f(std::forward<Args>(args)...);

    if constexpr (std::is_rvalue_reference_v<decltype(result)>) // allows to have RVO
    {
        return std::move(result);
    }
    else
    {
        return result;
    }
}

template <typename T, size_t N>
struct Array
{
    T items[N];
};

template <typename T, typename... U>
requires(...&& std::same_as<T, U>)
    Array(T, U...)
->Array<T, sizeof...(U) + 1>;

TEST_CASE("CTAD with concepts")
{
    Array arr1 = {1, 2, 3, 4};
    static_assert(std::is_same_v<decltype(arr1), Array<int, 4>>);
}

// using NaN to instantiate template with double param
// TODO