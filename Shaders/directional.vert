#version 450

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aUv;
layout(location = 2) in vec3 aNormal;

layout(set = 0, binding = 0) uniform UniformData
{
    mat4 projections[6];
}
uniformData;

layout(push_constant) uniform PushConstant
{
    mat4 model;
    float intensity;
    uint projectionIndex;
}
pushConstant;

void main()
{
    gl_Position = uniformData.projections[pushConstant.projectionIndex] * pushConstant.model * vec4(aPos, 1.0);
}