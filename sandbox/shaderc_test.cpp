#include <shaderc/shaderc.hpp>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vulkan/vulkan.hpp>
#include <vector>
#include <filesystem>

int main(int argc, const char *argv[])
{
    auto path = std::filesystem::path("/home/michaelb/dev/sword/src/shaders/fragment/red.frag");
    auto ext = path.extension();
    std::cout << "ext: " << ext << '\n';
    auto f = std::ifstream(path);
    std::stringstream stream;
    if (f.is_open())
    {
        stream << f.rdbuf();
    }
    std::string src = stream.str();
    shaderc::Compiler compiler;
    auto res = compiler.CompileGlslToSpv(src, shaderc_shader_kind::shaderc_glsl_default_fragment_shader, "hmm");
    auto status = res.GetCompilationStatus();
    if (status == 0)
        std::cout << "Success" << '\n';
    else 
        return 1;
    
    std::vector<uint32_t> code;
    for (const auto& i : res) 
    {
        code.push_back(i);
    }

    std::cout << code.size() << '\n';

    return 0;
}
