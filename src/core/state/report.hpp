#ifndef REPORT_HPP_
#define REPORT_HPP_

#include <string>
#include <iostream>

namespace sword
{

namespace state
{

enum class ReportType : uint8_t
{
    Shader,
    Renderpass,
    Pipeline,
    RenderpassInstance,
    RenderCommand,
    DescriptorSetLayout,
    DescriptorSet,
    Attachment,
    PipelineLayout
};

class Report
{
public:
    virtual void operator()() const = 0;
    virtual const std::string getObjectName() const = 0;
    virtual ReportType getType() const = 0;
    virtual ~Report() = default;
};

class ShaderReport final : public Report
{
public:
    ShaderReport(std::string n, const char* t, int i0, int i1, float f0, float f1) :
        name{n}, type{t}, specint0{i0}, specint1{i1}, specfloat0{f0}, specfloat1{f1} {}
    void operator()() const  override
    {
        std::cout << "================== Shader Report ==================" << std::endl;
        std::cout << "Name:                  " << name << std::endl;
        std::cout << "Type:                  " << type << std::endl;
        std::cout << "Spec Constant Int 0:   " << specint0 << std::endl;
        std::cout << "Spec Constant Int 1:   " << specint1 << std::endl;
        std::cout << "Spec Constant Float 0: " << specfloat0 << std::endl;
        std::cout << "Spec Constant Float 1: " << specfloat1 << std::endl;
    }
    const std::string getObjectName() const override {return name;};
    ReportType getType() const override {return ReportType::Shader;}
    void setSpecFloat(int index, float val)
    {
        if (index == 0) specfloat0 = val;
        if (index == 1) specfloat1 = val;
    }
    void setSpecInt(int index, int val)
    {
        if (index == 0) specint0 = val;
        if (index == 1) specint1 = val;
    }
private:
    const std::string name{"unitilialized"};
    const char* type{"uninilialized"};
    int specint0{0};
    int specint1{0};
    float specfloat0{0};
    float specfloat1{0};
};

}; //state

}; //sword

#endif /* ifndef REPORT_HPP_ */
