#include <state/descriptormanager.hpp>

namespace sword
{

namespace state
{

DescriptorManager::DescriptorManager(EditStack& es, CommandStack& cs, ExitCallbackFn cb) :
    BranchState{es, cs, cb, {}}
{
}

}; // namespace state

}; // namespace sword
