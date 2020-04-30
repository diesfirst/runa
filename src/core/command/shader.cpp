#include "shader.hpp"
#include <cstdlib>
#include <iostream>
#include <util/file.hpp>
#include <filesystem>
#include <sstream>
#include <fstream>
#include <application.hpp>

namespace sword
{

namespace command
{

void CompileShader::set(const std::string_view rel_path, const std::string_view name, state::ShaderReport* report)
{
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
    auto f = std::ifstream(src_path);
    std::stringstream ss;
    if (f.is_open())
        ss << f.rdbuf();
    else
    {
        std::cerr << "file not open" << '\n';
        return;
    }
    auto result = compiler.CompileGlslToSpv(ss.str(), kind, name.c_str());
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
    f.close();
}

state::Report* CompileShader::makeReport() const
{
    if (report == nullptr)  // take it to mean that no report existed before
        return new state::ShaderReport(name, type, 0, 0, 0.0, 0.0);
    else
        return report;
}

}; // namespace command

}; // namespace sword
