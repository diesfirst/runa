layout(set = 0, binding = 0) uniform uboBuf 
{ 
    float time;
	float mouseX;
	float mouseY;
    float r;
    float g;
    float b;
    float a;
    float brushSize;
    float brushX;
    float brushY;
    int sampleId;
    int null2; //for padding the xform
    mat4 xform;
} ubo;
