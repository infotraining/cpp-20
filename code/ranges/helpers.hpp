#ifndef RANGES_HELPERS_HPP
#define RANGES_HELPERS_HPP

#include <algorithm>
#include <iostream>
#include <random>
#include <ranges>
#include <string_view>
#include <vector>
#include <fstream>

namespace helpers
{
    inline void print_range(std::ranges::range auto&& rng , std::string_view prefix)
    {
        std::cout << prefix << ": [ ";
        for(auto&& item : rng)
        {
            std::cout << item << " ";
        }
        std::cout << "]\n";
    }

    inline auto create_numeric_dataset(size_t size, int low = -100, int high = 100)
    {
        std::vector<int> data(size);

        std::mt19937 rnd_gen{42};
        std::uniform_int_distribution<> distr(low, high);

        std::ranges::generate(data, [&]{ return distr(rnd_gen);});

        return data;
    }

    inline std::vector<std::string> load_dictionary()
    {
        std::ifstream fin{"en.dict"};

        std::vector<std::string> dictionary;
        std::string word;

        while(fin >> word)
        {
            dictionary.push_back(std::move(word));
        }

        return dictionary;
    }
}

#endif //RANGES_HELPERS_HPP
