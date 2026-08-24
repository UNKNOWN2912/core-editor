#version 450

layout(location = 0) in vec3 aPos;

layout(set = 0, binding = 0) uniform UniformData
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
    gl_Position = uniformData.projection * uniformData.view * pushConstant.model * vec4(aPos, 1.0);
}