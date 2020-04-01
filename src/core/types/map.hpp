#include <optional>
#include <vector>


namespace sword
{

template <typename S, typename T>
class SmallMap
{
public:
    using Element = std::pair<S, T>;
    SmallMap(std::initializer_list<Element> avail) : options{avail} {}
//    inline std::optional<T> findOption(CommandLineEvent* event) const
//    {
//        std::string input = event->getInput();
//        std::stringstream instream{input};
//        instream >> input;
//        return findOption(input);
//    }
    std::optional<T> findOption(const S& s) const 
    {
        for (const auto& item : options) 
            if (item.first == s)
                return item.second;
        return {};
    }

    void move(T t, SmallMap<S,T>& other) 
    {
        const size_t size = options.size();
        for (int i = 0; i < size; i++) 
           if (options[i].second == t)
           {
               Element element = options[i];
               other.push(element);
               options.erase(options.begin() + i); 
           }
    }

    void remove(T t)
    {
        size_t size = options.size();
        for (int i = 0; i < size; i++) 
           if (options[i].second == t)
               options.erase(options.begin() + i); 
    }

    std::vector<S> getStrings() const
    {
        std::vector<std::string> vec;
        vec.reserve(options.size());
        for (const auto& item : options) 
            vec.push_back(item.first);
        return vec;
    }

    std::string stringAt(int i) const { return options.at(i).first; }
    
    void push(Element element)
    {
        options.push_back(element);
    }

    T end()
    {
        return options.end();
    }

    size_t size() const { return options.size(); }

private:
    std::vector<std::pair<S, T>> options;
};


}; //sword
