#include <xcb/xcb.h>
#include <X11/Xlib-xcb.h>
#include <array>
#include "window.hpp"
#include <memory>
#include <thread>
#include <queue>
#include <fstream>

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
};

class Event
{
public:
    virtual EventCategory getCategory() const = 0;
    virtual std::string getName() const = 0;
    inline void setHandled() {handled = true;}
    inline bool isHandled() const {return handled;}
protected:
    bool handled{false};
};

class Abort : public Event
{
public:
   inline EventCategory getCategory() const override {return EventCategory::Abort;} 
   inline std::string getName() const override {return "Abort";}
};

class CommandLineEvent : public Event
{
public:
    CommandLineEvent(CommandLineInput input) : input{input} {};
    inline EventCategory getCategory() const override {return EventCategory::CommandLine;}
    inline std::string getName() const override {return "CommandLine";};
    inline CommandLineInput getInput() const {return input;}
    inline void serialize(std::ofstream& os) {
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
};

class KeyEvent : public WindowEvent
{
public:
    Key getKey() const {return key;}

protected:
    KeyEvent(Key key) : key{key} {}
    Key key;
};

class KeyPressEvent : public KeyEvent
{
public:
    KeyPressEvent(Key key) : KeyEvent{key} {}
    inline WindowEventType getType() const override {return WindowEventType::Keypress;}
    inline std::string getName() const override {return "KeyPressEvent";};
};

class KeyReleaseEvent : public KeyEvent
{
public:
    KeyReleaseEvent(Key key) : KeyEvent{key} {}
    inline WindowEventType getType() const override {return WindowEventType::Keypress;}
    inline std::string getName() const override {return "KeyReleaseEvent";};
};

class MouseButtonEvent : public WindowEvent
{
public:
    MouseButton getMouseButton() const {return button;}
protected:
    MouseButtonEvent(MouseButton button) : button{button} {};
    const MouseButton button;
};

class MousePressEvent : public MouseButtonEvent
{
public:
    MousePressEvent(MouseButton button) : MouseButtonEvent{button} {};
    inline WindowEventType getType() const override {return WindowEventType::MousePress;}
    inline std::string getName() const override {return "MousePressEvent";};
};

class MouseReleaseEvent : public MouseButtonEvent
{
public:
    MouseReleaseEvent(MouseButton button) : MouseButtonEvent{button} {};
    inline WindowEventType getType() const override {return WindowEventType::MouseRelease;}
    inline std::string getName() const override {return "MouseReleaseEvent";};
};

class MouseMotionEvent : public WindowEvent 
{
public:
    MouseMotionEvent(int16_t x, int16_t y) : xPos{x}, yPos{y} {}
    inline WindowEventType getType() const override {return WindowEventType::Motion;}
    inline std::string getName() const override {return "MouseMotionEvent";};
    inline int16_t getX() const {return xPos;}
    inline int16_t getY() const {return yPos;}
private:
    const int16_t xPos, yPos;
};

typedef std::queue<std::unique_ptr<Event>> EventQueue;

enum class InputMode : uint8_t
{
    CommandLine,
    Window,
};

//this class is not thread safe at all
class EventHandler
{
public:
	EventHandler(const XWindow&);
	~EventHandler();

    void setVocabulary(std::vector<std::string> vocab);

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
    static std::vector<std::string> vocabulary;
    static char* completion_generator(const char* text, int state);
    static char** completer(const char* text, int start, int end);
    bool keepWindowThread{true};
    bool keepCommandThread{true};
};
