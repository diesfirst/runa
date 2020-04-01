#include <command/updatevocab.hpp>
#include <command/popvocab.hpp>
#include <command/addvocab.hpp>
#include <application.hpp>

namespace sword
{

namespace command
{

void UpdateVocab::execute(Application* app)
{
    app->dispatcher.updateVocab();
}

void AddVocab::execute(Application* app)
{
    app->dispatcher.addVocab(vocabPtr);
}

void PopVocab::execute(Application* app)
{
    app->dispatcher.popVocab();
}

}; // namespace command

}; // namespace sword

