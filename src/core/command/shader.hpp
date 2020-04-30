#ifndef COMMAND_SHADER_HPP
#define COMMAND_SHADER_HPP

//imp: "shader.cpp"

#include "command.hpp"
#include <string>
#include <util/defs.hpp>
#include <filesystem>
#include <shaderc/shaderc.hpp>
#include <util/enum.hpp>

namespace sword
{

namespace state{ class ShaderReport; }

namespace command
{

class CompileShader : public Command
{
public:
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
};

}; // namespace command

}; // namespace sword

#endif /* end of include guard: COMMAND_SHADER_HPP */
