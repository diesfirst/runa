#include <command/vocab.hpp>
#include <application.hpp>

namespace sword
{

namespace command
{

void UpdateVocab::execute(Application* app)
{
    app->dispatcher.updateVocab();
    success();
}

void AddVocab::execute(Application* app)
{
    app->dispatcher.addVocab(vocabPtr);
    success();
}

void PopVocab::execute(Application* app)
{
    app->dispatcher.popVocab();
    success();
}

void SetVocab::execute(Application* app)
{
    app->dispatcher.setVocabulary(vocab);
    success();
}

}; // namespace command

}; // namespace sword

