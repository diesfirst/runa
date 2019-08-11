#ifndef PIPE_H
#define PIPE_H

#include "context.hpp"

class Pipe
{
public:
	Pipe(const Context&);
	virtual ~Pipe();
	void createGraphicsPipeline();

private:
	const Context& context;
	uint32_t width, height;
	
	vk::ShaderModule createShaderModule(const std::vector<char>& code);
};

#endif /* PIPE_H */
