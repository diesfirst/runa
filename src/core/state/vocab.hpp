#ifndef STATE_VOCAB_HPP
#define STATE_VOCAB_HPP

#include "option.hpp"
#include <iostream>

namespace sword
{

namespace state
{

class Vocab
{
public:
    void clear() { words.clear(); }
    void push_back(std::string word) { words.push_back(word); }
    void set( std::vector<std::string> strings) { words = strings; }
    void setMaskPtr( OptionMask* ptr ) { mask = ptr; }
    std::vector<std::string> getWords() const 
    {
        if (mask)
        {
            std::vector<std::string> wordsReturn;
            for (int i = 0; i < words.size(); i++) 
                if ((*mask)[i])
                    wordsReturn.push_back(words.at(i));
            return wordsReturn;
        }
        return words;
    }
    void print()
    {
        if (mask)
        {
            for (int i = 0; i < words.size(); i++) 
                if ((*mask)[i])
                    std::cout << words.at(i) << " ";
        }
        else
        {
            for (int i = 0; i < words.size(); i++) 
                std::cout << words.at(i) << " ";
        }
        std::cout << std::endl;
    }
private:
    OptionMask* mask{nullptr};
    std::vector<std::string> words;
};

}; // namespace state

}; // namespace sword

#endif /* end of include guard: STATE_VOCAB_HPP */
