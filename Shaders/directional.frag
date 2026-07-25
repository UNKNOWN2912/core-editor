#version 450
#extension GL_EXT_nonuniform_qualifier : require

layout(set = 1, binding = 0) uniform sampler2D albedoTextures[];

const uint UIntMax = 4294967295U;

layout(set = 1, binding = 0) uniform sampler2D albedoTextures[];

layout(location = 0) in InputData
{
    vec2 uv;
}
Input;

layout(push_constant) uniform PushConstant
{
    mat4 model;
    uint albedoIndex;
    uint normalIndex;
    uint roughnessIndex;
    uint metallicIndex;
    uint inputInt;
    float roughness;
    float metallic;
    float indexOfRefraction;
    vec4 color;
}
pushConstant;

void main()
{
    float alpha = pushConstant.color.a;
    if (pushConstant.albedoIndex != UIntMax)
    {
        alpha = texture(albedoTextures[pushConstant.albedoIndex], Input.uv).a;
    }

    if (alpha < 0.1)
    {
        discard;
    }
}