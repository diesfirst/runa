#ifndef COMMAND_UPDATEVOCAB_HPP
#define COMMAND_UPDATEVOCAB_HPP

#include <command/command.hpp>

namespace sword
{

namespace command
{

class UpdateVocab : public Command
{
public:
    void execute(Application*) override;
    const char* getName() const override { return "update_vocab";}
private:
};

}; // namespace command

}; // namespace sword

#endif /* end of include guard: COMMAND_UPDATEVOCAB_HPP */
