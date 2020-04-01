#ifndef COMMAND_ADDVOCAB_HPP
#define COMMAND_ADDVOCAB_HPP

#include <command/command.hpp>

namespace sword
{

namespace command
{

class AddVocab : public Command
{
public:
    void execute(Application* app) override;
    const char* getName() const override { return "add_vocab";}
    void set(const std::vector<std::string>* vPtr) { this->vocabPtr = vPtr; }
private:
    const std::vector<std::string>* vocabPtr;
};

}; // namespace command

}; // namespace sword

#endif /* end of include guard: COMMAND_UPDATEVOCAB_HPP */
