#include <range/v3/all.hpp>
#include <iostream>
#include <string>
#include <functional>
#include <tuple>
#include <iterator>
#include <vector>

int main()
{
    std::vector vec = { 1, 2, 1, 2, 1, 4, 5, 4, 7, 9 };

    std::cout << "\n------------------\n";

    vec |= ranges::action::sort(std::less{});

    auto count_items = [](const auto& range) { return ranges::accumulate(range, 0, [](int counter, const auto& item) { return ++counter; }); };

    auto freq = vec
        | ranges::view::group_by(std::equal_to{})
        | ranges::view::transform([=](const auto& group) {
            const auto first = std::begin(group);
            const auto last = std::end(group);
            const size_t count = count_items(group);
            return std::pair{*first, count};
        })
        ;

    for(const auto& [value, count] : freq)
    {
        std::cout << value << " - " << count << std::endl;
    }
}