#ifndef COMMAND_POPVOCAB_HPP
#define COMMAND_POPVOCAB_HPP

#include <command/command.hpp>
#include <vector>
#include <string>

namespace sword
{

namespace command
{

class PopVocab : public Command
{
public:
    void execute(Application* app) override;
    const char* getName() const override { return "pop_vocab";}
private:
};

}; // namespace command

}; // namespace sword

#endif /* end of include guard: COMMAND_UPDATEVOCAB_HPP */
