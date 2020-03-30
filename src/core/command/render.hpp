#include "command.h"

class render : public Command
{
public:
    CMD_BASE("render");
    void set() {}
private:
// parms
    int renderCommandId{0};
    bool updateUBO{false};
}
