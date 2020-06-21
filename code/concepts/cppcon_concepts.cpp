#include "catch.hpp"
#include <iostream>

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