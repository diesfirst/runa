#ifndef VOCAB_HPP_
#define VOCAB_HPP_

#include <command/commandtypes.hpp>
#include <types/pool.hpp>
#include <command/updatevocab.hpp>

namespace sword
{

namespace command { class UpdateVocab; };

class Vocab
{
public:
    Vocab(CommandStack& cmdStack) : cmdStack{cmdStack} {}
    Vocab& operator=(const std::vector<std::string> strings)
    {
        vocab = strings;
        auto cmd = uvPool.request();
        cmdStack.push(std::move(cmd));
        return *this;
    }
    const std::vector<std::string>* getValues() const {return &vocab;}
    inline auto begin() const {return vocab.begin();}
    inline auto end() const {return vocab.end();}

private:
    CommandPool<command::UpdateVocab> uvPool{1};
    CommandStack& cmdStack;
    std::vector<std::string> vocab;
};

}; //sword

#endif /* end of include guard: VOCAB_HPP_ */
