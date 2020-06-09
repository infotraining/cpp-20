---?image=slides_pl/img/Subtle-Prism.svg&size=cover


# C++20 - New Hope

Krystian Piękoś

---

## Informacje organizacyjne

- Czas trwania szkolenia: 
    - 9:00 - 16:00

- Materiały szkoleniowe:
    - https://infotraining.bitbucket.io/cpp-20
    - [repozytorium GIT](https://bitbucket.org/infotraining)

- Przerwy
- Lista obecności
- Ankieta

---

## Historia standaryzacji C++

* **1998** - pierwszy standard ISO C++ - C++98
* **2003** - C++03 - TC1 ("Technical Corrigendum 1")
* **2005** - opublikowany "Technical Report 1"
* **2011** - C++0x ratyfikowany jako C++11
* **2014** - C++14 - bugfixes & minor revision
* **2017** - C++17 - umiarkowana modyfikacja standardu 
* **2020** - C++20 - duża modyfikacja standardu

+++

## Stałe definiujące wersje

Stała preprocesora ``__cplusplus`` definiuje, która wersja standardu jest dostępna w trakcie kompilacji

| Wersja standardu | Wartość stałej |
| :--------------: |:--------------:|
| C++98 & C++03    | ``199711L``    |
| C++11            | ``201103L``    |
| C++14            | ``201402L``    |
| C++17            | ``201703L``    |
| C++20            | ``>201703L ``   |

+++

## C++20 w kompilatorze gcc

* Należy użyć opcji kompilacji:
  
  - ``-std=c++2a`` - od wersji 9.2
  - ``-std=c++20`` - od wersji 10.1

---?include=slides_pl/modules/concepts.md
