#version 450

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec2 aUv;
layout(location = 2) in vec3 aNormal;

layout(set = 1, binding = 0) uniform UniformData
{
    mat4 view;
    mat4 projection;
    mat4 directionalMatrix1;
    mat4 directionalMatrix2;
    mat4 directionalMatrix3;
    mat4 directionalMatrix4;
    vec3 cameraPosition;
    int lightCount;
    vec3 cameraFront;
}
uniformData;

layout(location = 0) out OutData
{
    vec2 uv;
    vec3 normal;
    vec3 fragPos;
    int lightCount;
}
outData;

void main()
{
    outData.uv = aUv;
    outData.normal = aNormal;
    outData.fragPos = aPosition;
    outData.lightCount = uniformData.lightCount;

    mat4 view = mat4(mat3(uniformData.view));
    gl_Position = uniformData.projection * view * vec4(aPosition, 1.0);
}