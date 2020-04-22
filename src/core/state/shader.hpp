#ifndef STATE_SHADER_HPP
#define STATE_SHADER_HPP

//imp: state/shader.cpp

#include <state/state.hpp>
#include <types/map.hpp>
#include <command/rendercommands.hpp>

namespace sword
{

namespace state
{

constexpr const char* SHADER_DIR = "/home/michaelb/dev/sword/build/shaders/";

namespace shader
{
    enum class SpecType : uint8_t {integer, floating};
}

using ShaderReports = Reports<ShaderReport>;

class LoadFragShaders : public LeafState
{
public:
    const char* getName() const override { return "load_frag_shaders"; }
    void handleEvent(event::Event*) override;
    virtual ~LoadFragShaders() = default;   
    LoadFragShaders(StateArgs sa, Callbacks cb);
private:
    CommandPool<command::LoadFragShader>& lfPool;
    void onEnterExt() override;
};

class LoadVertShaders : public LeafState
{
public:
    const char* getName() const override { return "load_vert_shaders"; }
    void handleEvent(event::Event*) override;
    virtual ~LoadVertShaders() = default;   
    LoadVertShaders(StateArgs sa, Callbacks cb);
private:
    CommandPool<command::LoadVertShader>& lvPool;
    void onEnterExt() override;
};

class SetSpec : public LeafState
{
public:
    const char* getName() const override { return "set_spec"; }
    void handleEvent(event::Event*) override;
    virtual ~SetSpec() = default;   
    SetSpec(StateArgs sa, Callbacks cb, shader::SpecType t, ShaderReports& reports);
private:
    CommandPool<command::SetSpecInt>& ssiPool;
    CommandPool<command::SetSpecFloat>& ssfPool;
    shader::SpecType type;
    ShaderReports& reports;
    void onEnterExt() override;
};

class ShaderManager final : public BranchState
{
public:
    enum class Op : Option {loadFrag, loadVert, setSpecInt, setSpecFloat, printReports};
    constexpr Option opcast(Op op) {return static_cast<Option>(op);}
    constexpr Op opcast(Option op) {return static_cast<Op>(op);}
    const char* getName() const override { return "shader_manager"; }
    void handleEvent(event::Event*) override;
    virtual ~ShaderManager() = default;
    ShaderManager(StateArgs, Callbacks, ReportCallbackFn<ShaderReport>);
    enum class SpecType : uint8_t {integer, floating};
private:
    void printReports();
    LoadFragShaders loadFragShaders;
    LoadVertShaders loadVertShaders;
    SetSpec setSpecInt;
    SetSpec setSpecFloat;

    ShaderReports shaderReports;
};

}; // namespace state

}; // namespace sword

#endif /* end of include guard: STATE_SHADER_HPP */
