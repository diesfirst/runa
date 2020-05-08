#ifndef EVENT_EVENT_HPP
#define EVENT_EVENT_HPP

#include <xcb/xcb.h>
#include <render/surface/window.hpp>
#include <queue>
#include <fstream>
#include <sstream>

namespace sword
{

namespace state { class Vocab; }

namespace event
{

namespace symbol
{

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

}

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
    symbol::MouseButton mouseButton;
    symbol::Key key;
    WindowEventType eventType;
};

enum class Category : uint8_t
{
    CommandLine,
    Window,
    Abort,
    File,
    Nothing,
};

class Event
{
public:
    virtual ~Event() = default;
    virtual Category getCategory() const = 0;
    virtual std::string getName() const = 0;
    template <typename... Args> void set(Args... args) {}
    inline bool isAvailable() const {return !inUse;}
    inline void setHandled() {handled = true;}
    inline bool isHandled() const {return handled;}
    inline static Category unserializeCategory(std::ifstream& is)
    {
        Category ec;
        is.read((char*)&ec, sizeof(Category));
        if (ec == Category::CommandLine)
            return Category::CommandLine;
        else if (ec == Category::Abort)
            return Category::Abort;
        else
        {
            throw std::runtime_error("Could not unserialize category");
        }
    }
    void reset() {inUse = false; handled = false;}
    void activate() {inUse = true;}
protected:
    bool handled{false};
    bool inUse{false};
};

class File : public Event
{
public:
    Category getCategory() const override { return Category::File; }
    std::string getName() const override { return "File"; }
    void set(int wd, std::string path) { this->wd = wd; this->path = path; }
    std::string getPath() const { return path; }
    int getWd() const { return wd; }
private:
    int wd;
    std::string path;
};

class Abort : public Event
{
public:
   Category getCategory() const override {return Category::Abort;} 
   std::string getName() const override {return "Abort";}
   void set() {}
   void serialize(std::ofstream& os) 
   {
       auto cat = getCategory();
       os.write((char*)&cat, sizeof(Category));
   }
};

class Nothing : public Event
{
public:
   inline Category getCategory() const override {return Category::Nothing;} 
   inline std::string getName() const override {return "Nothing";}
};

class CommandLine: public Event
{
public:
    void set(std::string input) { this->input = input;}
    Category getCategory() const override {return Category::CommandLine;}
    std::string getName() const override {return "CommandLine";};
    std::string getInput() const {return input;}
    std::string getFirstWord() const 
    {
        std::stringstream ss{input};
        std::string word; ss >> word;
        return word;
    }
    template<typename T>
    T getFirst() const
    {
        std::stringstream ss{input};
        T item; ss >> item;
        return item;
    }
    template<typename T, int I>
    T getArg()
    {
        std::stringstream ss{input};
        std::string catcher;
        T arg;
        for (int i = 0; i < I; i++) 
            ss >> catcher;   
        ss >> arg; 
        return arg;
    }
    std::stringstream getStream() const
    {
        return std::stringstream{input};
    }
    inline void serialize(std::ofstream& os) {
        auto cat = getCategory();
        os.write((char*)&cat, sizeof(Category));
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
    std::string input;
};

class Window: public Event
{
public:
    inline Category getCategory() const override {return Category::Window;}
    virtual WindowEventType getType() const = 0;
    inline int16_t getX() const {return xPos;}
    inline int16_t getY() const {return yPos;}
protected:
    int16_t xPos, yPos;
};

class Keyboard : public Window
{
public:
    symbol::Key getKey() const {return key;}

protected:
    symbol::Key key;
};

class KeyPress final : public Keyboard
{
public:
    void set(int16_t x, int16_t y, symbol::Key key) {xPos = x; yPos = y; this->key = key;}
    inline WindowEventType getType() const override {return WindowEventType::Keypress;}
    inline std::string getName() const override {return "KeyPressEvent";};
};

class KeyRelease final : public Keyboard
{
public:
    void set(int16_t x, int16_t y, symbol::Key key) {xPos = x; yPos = y; this->key = key;}
    inline WindowEventType getType() const override {return WindowEventType::Keypress;}
    inline std::string getName() const override {return "KeyReleaseEvent";};
};

class MouseButton : public Window
{
public:
    symbol::MouseButton getMouseButton() const {return button;}
protected:
    symbol::MouseButton button;
};

class MousePress final : public MouseButton
{
public:
    void set(int16_t x, int16_t y, symbol::MouseButton button) {xPos = x; yPos = y; this->button = button;}
    inline WindowEventType getType() const override {return WindowEventType::MousePress;}
    inline std::string getName() const override {return "MousePressEvent";};
};

class MouseRelease final : public MouseButton
{
public:
    void set(int16_t x, int16_t y, symbol::MouseButton button) {xPos = x; yPos = y; this->button = button;}
    inline WindowEventType getType() const override {return WindowEventType::MouseRelease;}
    inline std::string getName() const override {return "MouseReleaseEvent";};
};

class MouseMotion final : public Window
{
public:
    void set(int16_t x, int16_t y) {xPos = x; yPos = y;}
    inline WindowEventType getType() const override {return WindowEventType::Motion;}
    inline std::string getName() const override {return "MouseMotionEvent";};
};

enum class InputMode : uint8_t
{
    CommandLine,
    Window,
};


}; // namespace event


}; // namespace sword



#endif /* end of include guard: EVENT_EVENT_HPP */


