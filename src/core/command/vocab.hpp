#ifndef COMMAND_VOCAB_HPP
#define COMMAND_VOCAB_HPP

#include <command/command.hpp>
#include <vector>
#include <string>

namespace sword
{

namespace state { class Vocab; }

namespace command
{

class UpdateVocab : public Command
{
public:
    void execute(Application*) override;
    const char* getName() const override { return "update_vocab";}
private:
};

class AddVocab : public Command
{
public:
    void execute(Application* app) override;
    const char* getName() const override { return "add_vocab";}
    void set(const state::Vocab* vPtr) { this->vocabPtr = vPtr; }
private:
    const state::Vocab* vocabPtr;
};

class PopVocab : public Command
{
public:
    void execute(Application* app) override;
    const char* getName() const override { return "pop_vocab";}
private:
};

class SetVocab : public Command
{
public:
    void execute(Application* app) override;
    const char* getName() const override { return "set_vocab";}
    void set(const std::vector<std::string> v) { this->vocab = v; }
private:
    std::vector<std::string> vocab;
};

}; // namespace command

}; // namespace sword

#endif /* end of include guard: COMMAND_VOCAB_HPP */
