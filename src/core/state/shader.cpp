#include <state/shader.hpp>
#include <filesystem>
#include <sstream>

namespace sword
{

namespace state
{

void LoadFragShaders::onEnterExt()
{
    std::vector<std::string> words;
    auto dir = std::filesystem::directory_iterator(SHADER_DIR);
    for (const auto& entry : dir) 
        words.push_back(entry.path().filename());
    setVocab(words);
    std::cout << "Enter the names of the fragment shaders to load." << std::endl;
}

void LoadFragShaders::handleEvent(event::Event* event)
{
    if (event->getCategory() == event::Category::CommandLine)
    {
        auto cmdevent = static_cast<event::CommandLine*>(event);
        auto stream = cmdevent->getStream();
        std::string reciever;
        while (stream >> reciever)
        {
            auto cmd = lfPool.request(reciever);
            pushCmd(std::move(cmd));
            if (reportCallback)
            {
                auto report = new ShaderReport(reciever, "Frag", 0, 0, 0, 0);
                std::invoke(reportCallback, report);
            }
        }
        event->setHandled();
        popSelf();
    }
}

void LoadVertShaders::onEnterExt()
{
    std::vector<std::string> words;
    auto dir = std::filesystem::directory_iterator(SHADER_DIR);
    for (const auto& entry : dir) 
        words.push_back(entry.path().filename());
    setVocab(words);
    std::cout << "Enter the names of the vert shaders to load." << std::endl;
}

void LoadVertShaders::handleEvent(event::Event* event)
{
    if (event->getCategory() == event::Category::CommandLine)
    {
        auto cmdevent = static_cast<event::CommandLine*>(event);
        auto stream = cmdevent->getStream();
        std::string reciever;
        while (stream >> reciever)
        {
            auto cmd = lvPool.request(reciever);
            pushCmd(std::move(cmd));
            if (reportCallback)
            {
                auto report = new ShaderReport(reciever, "Vert", 0, 0, 0, 0);
                std::invoke(reportCallback, report);
            }
        }
        event->setHandled();
        popSelf();
    }
}

ShaderManager::ShaderManager(EditStack& es, CommandStack& cs)  : 
    BranchState{es, cs,{
        {"load_frag_shaders", opcast(Op::loadFrag)},
        {"load_vert_shaders", opcast(Op::loadVert)},
        {"print_reports", opcast(Op::printReports)}
    }},
    loadFragShaders{es, cs, std::bind(&BranchState::addReport<ShaderReport>, this, std::placeholders::_1, &shaderReports)},
    loadVertShaders{es, cs, std::bind(&BranchState::addReport<ShaderReport>, this, std::placeholders::_1, &shaderReports)}
{   
    activate(opcast(Op::loadFrag));
    activate(opcast(Op::loadVert));
    activate(opcast(Op::printReports));
}

void ShaderManager::handleEvent(event::Event* event)
{
    if (event->getCategory() == event::Category::CommandLine)
    {
        Optional option = extractCommand(event);
        if (!option) return;
        switch(opcast(*option))
        {
            case Op::loadFrag: pushState(&loadFragShaders); break;
            case Op::loadVert: pushState(&loadVertShaders); break;
            case Op::printReports: printReports(); break;
        }
    }
}

void ShaderManager::printReports()
{
    for (const auto& report : shaderReports) 
    {
        std::invoke(*report);
    }
}

//void ShaderManager::addFragShaderReport(Report* ptr)
//{
//    auto shaderReportPtr = dynamic_cast<ShaderReport*>(ptr); 
//    if (shaderReportPtr)
//        shaderReports.emplace_back(shaderReportPtr);
//    else
//    {
//        std::cout << "Bad pointer passed to addFragShaderReport" << std::endl;
//        delete ptr;
//    }
//}

}; // namespace state

}; // namespace sword
