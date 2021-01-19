module;

#include <iostream>
#include <string_view>

export module Hello;

export void say_hello(std::string_view name)
{
    std::cout << "Hello " << name << "!\n";
}