#include "catch.hpp"
#include <iostream>
#include <concepts>

namespace Classic
{
    struct MessageBase
    {        
        virtual ~MessageBase() = default;
    };

    template <typename TMessage>
    struct Handler
    {
        virtual void handle(const TMessage& msg) = 0;
        virtual ~Handler() = default;
    };

    class ComponentBase
    {        
        std::string id_;
        ComponentBase* parent_;
        std::vector<std::unique_ptr<ComponentBase>> children_;

    public:
        template <typename... Ts>
        ComponentBase(std::string id, Ts... child)
            : id_{std::move(id)}
        {            
            auto add_child_node = [this](auto node) {
                node->set_parent(this);
                children_.push_back(std::move(node));
            }; 

            (add_child_node(std::move(child)), ...);
        }

        ComponentBase() = default;

        virtual ~ComponentBase() = default;

        virtual void process_event(const MessageBase& msg) = 0;
    
        void send_down(const MessageBase& msg)
        {            
            for(const auto& child : children_)
            {                
                child->process_event(msg);                
            }
        }
        
        void send_up(const MessageBase& msg) 
        {            
            if (!parent_) 
                return;
                            
            parent_->process_event(msg);
        }

        const std::string& id() const
        {
            return id_;
        }

        ComponentBase* parent() const
        {
            return parent_;
        }

        void set_parent(ComponentBase* p)
        {
            parent_ = p;
        }
    };


    template <typename... Ms>
    struct Component : ComponentBase, Handler<Ms>...
    {                        
        using ComponentBase::ComponentBase;

        using Handler<Ms>::handle...;

        template <typename M>
        bool try_handle(const MessageBase& msg)
        {
            if (const M* m = dynamic_cast<const M*>(&msg))
            {
                handle(*m);
                return true;
            }
            
            return false;
        }

        void process_event(const MessageBase& msg) override
        {
            (try_handle<Ms>(msg) || ...);
        }
    };

    struct Start : MessageBase {};

    struct Tick : MessageBase {};

    struct Tock : MessageBase {};

    struct TickTock : Component<Start, Tock>
    {                
        using Component<Start, Tock>::Component;

        void handle(const Start& msg) override
        {            
            std::cout << id() << " start\n";            
            std::cout << id() << " tick\n";            
            send_down(Tick{});
        }

        void handle(const Tock& msg) override
        {         
            std::cout << id() << " tock\n";              
        }
    };

    struct Idle : Component<Tick, Tock>
    {
        using Component<Tick, Tock>::Component;

        void handle(const Tick& msg) override
        {            
            puts(".");            
            send_down(msg);
        }

        void handle(const Tock& msg) override
        {         
            puts("#");
            send_up(msg);
        }
    };

    struct Responder : Component<Tick> 
    {
        using Component<Tick>::Component;

        void handle(const Tick& msg) override
        {   
            std::cout << id() << " responds to tick...\n";                     
            send_up(Tock{});
        }
    };
}

TEST_CASE("tick-tock")
{
    using namespace Classic;

    auto tick_tock = TickTock(
        "tick_tock", 
            std::make_unique<Idle>("idle1", 
                std::make_unique<Responder>("responder1"),
                std::make_unique<Idle>("idle2",
                    std::make_unique<Responder>("responder2")
                )
            )
        );    
    
    tick_tock.process_event(Start{});
}

namespace Concepts
{
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

    template <node Root_, tree...Children_>
    struct Tree 
    {
        Root_ root;
        std::tuple<Children_...> children;
        static constexpr std::size_t child_count = sizeof...(Children_);

        Tree() = default;

        Tree(std::convertible_to<Root_> auto&& root_node) : root{std::forward<decltype(root)>(root_node)}
        {}

        template <std::size_t index_>
            requires (index_ < sizeof...(Children_))
        tree auto& child() 
        {
            return std::get<index_>(children);
        }
    };

    // tests for tree concept
    Tree<int> t;
    static_assert(node<decltype((t.root))>);
    static_assert(tree<Tree<int>>);
    static_assert(tree<Tree<int, Tree<int>>>);

    template <typename T>
    concept tree_context = requires (T t) {
        t.tree; requires tree<std::remove_reference_t<decltype(t.tree)>>;
        //{ t.location } -> tree_location;
    };

    template <typename T>
    concept message = std::is_object_v<T>;

    template <tree Tree_ , typename TreeLocation_>
    struct TreeContext
    {
        Tree_& tree;
        TreeLocation_ location;

        void send_down() {}
        void send_up(message auto msg) 
        {
            if constexpr(!location.root) {
                
            }
        }        
    };

  
    struct TickTock
    {        
        void handle(const Start& msg, tree_context auto context)
        {
            puts("Tick...");
            context.send_down(Tick{});
        }
        
        void handle(const Tock& msg, tree_context auto context)
        {
            puts("Tock!");
        }
    };

    struct Responder
    {        
        void handle(const Tick& msg, tree_context auto context)
        {
            context.send_up(Tock{});
        }
    };
}

TEST_CASE("tick-tock with concepts")
{
    using namespace Concepts;

    // Tree<int, Tree<int>> tree;

    // auto tree_backup = tree;

    // auto root_value = tree.child<0>().root;
}