#version 450

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aUv;
layout(location = 2) in vec3 aNormal;

layout(set = 0, binding = 0) uniform UniformData
{
    mat4 projections[6];
}
uniformData;

layout(location = 0) out OutputData
{
    mat4 projections[6];
    vec2 uv;
}
Output;

layout(push_constant) uniform PushConstant
{
    mat4 model;
}
pushConstant;

void main()
{
    for (int i = 0; i < 6; i++)
    {
        Output.projections[i] = uniformData.projections[i];
    }
    Output.uv = aUv;
    gl_Position = pushConstant.model * vec4(aPos, 1.0);
}