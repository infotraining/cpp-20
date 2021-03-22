#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include "catch.hpp"
#include <array>
#include <iostream>
#include <memory>
#include <ranges>
#include <vector>

namespace rngs = std::ranges;

template <typename TRange>
     requires
         rngs::input_range<TRange>
         && std::common_reference_with<rngs::range_reference_t<TRange>, std::uint64_t>
class ViewAddConstant : public rngs::view_base
{
    struct DataMembers
    {
        TRange range;
    };

    std::shared_ptr<DataMembers> data_members_;

public:
    using TRangeIterator = rngs::iterator_t<TRange>;

    template <typename Iter>
    class ConstantIterator
    {
        Iter pos_;
    public:
        using value_type = uint64_t;
        using reference = uint64_t;
        using difference_type = ptrdiff_t;
        using const_reference = uint64_t;

        ConstantIterator() = default;

        ConstantIterator(Iter pos) : pos_{pos}
        {}

        ConstantIterator& operator++()
        {
            ++pos_;
            return *this;
        }

        ConstantIterator operator++(int)
        {
            ConstantIterator tmp = *this;
            pos_++;
            return tmp;
        }

        reference operator*() const
        {
            return *pos_ + 42;
        }

        auto operator<=>(const ConstantIterator&) const = default;
    };

    using reference = uint64_t;
    using const_reference = uint64_t;
    using value_type = uint64_t;

    using iterator = ConstantIterator<TRangeIterator>;
    using const_iterator = ConstantIterator<TRangeIterator>;

    ViewAddConstant() = default;

    ViewAddConstant(TRange&& range)
        : data_members_ {std::make_shared<DataMembers>(std::forward<TRange>(range))}
    {
    }

    const_iterator begin() const
    {
        return iterator{std::begin(data_members_->range)};
    }

    iterator begin()
    {
        return iterator{std::begin(data_members_->range)};
    }

    const_iterator cbegin() const
    {
        return begin();
    }

    const_iterator end() const
    {
        return iterator {std::end(data_members_->range)};
    }

    iterator end()
    {
        return iterator {std::end(data_members_->range)};
    }

    const_iterator cend() const
    {
        return end();
    }
};

template <typename TRange>
ViewAddConstant(TRange &&)->ViewAddConstant<TRange>;

static_assert(rngs::input_range<ViewAddConstant<std::vector<uint64_t>>>);
static_assert(rngs::view<ViewAddConstant<std::vector<uint64_t>>>);

struct AddConstantFn
{
    template <typename TRange>
        requires
            rngs::input_range<TRange>
            && std::common_reference_with<rngs::range_reference_t<TRange>, uint64_t>
    auto operator()(TRange&& range) const
    {
        return ViewAddConstant {std::forward<TRange>(range)};
    }
};

template <typename TRange>
    requires rngs::input_range<TRange>
        && std::common_reference_with<rngs::range_reference_t<TRange>, uint64_t>
auto operator|(TRange&& range, const AddConstantFn&)
{
    return ViewAddConstant {std::forward<TRange>(range)};
}

namespace custom_views
{
    constexpr AddConstantFn add_constant;
}

TEST_CASE("ViewAddConstant")
{
    SECTION("l-value")
    {
        std::vector vec = {1, 2, 3, 4, 5};

        ViewAddConstant view_add_constant(vec);

        REQUIRE(rngs::equal(view_add_constant, std::vector {43, 44, 45, 46, 47}));
    }

    SECTION("const l-value")
    {
        const std::vector vec = {1, 2, 3, 4, 5};

        ViewAddConstant view_add_constant(vec);

        REQUIRE(rngs::equal(view_add_constant, std::vector {43, 44, 45, 46, 47}));
    }

    SECTION("r-value")
    {
        ViewAddConstant view_add_constant(std::vector<int> {1, 2, 3, 4, 5});

        REQUIRE(rngs::equal(view_add_constant, std::vector {43, 44, 45, 46, 47}));
    }
}

TEST_CASE("custom_view::add_constant")
{
    SECTION("l-value")
    {
        std::vector vec = {1, 2, 3, 4, 5};

        auto view_add_constant = vec | custom_views::add_constant;

        REQUIRE(rngs::equal(view_add_constant, std::vector {43, 44, 45, 46, 47}));
    }

    SECTION("const l-value")
    {
        const std::vector vec = {1, 2, 3, 4, 5};

        auto view_add_constant = vec | custom_views::add_constant;

        REQUIRE(rngs::equal(view_add_constant, std::vector {43, 44, 45, 46, 47}));
    }

    SECTION("r-value")
    {
        auto view_add_constant = std::vector {1, 2, 3, 4, 5} | custom_views::add_constant;

        REQUIRE(rngs::equal(view_add_constant, std::vector {43, 44, 45, 46, 47}));
    }
}

template <typename Iter>
class ConstantIterator
{
    Iter pos_;
public:
    using value_type = uint64_t;
    using reference = uint64_t;
    using difference_type = ptrdiff_t;
    using const_reference = uint64_t;

    ConstantIterator() = default;

    ConstantIterator(Iter pos) : pos_{pos}
    {}

    ConstantIterator& operator++()
    {
        ++pos_;
        return *this;
    }

    ConstantIterator operator++(int)
    {
        ConstantIterator tmp = *this;
        pos_++;
        return tmp;
    }

    reference operator*() const
    {
        return *pos_ + 42;
    }

    auto operator<=>(const ConstantIterator&) const = default;
};

TEST_CASE("iterator")
{
    std::array vec = {1, 2, 3, 4};

    ConstantIterator it{vec.begin()};

    REQUIRE(*it == 43);
    REQUIRE(*(++it) == 44);

    static_assert(std::input_iterator<decltype(it)>);
}