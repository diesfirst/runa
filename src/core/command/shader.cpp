#include "shader.hpp"
#include "libshaderc_util/file_finder.h"
#include "shaderc/shaderc.hpp"
#include <cstdlib>
#include <iostream>
#include <memory>
#include <util/file.hpp>
#include <filesystem>
#include <sstream>
#include <fstream>
#include <application.hpp>
#include <util/debug.hpp>

namespace sword
{

namespace command
{

CompileShader::CompileShader()
{
    // we could make the file_finder a static member... because it will be the same for all instantiations of this class.
    // not sure if that would affect thread safety though...
    std::unique_ptr<shaderc::CompileOptions::IncluderInterface> includer = std::make_unique<glslc::FileIncluder>(&file_finder);
    compileOptions.SetIncluder(std::move(includer));
    std::filesystem::recursive_directory_iterator iter{src_path};
    for (const auto& d : iter) 
    {
        if (std::filesystem::is_directory(d.path()))
        {
            file_finder.search_path().push_back(d.path());
        }
    }
}

void CompileShader::set(const std::string_view rel_path, const std::string_view name, state::ShaderReport* report)
{
    src_path = SHADER_SRC; //must reset this path so we don't reappend to it
    this->name = name;
    this->report = report;
    src_path.append(rel_path);
    if (src_path.extension() == ".frag")
    {
        kind = shaderc_shader_kind::shaderc_glsl_fragment_shader;
        type = ShaderType::frag;
    }
    else if (src_path.extension() == ".vert")
    {
        kind = shaderc_shader_kind::shaderc_glsl_vertex_shader;
        type = ShaderType::vert;
    }
    else
        std::cerr << "File type not recognised" << '\n';
}

void CompileShader::execute(Application* app)
{
    std::ifstream f;
    std::stringstream ss;
    std::cout << "SRC PATH: " << src_path << '\n';
    f.open(src_path);
    if (!f.fail())
        ss << f.rdbuf();
    else
    {
        std::cerr << "file not open" << '\n';
        return;
    }
    auto result = compiler.CompileGlslToSpv(ss.str(), kind, name.c_str(), compileOptions);
    if (result.GetCompilationStatus() == 0)
    {
        std::vector<uint32_t> code;
        for (const auto& i : result) 
        {
            code.push_back(i);
        }
        switch (type)
        {
            case ShaderType::frag: app->renderer.loadFragShader(std::move(code), name); success(); break;
            case ShaderType::vert: app->renderer.loadVertShader(std::move(code), name); success(); break;
        }
    }
    else
    {
        std::cerr << "CompileShader::execute: failed to compile. Error message: " << result.GetErrorMessage() << '\n';
    }
}

state::Report* CompileShader::makeReport() const
{
    if (report == nullptr)  // take it to mean that no report existed before
        return new state::ShaderReport(name, type, 0, 0, 0.0, 0.0, src_path);
    else
        return report;
}

}; // namespace command

}; // namespace sword
