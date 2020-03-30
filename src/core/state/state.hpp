#ifndef STATE_HPP_
#define STATE_HPP_

#include <command/commandtypes.hpp>
#include <state/report.hpp>
#include <state/editstack.hpp>
#include <types/vocab.hpp>
#include <types/stack.hpp>
#include <types/map.hpp>
#include <functional>
#include <bitset>
#include <event/event.hpp>

#define STATE_BASE(name) \
    void handleEvent(Event* event) override;\
    virtual const char* getName() const override {return name;}

namespace sword { namespace state { class State; }; };

namespace sword
{

//namespace event{ class Event;}
    

namespace state
{

using ReportCallbackFn = std::function<void(Report*)>;
using OptionMask = std::bitset<32>;

struct StateData
{
    StateData(EditStack& es, CommandStack& cs) :
        cmdStack{cs}, editStack{es}, vocab{cs} {}
    CommandStack& cmdStack;
    EditStack& editStack;
    Vocab vocab;
};

class State
{
public:
    virtual void handleEvent(event::Event* event) = 0;
    virtual const char* getName() const = 0;
    virtual ~State() = default;
    void onEnter(Application*);
    void onExit(Application*);
//    virtual std::vector<const Report*> getReports() const {return {};};
protected:
    State(CommandStack& cs) : cmdStack{cs}, vocab{cs} {}
    State(CommandStack& cs, ReportCallbackFn callback) : cmdStack{cs}, vocab{cs}, reportCallback{callback} {}
//    State(EditStack& editStack, CommandStack& cmdStack) : stateEdits{editStack}, cmdStack{cmdStack}, vocab{cmdStack} {}
//    State(EditStack& editStack, CommandStack& cmdStack, ReportCallbackFn callback) : 
//        stateEdits{editStack}, cmdStack{cmdStack}, vocab{cmdStack}, reportCallback{callback} {}
    void pushCmd(CmdPtr ptr);
    Vocab vocab;
//    inline static Command::Pool<Command::RefreshState> refreshPool{10};
private:
    ReportCallbackFn reportCallback{nullptr};
    CommandStack& cmdStack;
    virtual void onEnterExt(Application*) {}
    virtual void onExitExt(Application*) {}
};

class LeafState : public State
{
public:
    ~LeafState() = default;
protected:
    LeafState(EditStack& es, CommandStack& cs) :
        State{cs}, editStack{es} {}
    LeafState(EditStack& es, CommandStack& cs, ReportCallbackFn callback) :
        State{cs, callback}, editStack{es} {}
    void popSelf() { editStack.popState(); }
private:
    EditStack& editStack;
};

using Option = uint8_t;
using Optional = std::optional<Option>;

class BranchState : public State
{
public:
    ~BranchState() = default;
    virtual void dynamicOpsActive(bool) = 0;
    void onResume(Application* app) {} //TODO: must define better
    void onPush(Application* app) {} //TODO: must define better
protected:
    using Element = std::pair<std::string, Option>;
    BranchState(EditStack& es, CommandStack& cs, 
            std::initializer_list<Element> ops) :
        State{cs}, editStack{es}, options{ops} 
    {
        vocab = options.getStrings();
    }
    void pushState(State* state);
    Optional extractCommand(event::Event*);

    OptionMask opmask;
private:
    virtual void onResumeExt(Application*) {}
    virtual void onPushExt(Application*) {}
    State* activeChild{nullptr};
    std::vector<const Report*> reports;
    SmallMap<std::string, Option> options;
    EditStack& editStack;
};

}; //state

}; //sword

#endif /* end of include guard: STATE_HPP_ */
