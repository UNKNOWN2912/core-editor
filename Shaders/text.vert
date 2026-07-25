#version 450

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec2 aUv;

layout(location = 2) in uint aInstanceCharaterTextureId;
layout(location = 3) in mat4 aInstanceModel;
layout(location = 7) in vec4 aInstanceColor;

layout(set = 1, binding = 0) uniform UniformData
{
    mat4 view;
    mat4 projection;
}
uniformData;

layout(location = 0) out OutputData
{
    vec2 uv;
    flat uint charaterId;
    vec4 color;
}
outdata;

void main()
{
    vec3 currentPos = vec3(aInstanceModel * vec4(aPosition, 1.0));
    outdata.charaterId = aInstanceCharaterTextureId;
    outdata.uv = aUv;
    outdata.color = aInstanceColor;

    gl_Position = uniformData.projection * uniformData.view * aInstanceModel * vec4(aPosition, 1.0);
}