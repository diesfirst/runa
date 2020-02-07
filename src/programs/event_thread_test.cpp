#include "../core/event.hpp"

const char* words[] = {"foo", "bar"};

bool quit{false};
bool windowShouldClose{false};

class Command
{
public:
    virtual void execute();
    virtual const char* getName() const;
};

class ReportStuff : public Command 
{
public:
    ReportStuff();
    const char* getName() const {return "reportStuff";}
};

class AppState;

class Application
{
public:
    Application() 
    {
    }
    void setCommands();
private:
    AppState* initialState;
};

class AppState
{
public:
    virtual Command* handleEvent(Application*, Event* event);
    virtual void onEnter(Application*);
private:
};

class InitialState : public AppState
{
public:
    Command* handleEvent(Application* app, Event* event) override
    {
        if (event->getCategory() == EventCategory::CommandLine)
        {
            auto input = static_cast<CommandLineEvent*>(event)->getInput();
            for (auto cmd : cmds) {
                if (cmd->getName() == input)
                    return cmd;
            }
        }
        return nullptr;
    }

    void onEnter(Application* app) override
    {
        app->setCommands();
    }
private:
    static std::vector<Command*> cmds;
};

Application app;

std::vector<Command*> InitialState::cmds = {&app.reportCmd};

void handleEvent(Event* event)
{
    std::cout << "Event:" << event << std::endl;
    std::cout << "Event type: " << event->getName() << std::endl;
    switch (event->getCategory())
    {
        case EventCategory::CommandLine:
            {
                auto e = static_cast<CommandLineEvent*>(event);
                auto input = e->getInput();
                if (input == "quit")
                    quit = true;
                break;
            }
        case EventCategory::Window:
            {
                auto e = (WindowEvent*)event;
                if (e->getType() == WindowEventType::Keypress)
                {
                    auto a = (KeyPressEvent*)event;
                    if (a->getKey() == Key::Esc)
                        windowShouldClose = true;
                }
                break;
            }
    }
}

int main(int argc, const char *argv[])
{
    XWindow window{500, 500};
    EventHandler ev{window};
    window.open();

    ev.setVocabulary(words, 2);
    ev.pollEvents();
    while (!quit)
    {
        std::cout << "Events in queue: " << ev.eventQueue.size() << std::endl;
        while (!ev.eventQueue.empty())
        {
            auto event = ev.eventQueue.front().get();
            if (event)
                handleEvent(event);
            ev.eventQueue.pop();
        }
        if (windowShouldClose)
        std::cout << "tick." << std::endl;
        sleep(1);
    }
    return 0;
}


