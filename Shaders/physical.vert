#version 450

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aUv;
layout(location = 2) in vec3 aNormal;
layout(location = 3) in vec3 aTangent;
layout(location = 4) in vec3 aBitangent;

layout(location = 0) out OutputData
{
    vec2 uv;
    vec3 normal;
    vec3 fragPos;
    vec4 directionalPos1;
    vec4 directionalPos2;
    vec4 directionalPos3;
    vec4 directionalPos4;
    vec3 cameraPosition;
    vec3 cameraFront;
    int lightCount;
    float time;
}
Output;

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
    float time;
}
uniformData;

layout(push_constant) uniform PushConstant
{
    mat4 model;
    uint albedoIndex;
    uint specularIndex;
    uint roughnessIndex;
    uint metallicIndex;
}
pushConstant;

void main()
{

    vec3 currentPos = vec3(pushConstant.model * vec4(aPos, 1.0));

    Output.normal = mat3(transpose(inverse(pushConstant.model))) * aNormal;
    Output.uv = aUv;
    Output.fragPos = currentPos;
    Output.directionalPos1 = uniformData.directionalMatrix1 * pushConstant.model * vec4(aPos, 1.0);
    Output.directionalPos2 = uniformData.directionalMatrix2 * pushConstant.model * vec4(aPos, 1.0);
    Output.directionalPos3 = uniformData.directionalMatrix3 * pushConstant.model * vec4(aPos, 1.0);
    Output.directionalPos4 = uniformData.directionalMatrix4 * pushConstant.model * vec4(aPos, 1.0);
    Output.cameraPosition = uniformData.cameraPosition;
    Output.cameraFront = uniformData.cameraFront;
    Output.lightCount = uniformData.lightCount;
    Output.time = uniformData.time;

    gl_Position = uniformData.projection * uniformData.view * pushConstant.model * vec4(aPos, 1.0);
    // gl_Position = uniformData.directionalMatrix * pushConstant.model * vec4(aPos, 1.0);
}