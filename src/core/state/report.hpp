#ifndef REPORT_HPP_
#define REPORT_HPP_

#include <string>

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
};

}; //state

}; //sword

#endif /* ifndef REPORT_HPP_ */
