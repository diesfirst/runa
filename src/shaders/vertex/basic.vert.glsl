#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;

layout(binding = 0) uniform UniformBufferObject
{
	mat4 projection;
	mat4 view;
} ubo;

layout(binding = 1) uniform UboDynamic
{
	mat4 model;
} uboDyn;

layout(location = 0) out vec3 fragColor;

mat4 ident = mat4(1.0);

void main() 
{
	gl_Position = ubo.view * uboDyn.model * vec4(inPosition, 0.0, 1.0);
	fragColor = inColor;
}
