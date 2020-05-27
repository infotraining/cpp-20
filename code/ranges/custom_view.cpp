#include "catch.hpp"
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
    struct Iterator : rngs::iterator_t<TRange>
    {
        using Base = rngs::iterator_t<TRange>;
        using Reference = uint64_t;

        Iterator() = default;

        Iterator(const Base& b)
            : Base {b}
        {
        }

        Iterator operator++(int)
        {
            auto iter = *this;
            static_cast<Base&>(*this)++;
            return iter;
        }

        Iterator& operator++()
        {
            ++static_cast<Base&>(*this);
            return *this;
        }

        Reference operator*() const
        {
            return *static_cast<Base>(*this) + 42;
        }
    };

    using reference = uint64_t;
    using const_reference = uint64_t;
    using value_type = uint64_t;

    using iterator = Iterator;
    using const_iterator = Iterator;

    ViewAddConstant() = default;

    ViewAddConstant(TRange&& range)
        : data_members_ {std::make_shared<DataMembers>(std::forward<TRange>(range))}
    {
    }

    iterator begin() const
    {
        return std::begin(data_members_->range);
    }

    const_iterator cbegin() const
    {
        return begin();
    }

    iterator end() const
    {
        return std::end(data_members_->range);
    }

    const_iterator cend() const
    {
        return end();
    }        
};

static_assert(rngs::input_range<ViewAddConstant<std::vector<uint64_t>>>);
static_assert(rngs::view<ViewAddConstant<std::vector<uint64_t>>>);

template <typename TRange>
ViewAddConstant(TRange&&) -> ViewAddConstant<TRange>;

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
    requires 
        rngs::input_range<TRange>
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
        ViewAddConstant view_add_constant(std::vector<int>{1, 2, 3, 4, 5});        

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
        auto view_add_constant = std::vector{1, 2, 3, 4, 5} | custom_views::add_constant;

        REQUIRE(rngs::equal(view_add_constant, std::vector {43, 44, 45, 46, 47}));
    }
}