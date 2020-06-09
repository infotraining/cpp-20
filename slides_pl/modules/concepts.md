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