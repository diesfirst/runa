#include <command/vocab.hpp>
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

void SetVocab::execute(Application* app)
{
    app->dispatcher.setVocabulary(vocab);
}

}; // namespace command

}; // namespace sword

