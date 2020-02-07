#include "../core/event.hpp"

class AppState;
class ReportStuff;
class Application;
class LoadShaderState;
class Command;

class Application
{
public:
    Application(EventHandler& ev) : ev{ev} {}
    void setVocabulary(std::vector<std::string>);
    void run();
private:
    const AppState* state;
    EventHandler& ev;
};

class AppState
{
public:
    virtual Command* handleEvent(Application*, Event* event) const = 0;
    virtual void onEnter(Application*) const = 0;
};

class InitialState : public AppState
{
public:
    Command* handleEvent(Application* app, Event* event) const override;
    void onEnter(Application* app) const override;
private:
    static std::vector<Command*> cmds;
};

class FooState : public AppState
{
public:
    Command* handleEvent(Application* app, Event* event) const override;
    void onEnter(Application* app) const override;
private:
    static std::vector<Command*> cmds;
};

class LoadShaderState : public AppState
{
public:
    Command* handleEvent(Application* app, Event* event) const override;
    void onEnter(Application* app) const override;
};

class Command
{
public:
    virtual void execute(Application*) = 0;
    virtual const char* getName() const = 0;
    virtual const AppState* nextState() const = 0;
};

class ReportStuff : public Command 
{
public:
    inline ReportStuff() {};
    inline const char* getName() const override {return "reportStuff";}
    const AppState* nextState() const override;
    void execute(Application*) override;
};

class PrintWee: public Command 
{
public:
    inline PrintWee() {};
    inline const char* getName() const override {return "printWee";}
    const AppState* nextState() const override;
    void execute(Application*) override;
};

class LoadShaders: public Command
{
public:
    inline LoadShaders() {};
    inline const char* getName() const override {return "loadShaders";}
    const AppState* nextState() const override {return &nState;}
    void execute(Application*) override {} //just return immediately
private:
    const LoadShaderState nState;
};
