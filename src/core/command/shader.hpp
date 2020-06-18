#ifndef COMMAND_SHADER_HPP
#define COMMAND_SHADER_HPP

//imp: "shader.cpp"

#include "command.hpp"
#include "libshaderc_util/file_finder.h"
#include <string>
#include <util/defs.hpp>
#include <filesystem>
#include <shaderc/shaderc.hpp>
#include <util/enum.hpp>
#include <file_includer.h>

namespace sword
{

namespace state{ class ShaderReport; }

namespace command
{

class CompileShader : public Command
{
public:
    CompileShader();
    void execute(Application*) override;
    const char* getName() const override {return "CompileShader";};
    void set(const std::string_view path, const std::string_view name, state::ShaderReport* report = nullptr);
    state::Report* makeReport() const override;
private:
    inline static const shaderc::Compiler compiler{}; // may not be threadsafe 
    std::string name;
    ShaderType type;
    shaderc_shader_kind kind;
    std::filesystem::path src_path{SHADER_SRC};
    state::ShaderReport* report{nullptr};
    shaderc::CompileOptions compileOptions;
    shaderc_util::FileFinder file_finder;
};

class CompileShaderCode : public Command
{
public:
    void execute(Application*) override;
    const char* getName() const override {return "CompileShaderCode";};
    void set(const std::string_view code, const std::string_view name, ShaderType type, state::ShaderReport* report = nullptr)
    {
        this->name = name;
        this->glslCode= code;
        this->type = type;
    }
    state::Report* makeReport() const override;
private:
    inline static const shaderc::Compiler compiler{}; // may not be threadsafe 
    std::string name;
    std::string glslCode;
    ShaderType type;
    shaderc_shader_kind kind;
    state::ShaderReport* report{nullptr};
};


}; // namespace command

}; // namespace sword

#endif /* end of include guard: COMMAND_SHADER_HPP */
