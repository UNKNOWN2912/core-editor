#version 450

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec2 aUv;

layout(location = 2) in mat4 aInstanceModel;
layout(location = 6) in vec4 aInstanceForgroundColor;
layout(location = 7) in vec4 aInstanceBackgroundColor;
layout(location = 8) in uint aStartIndex;
layout(location = 9) in uint aCount;

layout(set = 0, binding = 0) uniform UniformData
{
    mat4 view;
    mat4 projection;
}
uniformData;

layout(location = 0) out OutputData
{
    vec2 uv;
    vec4 forgroundColor;
    vec4 backgroundColor;
    flat uint startIndex;
    flat uint count;
}
outdata;

void main()
{
    vec3 currentPos = vec3(aInstanceModel * vec4(aPosition, 1.0));
    outdata.uv = aUv;
    outdata.forgroundColor = aInstanceForgroundColor;
    outdata.backgroundColor = aInstanceBackgroundColor;
    outdata.startIndex = aStartIndex;
    outdata.count = aCount;

    mat4 inv = inverse(aInstanceModel);

    gl_Position = uniformData.projection * uniformData.view * aInstanceModel * vec4(aPosition, 1.0);
}