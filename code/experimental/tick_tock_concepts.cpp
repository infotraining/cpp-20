#include "catch.hpp"
#include <iostream>
#include <concepts>
#include <boost/hana.hpp>
#include <boost/type_index.hpp>
#include <fmt/core.h>

namespace Concepts
{
    namespace hana = boost::hana;
    using namespace boost::hana::literals;

    template <typename T>
    concept size_constant = std::convertible_to<T, std::size_t> 
    //     && requires(T t) {
    //     { T::value } -> std::convertible_to<std::size_t>;
    //     std::integral_constant<std::size_t, (std::size_t)T{}>{};
    // }
    ;

    template <typename T>
    concept tree_location = requires(T t, const T ct) {
        { t.is_root } -> std::convertible_to<bool>;
        t.indices,
        ct.of_child(0_c);
        requires !T::is_root || requires {
            { ct.head() } -> std::convertible_to<std::size_t>;
            ct.tail();
            ct.of_parent();
        };
    };

    template <std::size_t... indices_>
    struct TreeLocation
    {
        static constexpr const bool is_root = sizeof...(indices_) == 0;

        hana::tuple<hana::size_t<indices_>...> indices;

        constexpr TreeLocation() {}

        constexpr TreeLocation(hana::tuple<hana::size_t<indices_>...> indices_tpl) : indices{indices_tpl}
        {}

        constexpr auto of_parent() const
        {
            return Concepts::TreeLocation{hana::drop_back(hana::tuple<hana::size_t<indices_>...>{}, 1_c)};
        }

        constexpr std::size_t head() const
        {
            return hana::at(indices, 0_c);
        }

        constexpr auto tail() const
        {
            return Concepts::TreeLocation{hana::drop_front(hana::tuple<hana::size_t<indices_>...>{}, 1_c)};
        }

        constexpr auto of_child(size_constant auto index) const
        {
            return Concepts::TreeLocation<indices_..., index>{};
        }
    };

    std::ostream& operator<<(std::ostream& out, const tree_location auto& tl)
    {
        out << "< ";
        hana::for_each(tl.indices, [&out](const size_constant auto& index) { out << index << " ";});
        out << ">";
        return out;
    }

    TEST_CASE("tree location")
    {
        static_assert(TreeLocation<>::is_root);
        static_assert(!TreeLocation<0>::is_root);

        TreeLocation<0, 1, 2, 3>{};
        auto tl = TreeLocation<0, 1, 2>{}.of_parent();
        static_assert(std::is_same_v<decltype(tl), TreeLocation<0, 1>>);
        static_assert(TreeLocation<0, 1, 2>{}.head() == 0);
        static_assert(std::is_same_v<decltype(TreeLocation<0, 1, 2>{}.tail()), TreeLocation<1, 2>>);
        static_assert(std::is_same_v<decltype(TreeLocation<1, 2, 3>{}.of_child(4_c)), TreeLocation<1, 2, 3, 4>>);
    }

    struct Start {};
    struct Tick {};
    struct Tock {};

    template <template <typename...> typename Template, typename... Args>
    constexpr bool is_specialization_of_v = false;

    template <template <typename...> typename Template, typename... Args>
    constexpr bool is_specialization_of_v<Template, Template<Args...>> = true;

    template <typename T>
    concept node = std::is_object_v<std::remove_reference_t<T>>;

    template <typename T>
    concept tree = requires(T t) {
        t.root; requires node<decltype(t.root)>;
        { T::child_count } -> std::convertible_to<std::size_t>;
        requires T::child_count == 0 || requires {
            t.template child<0>();
        };
    };

    template<typename T>
    concept tree_ref = std::is_reference_v<T> && tree<std::remove_reference_t<T>>;

    template <node Root_, tree...Children_>
    struct Tree
    {
        Root_ root;
        std::tuple<Children_...> children;
        static constexpr size_constant auto child_count = sizeof...(Children_);

        Tree() = default;

        Tree(std::convertible_to<Root_> auto&& root_node, tree auto&&... children)
            : root{std::forward<decltype(root)>(root_node)},
              children{std::forward<decltype(children)>(children)...}
        {
            static_assert(tree<Tree>);
        }

        void print() const
        {
            std::cout << "{ " << root;
            if constexpr(sizeof...(Children_) > 0)
            {
                std::cout << children << " ";
            }
            std::cout << " }";
        }

        template <std::size_t index_>
            requires (index_ < sizeof...(Children_))
        tree auto& child()
        {
            return std::get<index_>(children);
        }

        tree auto& subtree(tree_location auto location)
        {
            if constexpr(location.is_root)
            {
                return *this;
            }
            else
            {
                return child<location.head()>().subtree(location.tail());
            }
        }

        auto operator<=>(const Tree&) const = default;
    };

    template <typename Root_, typename... Children_>
    Tree(Root_, Children_...) -> Tree<Root_, Children_...>;

    std::ostream& operator<<(std::ostream& out, const tree auto& t)
    {
        std::cout << "{ " << t.root;
        
        if (t.child_count > 0)
        {            
            std::cout << " " << t.children << " ";
        }
        
        std::cout << " }";

        return out;        
    }


    TEST_CASE("tree")
    {
        // tests for tree concept
        Tree<int> t1{1};
        REQUIRE(t1.subtree(TreeLocation<>{}) == t1);
        static_assert(node<decltype((t1.root))>);
        static_assert(tree<Tree<int>>);
        static_assert(tree<Tree<int, Tree<int>>>);

        auto t2 = Tree{1, Tree<int>{2}};
        auto sub_t2 = t2.subtree(TreeLocation<0>{});
        REQUIRE(sub_t2 == Tree<int>{2});
    }

    template <typename T>
    concept tree_context = requires (T t) {
        t.tree; requires tree<std::remove_reference_t<decltype(t.tree)>>;
        //{ t.location } -> tree_location;
    };

    template <typename T>
    concept message = std::is_object_v<T>;

    template <tree Tree_ , typename TreeLocation_ = TreeLocation<>>
    struct TreeContext
    {
        Tree_& tree;
        TreeLocation_ location;

        TreeContext(Tree_& tree, TreeLocation_ location = TreeLocation_{}): tree{ tree }
        {
            static_assert(tree_context<TreeContext>);
        }

        void send_down(message auto msg)
        {            
            const auto& msg_name = boost::typeindex::type_id<decltype(msg)>().pretty_name();

            Concepts::tree auto& current_tree = tree.subtree(location);
            
            //std::cout << "send_down(" << msg_name << ") at " << location << " - " << current_tree << "\n";
            fmt::print("send_down({} at {} - subtree: {}", msg_name, location, current_tree);

            if constexpr(requires { current_tree.root.handle(msg); })
            {
                std::cout << boost::typeindex::type_id<decltype(current_tree.root)>().pretty_name() << " handles " 
                    << msg_name << "\n";

                current_tree.root.handle(msg);
            }

            hana::size_t<current_tree.child_count> child_count;

            std::cout << "+ sending down " << msg_name << " to " << child_count << " children: ";
            child_count.times.with_index(
                [&](auto child_index) {
                    Concepts::node auto& child = tree.subtree(location.of_child(child_index)).root;
                    Concepts::tree_context auto child_context = Concepts::TreeContext{tree, location.of_child(child_index)};

                    std::cout << child_index << ": " << child << " : " << child_context.location << "\n";

                    if constexpr(requires { child.handle(msg, child_context); })
                    {
                        child.handle(msg, child_context);
                    }
                    else if constexpr(requires {child.handle(msg);})
                    {
                        child.handle(msg);
                        child_context.send_down(msg);
                    }
                    else
                    {
                        child_context.send_down(msg);
                    }
                }
            );
        }

        void send_up(message auto msg)
        {            
            if constexpr(!location.is_root) {
                node auto& parent = tree.subtree(location.of_parent()).root;
                tree_context auto parent_context = Concepts::TreeContext{tree, location.of_parent()};

                const auto& msg_name = boost::typeindex::type_id<decltype(msg)>().pretty_name();
                std::cout << "send_up(" << msg_name << ") at " << location << " - " << parent << "\n";

                if constexpr(requires { parent.handle(msg, parent_context); })
                {
                    parent.handle(msg, parent_context);
                }
                else if constexpr(requires { parent.handle(msg); })
                {
                    parent.handle(msg);
                    parent_context.send_up(msg);
                }
                else
                {
                    parent_context.send_up(msg);
                }
            }
        }
    };

    struct Idle
    {
        void handle(message auto &msg)
        {
            const auto& msg_name = boost::typeindex::type_id<decltype(msg)>().pretty_name();
            std::cout << " * Idle is handling a msg: " << msg_name << "\n";
        }

        // void handle(message auto &msg)
        //     requires (sizeof(msg) > 1)
        // {
        //     puts("!");
        // }
    };

    std::ostream& operator<<(std::ostream& out, const Idle&)
    {
        out << "Idle";
        return out;
    }


    struct TickTock
    {
        void handle(const Start& msg, tree_context auto context)
        {
            puts(" * Tick! TickTock handler sends down the Tick");
            context.send_down(Tick{});
        }

        void handle(const Tock& msg)
        {
            puts(" * Tock! TickTock handler");
        }
    };

    std::ostream& operator<<(std::ostream& out, const TickTock&)
    {
        out << "TickTock";
        return out;
    }

    struct Responder
    {
        int id;

        void handle(const Tick& msg, tree_context auto context)
        {
            std::cout << "* Responder#" << id << " responsds sending the Tock up\n";
            context.send_up(Tock{});
        }
    };

    std::ostream& operator<<(std::ostream& out, const Responder& r)
    {
        out << "Responder#" << r.id ;
        return out;
    }

    template <typename... Ts_>
    std::ostream& operator<<(std::ostream& out, const std::tuple<Ts_...>& tpl)
    {
        std::apply(
            [&out](const auto&... args) {                
                ((out << args << " "), ...);                
            }, tpl);

        return out;        
    }
}

TEST_CASE("tick-tock with concepts")
{
    using namespace Concepts;

    auto t =
        Tree{
            Idle{},
            Tree {
                TickTock{},
                Tree {
                    Responder{1},
                    Tree {
                        Idle{}
                    }
                }
            },
            Tree {
                TickTock{},
                Tree {
                    Responder{2}
                }
            }            
        };

    std::cout << t << "\n";
    std::cout << "-----------------------------------------------\n";

    TreeContext(t).send_down(Start{});
}