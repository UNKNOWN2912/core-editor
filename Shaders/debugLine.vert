#version 450

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 color;

layout(location = 0) out OutputData
{
    vec3 color;
}
Output;

layout(set = 0, binding = 0) uniform UniformData
{
    mat4 view;
    mat4 projection;
}
uniformData;

void main()
{
    Output.color = color;
    gl_Position = uniformData.projection * uniformData.view * vec4(aPos, 1.0);
}