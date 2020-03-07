#include <xcb/xcb.h>
#include <X11/Xlib-xcb.h>
#include <array>
#include "window.hpp"
#include <memory>
#include <thread>
#include <queue>
#include <fstream>
#include <sstream>
#include <optional>

enum class MouseButton : uint8_t
{
    Left = 1,
    Middle = 2,
    Right = 3,
};

enum class Key : uint8_t
{
    R = 27,
    Shift_L = 50,
    C = 54,
    B = 56,
    Alt = 64,
    Esc = 9,
    S = 39,
    A = 38,
    P = 33
};

enum class WindowEventType : uint8_t
{
    MousePress = 4,
    MouseRelease = 5,
    Motion = 6,
    Keypress = 2,
    Keyrelease = 3,
    LeaveWindow = 8,
    EnterWindow = 7,
};

struct WindowInput
{
	int16_t mouseX{0};
	int16_t mouseY{0};
    MouseButton mouseButton;
    Key key;
    WindowEventType eventType;
};

typedef std::string CommandLineInput;

enum class EventCategory : uint8_t
{
    CommandLine,
    Window,
    Abort,
    Nothing,
};

class Event
{
public:
    virtual EventCategory getCategory() const = 0;
    virtual std::string getName() const = 0;
    inline void setHandled() {handled = true;}
    inline bool isHandled() const {return handled;}
    inline static EventCategory unserializeCategory(std::ifstream& is)
    {
        EventCategory ec;
        is.read((char*)&ec, sizeof(EventCategory));
        if (ec == EventCategory::CommandLine)
            return EventCategory::CommandLine;
        else if (ec == EventCategory::Abort)
            return EventCategory::Abort;
        else
        {
            throw std::runtime_error("Could not unserialize category");
        }
    }
protected:
    bool handled{false};
};

class Abort : public Event
{
public:
   inline EventCategory getCategory() const override {return EventCategory::Abort;} 
   inline std::string getName() const override {return "Abort";}
   inline void serialize(std::ofstream& os) 
   {
       auto cat = getCategory();
       os.write((char*)&cat, sizeof(EventCategory));
   }
};

class Nothing : public Event
{
public:
   inline EventCategory getCategory() const override {return EventCategory::Nothing;} 
   inline std::string getName() const override {return "Nothing";}
};

class CommandLineEvent : public Event
{
public:
    CommandLineEvent(CommandLineInput input) : input{input} {};
    inline EventCategory getCategory() const override {return EventCategory::CommandLine;}
    inline std::string getName() const override {return "CommandLine";};
    inline CommandLineInput getInput() const {return input;}
    inline std::string getFirstWord() const 
    {
        std::stringstream ss{input};
        std::string word; ss >> word;
        return word;
    }
    inline void serialize(std::ofstream& os) {
        auto cat = getCategory();
        os.write((char*)&cat, sizeof(EventCategory));
        size_t inputSize = input.size();
        os.write((char*)&inputSize, sizeof(size_t));
        os.write((char*)input.data(), input.size());
        os.write((char*)&handled, sizeof(bool));
    }
    inline void unserialize(std::ifstream& is) {
        size_t inputSize;
        is.read((char*)&inputSize, sizeof(size_t));
        char cstr[inputSize];
        is.read(cstr, inputSize);
        input.assign(cstr, inputSize);
        is.read((char*)&handled, sizeof(bool));
    }
private:
    CommandLineInput input;
};

class WindowEvent : public Event
{
public:
    inline EventCategory getCategory() const override {return EventCategory::Window;}
    virtual WindowEventType getType() const = 0;
    inline int16_t getX() const {return xPos;}
    inline int16_t getY() const {return yPos;}
protected:
    WindowEvent(const int16_t x, const int16_t y) : xPos{x}, yPos{y} {}
    const int16_t xPos, yPos;
};

class KeyEvent : public WindowEvent
{
public:
    Key getKey() const {return key;}

protected:
    KeyEvent(int16_t x, int16_t y, Key key) : WindowEvent{x, y}, key{key} {}
    Key key;
};

class KeyPressEvent : public KeyEvent
{
public:
    KeyPressEvent(int16_t x, int16_t y, Key key) : KeyEvent{x, y, key} {}
    inline WindowEventType getType() const override {return WindowEventType::Keypress;}
    inline std::string getName() const override {return "KeyPressEvent";};
};

class KeyReleaseEvent : public KeyEvent
{
public:
    KeyReleaseEvent(int16_t x, int16_t y, Key key) : KeyEvent{x, y, key} {}
    inline WindowEventType getType() const override {return WindowEventType::Keypress;}
    inline std::string getName() const override {return "KeyReleaseEvent";};
};

class MouseButtonEvent : public WindowEvent 
{
public:
    MouseButton getMouseButton() const {return button;}
protected:
    MouseButtonEvent(int16_t x, int16_t y, MouseButton button) : WindowEvent{x, y}, button{button} {};
    const MouseButton button;
};

class MousePressEvent : public MouseButtonEvent
{
public:
    MousePressEvent(int16_t x, int16_t y, MouseButton button) :  MouseButtonEvent{x, y, button} {};
    inline WindowEventType getType() const override {return WindowEventType::MousePress;}
    inline std::string getName() const override {return "MousePressEvent";};
};

class MouseReleaseEvent : public MouseButtonEvent
{
public:
    MouseReleaseEvent(int16_t x, int16_t y, MouseButton button) : MouseButtonEvent{x, y, button} {};
    inline WindowEventType getType() const override {return WindowEventType::MouseRelease;}
    inline std::string getName() const override {return "MouseReleaseEvent";};
};

class MouseMotionEvent : public WindowEvent 
{
public:
    MouseMotionEvent(int16_t x, int16_t y) : WindowEvent{x, y} {}
    inline WindowEventType getType() const override {return WindowEventType::Motion;}
    inline std::string getName() const override {return "MouseMotionEvent";};
};

typedef std::queue<std::unique_ptr<Event>> EventQueue;

enum class InputMode : uint8_t
{
    CommandLine,
    Window,
};

using Vocab = std::vector<std::string>;

//this class is not thread safe at all
class EventHandler
{
public:
	EventHandler(const XWindow&);
	~EventHandler();

    void setVocabulary(std::vector<std::string> vocab);
    static void updateVocab();
    void addVocab(const Vocab*);
    void popVocab();

	void fetchWindowInput();
    void fetchCommandLineInput();
    void pollEvents();

    void runCommandLineLoop();
    void runWindowInputLoop();

    void getNextEvent();

    WindowInput windowInput;
    CommandLineInput commandLineInput;
    inline void setInputMode(InputMode mode) {inputMode = mode;}

    EventQueue eventQueue;

private:
	const XWindow& window;
    InputMode inputMode{InputMode::CommandLine};

//    static uint8_t wordCount;
//    static const char** vocabulary;
    inline static Vocab vocabulary{};
    inline static std::vector<const Vocab*> vocabPtrs;
    static char* completion_generator(const char* text, int state);
    static char** completer(const char* text, int start, int end);
    bool keepWindowThread{true};
    bool keepCommandThread{true};
};
