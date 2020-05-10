#include <state/shader.hpp>
#include <filesystem>
#include <sstream>
#include <util/file.hpp>
#include <util/debug.hpp>

namespace sword
{

namespace state
{

PrintShader::PrintShader(StateArgs sa, Callbacks cb) :
    LeafState{sa, cb}
{}

void PrintShader::onEnterExt()
{
    setVocab(getPaths(shader_dir::src, true));
    std::cout << "Enter a shader path." << '\n';
}

void PrintShader::handleEvent(event::Event* event)
{
    if (event->getCategory() == event::Category::CommandLine)
    {
        auto ce = toCommandLine(event);
        auto input = ce->getArg<std::string, 0>();
        auto path = std::string(shader_dir::src) + "/" + input;

        std::ifstream f(path);

        if (f.is_open())
        {
            std::cout << '\n';
            std::cout << f.rdbuf() << '\n';
            std::cout << '\n';
            f.close();
        }
        else
            std::cout << "Could not open file" << '\n';
        popSelf();
        event->setHandled();
    }
}

CompileShader::CompileShader(StateArgs sa, Callbacks cb) :
    LeafState{sa, cb}, pool{sa.cp.compileShader}
{
    sa.rg.compileShader = this;
}

void CompileShader::onEnterExt()
{
    setVocab(getPaths(SHADER_SRC, true));
    std::cout << "Enter a shader path and name." << '\n';
}

void CompileShader::handleEvent(event::Event* event)
{
    if (event->getCategory() == event::Category::CommandLine)
    {
        auto ce = toCommandLine(event);       
        auto path = ce->getArg<std::string, 0>();
        auto name = ce->getArg<std::string, 1>();
        if (name.empty())
        {
            std::cout << "Must specify a shader name." << '\n';
            event->setHandled();
            popSelf();
            return;
        }
        auto cmd = pool.request(reportCallback(), path, name);
        pushCmd(std::move(cmd));
        event->setHandled();
        popSelf();
    }
}

WatchFile::WatchFile(StateArgs sa, Callbacks cb) :
    LeafState{sa, cb}, pool{sa.cp.watchFile}
{}

void WatchFile::onEnterExt()
{
    setVocab(getPaths(SHADER_SRC, true));
    std::cout << "Enter a file to watch." << '\n';
}

void WatchFile::handleEvent(event::Event* event)
{
    if (event->getCategory() == event::Category::CommandLine)
    {
        auto ce = toCommandLine(event);
        auto rel_path = ce->getFirstWord();
        std::string path = std::string(SHADER_SRC) + "/" + rel_path;
        std::cout << "Setting watch on " << path << '\n';
        auto cmd = pool.request(reportCallback(), path);
        pushCmd(std::move(cmd));
        event->setHandled();
        popSelf();
    }
}

LoadFragShaders::LoadFragShaders(StateArgs sa, Callbacks cb) :
    LeafState{sa, cb}, lfPool{sa.cp.loadFragShader} 
{
    sa.rg.loadFragShaders = this;
}

void LoadFragShaders::onEnterExt()
{
    setVocab(getPaths(frag_dir));
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
            auto spv = toSpv(reciever);
            auto cmd = lfPool.request(reportCallback(), spv);
            pushCmd(std::move(cmd));
        }
        event->setHandled();
        popSelf();
    }
}

LoadVertShaders::LoadVertShaders(StateArgs sa, Callbacks cb) :
    LeafState{sa, cb}, lvPool{sa.cp.loadVertShader}
{
    sa.rg.loadVertShaders = this;
}

void LoadVertShaders::onEnterExt()
{
    setVocab(getPaths(vert_dir));
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
            auto spv = toSpv(reciever);
            auto cmd = lvPool.request(reportCallback(), spv);
            pushCmd(std::move(cmd));
        }
        event->setHandled();
        popSelf();
    }
}

SetSpec::SetSpec(StateArgs sa, Callbacks cb, shader::SpecType t, ShaderReports& reports) :
    LeafState{sa, cb}, type{t}, reports{reports}, ssiPool{sa.cp.setSpecInt}, ssfPool{sa.cp.setSpecFloat}
{
    sa.rg.setSpec = this;
}

void SetSpec::onEnterExt()
{
    std::vector<std::string> words;
    for (const auto& report : reports) 
    {
        words.push_back(report->getObjectName());
    }
    setVocab(words);
    std::cout << "Enter two numbers, an 'f' or 'v', and the shader names" << std::endl;
}

void SetSpec::handleEvent(event::Event* event)
{
    if (event->getCategory() == event::Category::CommandLine)
    {
        auto stream = toCommandLine(event)->getStream();
        int first; int second; char type_char; std::string name;
        ShaderType type;
        stream >> first >> second >> type_char;
        if (type_char == 'f')
            type = ShaderType::frag;
        else if (type_char == 'v')
            type = ShaderType::vert;
        while (stream >> name)
        {
            CmdPtr cmd;
            auto report = findReport<ShaderReport>(name, reports);
            if (this->type == shader::SpecType::floating)
            {
                cmd = ssfPool.request(name, type, first, second);
                report->setSpecFloat(0, first);
                report->setSpecFloat(1, second);
            }
            else 
            {
                cmd = ssiPool.request(name, type, first, second);
                report->setSpecInt(0, first);
                report->setSpecInt(1, second);
            }
            pushCmd(std::move(cmd));
        }
        event->setHandled();
        popSelf();
    }
}

ShaderManager::ShaderManager(StateArgs sa, Callbacks cb, ReportCallbackFn<ShaderReport> srcb)  : 
    BranchState{sa, cb, {
        {"load_frag_shaders", opcast(Op::loadFrag)},
        {"load_vert_shaders", opcast(Op::loadVert)},
        {"print_reports", opcast(Op::printReports)},
        {"set_spec_int", opcast(Op::setSpecInt)},
        {"set_spec_float", opcast(Op::setSpecFloat)},
        {"print_shader", opcast(Op::printShader)},
        {"compile_shader", opcast(Op::compileShader)},
        {"watch_file", opcast(Op::watchFile)}
    }},
    loadFragShaders{sa, {nullptr, [this, srcb](Report* report){ addReport(report, &shaderReports, srcb); }}},
    loadVertShaders{sa, {nullptr, [this, srcb](Report* report){ addReport(report, &shaderReports, srcb); }}},
    compileShader{sa, {nullptr, [this, srcb](Report* report){ addReport(report, &shaderReports, srcb); }}},
    watchFile{sa, {}},
    setSpecInt{sa, {}, shader::SpecType::integer, shaderReports},
    setSpecFloat{sa, {}, shader::SpecType::floating, shaderReports},
    printShader{sa, {}},
    csPool{sa.cp.compileShader},
    srCallback{srcb}
{   
    activate(opcast(Op::loadFrag));
    activate(opcast(Op::loadVert));
    activate(opcast(Op::printReports));
    activate(opcast(Op::setSpecFloat));
    activate(opcast(Op::setSpecFloat));
    activate(opcast(Op::printShader));
    activate(opcast(Op::compileShader));
    activate(opcast(Op::watchFile));
}

void ShaderManager::handleEvent(event::Event* event)
{
    if (event->getCategory() == event::Category::CommandLine)
    {
        Optional option = extractCommand(event);
        if (!option) return;
        switch(opcast<Op>(*option))
        {
            case Op::loadFrag: pushState(&loadFragShaders); break;
            case Op::loadVert: pushState(&loadVertShaders); break;
            case Op::printReports: printReports(); break;
            case Op::setSpecInt: pushState(&setSpecInt); break;
            case Op::setSpecFloat: pushState(&setSpecFloat); break;
            case Op::printShader: pushState(&printShader); break;
            case Op::compileShader: pushState(&compileShader); break;
            case Op::watchFile: pushState(&watchFile); break;
        }
    }
    if (event->getCategory() == event::Category::File)
    {
        //this algorithm ONLY works for fragment shaders currently... it should be fixed
        auto fe = static_cast<event::File*>(event);
        auto path = fe->getPath();
        for (const auto& report : shaderReports) 
        {
            std::invoke(*report);
            if (report->getSourcePath() == path)
            {
                auto rep = report.get();
                auto cmd = csPool.request(
                        OwningReportCallbackFn([this](Report* r) {
                            auto rep = static_cast<ShaderReport*>(r);
                            std::invoke(srCallback, rep);
                            }),
                        path, rep->getObjectName(), rep);
                pushCmd(std::move(cmd));
                break;
            }
        }
        event->setHandled();
    }
}

void ShaderManager::printReports()
{
    for (const auto& report : shaderReports) 
    {
        std::invoke(*report);
    }
}

}; // namespace state

}; // namespace sword
