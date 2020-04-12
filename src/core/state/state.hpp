#ifndef STATE_HPP_
#define STATE_HPP_

#include <command/commandtypes.hpp>
#include <state/report.hpp>
#include <state/editstack.hpp>
#include <types/stack.hpp>
#include <types/map.hpp>
#include <functional>
#include <bitset>
#include <event/event.hpp>
#include <command/updatevocab.hpp>
#include <command/addvocab.hpp>
#include <command/popvocab.hpp>

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
using ExitCallbackFn = std::function<void()>;
using OptionMask = std::bitset<32>;

class State
{
public:
    virtual void handleEvent(event::Event* event) = 0;
    virtual const char* getName() const = 0;
    virtual ~State() = default;
    void onEnter();
    void onExit();
//    virtual std::vector<const Report*> getReports() const {return {};};
protected:
    State(CommandStack& cs) : cmdStack{cs} {}
    State(CommandStack& cs, ExitCallbackFn callback) : cmdStack{cs}, onExitCallback{callback} {}
//    State(CommandStack& cs, ReportCallbackFn callback) : cmdStack{cs}, reportCallback{callback} {}
//    State(EditStack& editStack, CommandStack& cmdStack) : stateEdits{editStack}, cmdStack{cmdStack}, vocab{cmdStack} {}
//    State(EditStack& editStack, CommandStack& cmdStack, ReportCallbackFn callback) : 
//        stateEdits{editStack}, cmdStack{cmdStack}, vocab{cmdStack}, reportCallback{callback} {}
    void pushCmd(CmdPtr ptr);
    void setVocab(std::vector<std::string> strings);
    void clearVocab() { vocab.clear(); }
    void addToVocab(std::string word) { vocab.push_back(word); }
    void updateVocab();
    void printVocab();
//    inline static Command::Pool<Command::RefreshState> refreshPool{10};
private:
    CommandPool<command::UpdateVocab> uvPool{1};
    CommandPool<command::PopVocab> pvPool{1};
    CommandPool<command::AddVocab> avPool{1};
    std::vector<std::string> vocab;
    CommandStack& cmdStack;
    ExitCallbackFn onExitCallback{nullptr};
    virtual void onEnterExt() {}
    virtual void onExitExt() {}
};

class LeafState : public State
{
public:
    ~LeafState() = default;
protected:
    LeafState(EditStack& es, CommandStack& cs) :
        State{cs}, editStack{es} {}
    LeafState(EditStack& es, CommandStack& cs, ReportCallbackFn callback) :
        State{cs}, editStack{es}, reportCallback{callback} {}
    void popSelf() { editStack.popState(); }
    ReportCallbackFn reportCallback{nullptr};
private:
    EditStack& editStack;
};

using Option = uint8_t;
using Optional = std::optional<Option>;

class BranchState : public State
{
friend class Director;
public:
    ~BranchState() = default;
    void onResume();
    void onPush();
    template<typename R>
    void addReport(Report* ptr, std::vector<std::unique_ptr<R>>* derivedReports)
    {
        auto derivedPtr = dynamic_cast<R*>(ptr); 
        if (derivedPtr)
            derivedReports->emplace_back(derivedPtr);
        else
        {
            std::cout << "Bad report pointer passed." << std::endl;
            delete ptr;
        }
    }
protected:
    using Element = std::pair<std::string, Option>;
    BranchState(EditStack& es, CommandStack& cs, 
            std::initializer_list<Element> ops) :
        State{cs}, editStack{es}, options{ops} {}
    void pushState(State* state);
    Optional extractCommand(event::Event*);
    void activate(Option op) { topMask.set(op); }
    void deactivate(Option op) { topMask.reset(op); }
    void keepOn(Option op) { lowMask.set(op); }
    void updateActiveVocab();

private:
    virtual void onResumeExt() {}
    virtual void onPushExt() {}
    void onEnterExt() override { setActiveVocab(); }
    void setActiveVocab();
    void filterOutVocab();
    OptionMask topMask;
    OptionMask lowMask;
    State* activeChild{nullptr};
    std::vector<const Report*> reports;
    SmallMap<std::string, Option> options;
    EditStack& editStack;
    bool active = true;
};

}; //state

}; //sword

#endif /* end of include guard: STATE_HPP_ */
