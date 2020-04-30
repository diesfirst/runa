#ifndef STATE_SHADER_HPP
#define STATE_SHADER_HPP

//imp: state/shader.cpp

#include <state/state.hpp>
#include <types/map.hpp>
#include <command/rendercommands.hpp>
#include <command/shader.hpp>
#include <util/defs.hpp>
#include <filesystem>

namespace sword
{

namespace state
{

namespace shader_dir
{
    constexpr const char* build = SHADER_DIR;
    constexpr const char* src = SWORD"/src/shaders";
    constexpr const char* frag = SWORD"/src/shaders/fragment";
    constexpr const char* vert = SWORD"/src/shaders/vertex";
}

namespace shader
{
    enum class SpecType : uint8_t {integer, floating};
}

using ShaderReports = Reports<ShaderReport>;

class PrintShader : public LeafState
{
public:
    const char* getName() const override { return "PrintShader"; }
    void handleEvent(event::Event*) override;
    PrintShader(StateArgs, Callbacks);
private:
    void onEnterExt() override;
};

class LoadFragShaders : public LeafState
{
public:
    const char* getName() const override { return "load_frag_shaders"; }
    void handleEvent(event::Event*) override;
    LoadFragShaders(StateArgs sa, Callbacks cb);
private:
    CommandPool<command::LoadFragShader>& lfPool;
    static constexpr const char* frag_dir = shader_dir::frag;
    void onEnterExt() override;
};

class LoadVertShaders : public LeafState
{
public:
    const char* getName() const override { return "load_vert_shaders"; }
    void handleEvent(event::Event*) override;
    LoadVertShaders(StateArgs sa, Callbacks cb);
private:
    CommandPool<command::LoadVertShader>& lvPool;
    static constexpr const char* vert_dir = shader_dir::vert;
    void onEnterExt() override;
};

class SetSpec : public LeafState
{
public:
    const char* getName() const override { return "set_spec"; }
    void handleEvent(event::Event*) override;
    SetSpec(StateArgs sa, Callbacks cb, shader::SpecType t, ShaderReports& reports);
private:
    CommandPool<command::SetSpecInt>& ssiPool;
    CommandPool<command::SetSpecFloat>& ssfPool;
    shader::SpecType type;
    ShaderReports& reports;
    void onEnterExt() override;
};

class CompileShader : public LeafState
{
public:
    const char* getName() const override { return "CompileShader"; }
    void handleEvent(event::Event*) override;
    CompileShader(StateArgs, Callbacks);
private:
    void onEnterExt() override;
    CommandPool<command::CompileShader> pool;
};

class ShaderManager final : public BranchState
{
public:
    const char* getName() const override { return "shader_manager"; }
    void handleEvent(event::Event*) override;
    ShaderManager(StateArgs, Callbacks, ReportCallbackFn<ShaderReport>);
private:
    enum class Op : Option {compileShader, printShader, loadFrag, loadVert, setSpecInt, setSpecFloat, printReports};
    enum class SpecType : uint8_t {integer, floating};

    void printReports();

    LoadFragShaders loadFragShaders;
    LoadVertShaders loadVertShaders;
    SetSpec setSpecInt;
    SetSpec setSpecFloat;
    PrintShader printShader;
    CompileShader compileShader;

    ShaderReports shaderReports;
};

}; // namespace state

}; // namespace sword

#endif /* end of include guard: STATE_SHADER_HPP */
