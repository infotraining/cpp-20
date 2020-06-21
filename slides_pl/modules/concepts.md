---?image=slides_pl/img/Subtle-Prism.svg&size=cover

## Koncepty

+++

### Koncept

@ul
* jest nazwanym zbiorem wymagań
* id-expression ewaluowane do wartości logicznej `bool`
* musi być zdefiniowany na poziomie przestrzeni nazw
* nie może być rekursywny
* nie są dozwolone żadne specjalizacje (oryginalna definicja nie może być zmieniana)
@ulend

+++

### Koncept - 1

``` c++ code-noblend fragment
template <typename T>
concept Integral = std::is_integral_v<T>;
```

``` c++ code-noblend fragment
template <typename T>
concept SignedIntegral = Integral<T> && std::is_signed_v<T>;
```

``` c++ code-noblend fragment
template <typename T>
concept UnsignedIntegral = Integral<T> && !SignedIntegral<T>;
```

+++

### Koncept - 2

``` c++ code-noblend fragment
template<typename T>
concept Hashable = requires(T a) {
    { std::hash<T>{}(a) } -> std::convertible_to<std::size_t>;
};
```

+++ 

### Koncepty mogą być używane jako:

@ul
* nazwane wyrażenia - *id-expression*
* deklaracje typów parametrów szablonów
* ograniczenia placeholdera auto
* składnik definicji złożonych wymagań dla typów
@ulend

+++

### Szablony z ograniczeniami - 1

``` c++ code-noblend
template <std::integral T>
T square(T x)
{
    return x * x;
}
```

+++

### Szablony z ograniczeniami - 2

``` c++ code-noblend
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

print(std::vector{1, 2, 3});

print(std::map<int, std::string>{ {1, "one"}, {2, "two"} });
```
@[1-4](Definicja konceptu `PrintableRange`)
@[6](Użycie konceptu jako ograniczenia dla parametru szablonu)
@[13](@css[text-ok](OK!))
@[15](@css[text-error](ERROR! `std::map<int, std::string>` nie spełnia wymagań definiowanych przez koncept))

+++

### Koncepty + placeholder auto

Od C++20 możemy poprzedzić deklarację z placeholderem `auto` lub `decltype(auto)` konceptem ograniczającym:

``` c++ code-noblend
auto get_id()
{
    static int id_gen{};
    return ++id_gen;
}

std::unsigned_integral auto id = get_id();
```
@[7](Deklaracja `auto` z ograniczeniem typu)
@[7](@css[text-error](ERROR! deduced initializer does not satisfy placeholder constraints))

+++

### Koncepty + placeholder auto

Od C++20 możemy poprzedzić deklarację zmiennej z `auto` lub `decltype(auto)` konceptem
ograniczającym:

``` c++ code-noblend
auto get_id()
{
    static size_t id_gen{};
    return ++id_gen;
}

std::unsigned_integral auto id = get_id();
```
@[7](@css[text-ok](OK!))

+++

### Koncepty + placeholder auto

Od C++20 możemy poprzedzić deklarację zmiennej z `auto` lub `decltype(auto)` konceptem
ograniczającym:

``` c++ code-noblend
std::unsigned_integral auto get_id()
{
    static size_t id_gen{};
    return ++id_gen;
}

std::unsigned_integral auto id = get_id();
```
@[1-5](Ograniczenie może również być stosowane przy deklaracjach automatycznej detekcji typu zwracanego z funkcji!)

+++

### Placeholder auto z ograniczeniem

Jeśli przed placeholderem `auto` występuje koncept `C<A...>`, to dedukowany typ `T` musi spełniać ograniczenia zdefiniowane wyrażeniem `C<T, A...>`

``` c++ code-noblend
std::unsigned_integral auto get_id()
{
    static size_t id_gen{};
    return ++id_gen;
}

 std::convertible_to<uint64_t> auto id64 = get_id();
```
@[7](Dedukowany typ `size_t` jest podstawiony do konceptu `std::convertible_to<uint64_t, size_t>`)
@[7](@css[text-ok](OK!))

+++

### Ograniczenie szablonu (template constraint)

jest sekwencją logicznych operacji i operandów, które określają wymagania, jakie muszą spełniać argumenty szablonu.

+++

#### Ograniczenia

Są trzy rodzaje ograniczeń:
* koniunkcja *conjuction*
* dysjunkcja *disjunction*
* ograniczenie atomowe

+++

#### Koniunkcja ograniczeń

* Koniunkcja dwóch ograniczeń jest spełniona jeśli obydwa ograniczenia są spełnione.
* Ewaluacja przebiega od lewej do prawej (*short-circuited*)
  * jeśli lewy operand nie jest spełniony, nie jest wykonywane podstawianie argumentów szablonu dla prawego operanda

``` c++ code-noblend
template <typename T>
constexpr bool get_value() { return T::value; }

template <typename T>
    requires (sizeof(T) > 1) && (get_value<T>())
void f(T);   // #1

void f(int); // #2

f('a'); // OK! calls f(int)
```
@[5](Definicja ograniczenia - koniunkcja)
@[5, 10](Podczas sprawdzania ograniczeń dla #1 lewy operand koniunkcji 'sizeof(char) > 1' nie jest spełniony  `=>` `get_value<T>()` nie jest sprawdzane)
@[8, 10](#1 nie spełnia ograniczeń `=>` wywołana jest funkcja #2)

+++

#### Dysjunkcja ograniczeń

* tworzona przy pomocy operatora `||` w wyrażeniu ograniczającym
* jest spełniona jeśli spełniony jest jeden lub drugi operand ograniczenia (*short-circuited*)

+++

### Wyrażenie requires

Umożliwia zwięzłe zdefiniowanie wymagań dotyczących argumentów szablonu, które mogą być sprawdzone na etapie
kompilacji:

``` c++ code-noblend
template <typename T>
concept PointerLike = requires (T ptr) {
    typename T::element_type;
    { *ptr } -> std::same_as<typename T::element_type&>;
};
```

+++

### requires requires - WTF?

* Wyrażenie `requires` często jest używane po klauzuli `requires` w definicji ograniczeń szablonu
* Umożliwia to zdefiniowanie ograniczeń ad-hoc

``` c++ code-noblend
template <typename T>
    requires 
        requires (T x) { x + x; }
T add(T a, T b) { return a + b; }
```
@[2](Klauzula `requires` oznaczająca początek definicji ograniczeń szablonu)
@[3](Wyrażenie `requires` definiujące konkretne ograniczenie ad-hoc (bez konceptu))

+++

### Parametry wyrażenia requires

W wyrażeniu `requires` możemy wprowadzić listę lokalnych parametrów:

* parametry nie mogą mieć wartości domyślnych
* lista parametrów nie może kończyć się elipsą `...`
* parametry nie są instancjonowane ani linkowane - służą tylko definicji ograniczeń

``` c++ code-noblend
template <typename T>
concept Indexable = requires (T obj, size_t index) {
    typename T::value_type;
    { obj[index] } -> std::convertible_to<typename T::value_type&>;
};
```
@[2](Lista parametrów)

+++

### Ewaluacja wyrażenia requires

@ul
* *prvalue* typu `bool`
* Jeśli podstawienie argumentów szablonu do wyrażenia `requires` skutkuje:
  * nieprawidłowymi typami oraz wyrażeniami
  * lub naruszeniem semantycznych ograniczeń
* to wyrażenie `requires` jest ewaluowane do wartości `false` i nie powoduje, że program jest traktowany jako *ill-formed*
* Jeśli podstawienia typów i weryfikacja semantycznych wymagań przebiegnie pomyślnie wyrażenie jest ewaluowane do wartości `true`
* Sprawdzanie wymagań dokonywane jest w zadeklarowanej kolejności (leksykalnie)
@ulend

+++

### Proste wymagania

sprawdzają poprawność podanego wyrażenia:

``` c++ code-noblend
template <typename T>
concept Addable = requires(T a, T b) {
    a + b;
};
```
@[3](`Addable<T>` jest `true`, jeśli wyrażenie `a + b` jest poprawnym wyrażeniem)

+++

### Wymagania typu

sprawdzają poprawność typu:

``` c++ code-noblend
template<typename T, typename T::type = 0> struct S;
template<typename T> using Ref = T&;

template<typename T> concept C = requires {
    typename T::inner; // required nested member name
    typename S<T>; // required class template specialization
    typename Ref<T>; // required alias template substitution, fails if T is void
};
```
@[5](Wymaganie posiadania przez typ `T` zagnieżdżonego typu `inner`)
@[6](Wymaganie istnienia specjalizacji szablonu `S<T>` - nie musi być to typ kompletny)
@[7](Wymaganie możliwości podstawienia typu `T` do aliasu szablonu `Ref`)

+++

### Wymagania złożone

Wymagania złożone umożliwiają sprawdzenie określonych właściwości wyrażenia poddanego ewaluacji (np. zwracanego typu):

``` c++ code-noblend
requires {
    { Expr1 };
    { Expr2 } -> ConceptA;
    { Expr3 } noexcept -> ConceptB<B1, ..., Bn>;
    //...
};
```
@[3-4](Wymagania z określeniem zwracanego typu wykorzystują koncepty i są rozwijane do)

+++

### Wymagania złożone

Umożliwiają sprawdzenie określonych właściwości wyrażenia poddanego ewaluacji (np. zwracanego typu):

``` c++ code-noblend
requires {
    { Expr1 };
    { Expr2 } -> ConceptA<decltype((Expr2))>;
    { Expr3 } noexcept -> ConceptB<decltype((Expr3)),B1, ..., Bn>;
    //...
};
```

+++

#### Wymagania złożone - przykłady (1)

``` c++ code-noblend
template <typename T>
concept PostFixIncrementable = requires (T obj) {
    { obj++; }
};
```
@[3](Sprawdzenie, czy `obj++` jest poprawnym wyrażeniem)
@[3](Równoważne prostemu wymaganiu)

+++

#### Wymagania złożone - przykłady (1)

``` c++ code-noblend
template <typename T>
concept PostFixIncrementable = requires (T obj) {
    obj++;
};
```

+++

#### Wymagania złożone - przykłady (2)

``` c++ code-noblend
template <typename T>
concept Indexable = requires (T obj, size_t n) {
    { obj[n] } -> std::same_as<typename T::reference>;
    { obj.at(n) } -> std::same_as<typename T::reference>;
    { obj.size() } noexcept -> std::convertible_to<size_t>;
    { obj.~T() } noexcept;
};
```

+++

### Wymagania zagnieżdżone

Umożliwiają zdefiniowanie dodatkowego ograniczenia z wykorzystaniem lokalnych parametrów:

``` c++ code-noblend
requires {
    expression; // optional
    requires predicate;
}
```

``` c++ code-noblend
template <typename T>
concept LeanPointer = requires(T ptr) {
    *ptr; // simple requirement
    requires sizeof(ptr) == sizeof(intptr_t); // nested requirement
};
```

+++

#### Wymagania zagnieżdżone - przykład

``` c++ code-noblend
template <typename T>
concept AdditiveRange = requires (T&& c) {
    std::ranges::begin(c);
    std::ranges::end(c); 
    typename std::ranges::range_value_t<T>; // type requirement
    requires requires(std::ranges::range_value_t<T> x) { x + x; }; // nested requirement
};

template <AdditiveRange Rng>
auto sum(const Rng& data)
{
    return std::accumulate(std::begin(data), std::end(data), 
        std::ranges::range_value_t<Rng>{});
}

assert(sum(std::vector{1, 2, 3}) == 6);

assert(sum({ "one", "two", "three" }) == "onetwothree"s);
```
@[2](Wyrażenie `requires` z parametrem lokalnym `T&& c`)
@[3](Proste wymaganie)
@[4](Proste wymaganie)
@[5](Wymaganie typu)
@[6](Wymaganie zagnieżdżone `{x + x}` - wprowadza nowy parametr lokalny `x`)
@[9-14](Szablon korzystający z konceptu `AdditiveRange`)
@[16](@css[text-ok](OK!))
@[18](@css[text-error](ERROR! `{ x + x; }; is not satisfied`))