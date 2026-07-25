#version 450

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aUv;
layout(location = 2) in vec3 aNormal;

layout(set = 0, binding = 0) uniform UniformData
{
    mat4 projections[6];
    vec3 cameraPosition;
}
uniformData;

layout(push_constant) uniform PushConstant
{
    mat4 model;
}
pushConstant;

layout(location = 0) out OutputData
{
    mat4 projections[6];
    vec3 fragPos;
    vec3 cameraPos;
    vec2 uv;
}
Output;

void main()
{
    Output.uv = aUv;
    Output.fragPos = vec3(pushConstant.model * vec4(aPos, 1.0));
    Output.cameraPos = uniformData.cameraPosition;

    for (int i = 0; i < 6; i++)
    {
        Output.projections[i] = uniformData.projections[i];
    }

    gl_Position = pushConstant.model * vec4(aPos, 1.0);
}