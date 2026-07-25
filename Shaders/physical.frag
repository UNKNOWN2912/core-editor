#version 450
#extension GL_EXT_nonuniform_qualifier : require

layout(location = 0) out vec4 outputColor;
layout(set = 0, binding = 0) uniform sampler2D albedoTextures[];

layout(set = 2, binding = 0) uniform samplerCube cubeShadowMap[];
layout(set = 2, binding = 0) uniform sampler2DArray layeredShadowMap[];
layout(set = 2, binding = 0) uniform sampler2D shadowMap[];

const int LightType_DirectionalLight = 0;
const int LightType_PointLight = 1;
const int LightType_SpotLight = 2;

const uint UIntMax = 4294967295U;

const float pi = 3.14159265359;

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

struct Light
{
    vec3 position;
    float intensity;

    vec3 color;
    float innerAngle;

    vec3 direction;
    float outerAngle;

    float radius;
    int type;
    int shadowMapIndex;
    int castShadow;
};

layout(location = 0) in InputData
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
    flat int lightCount;
    flat float time;
}
Input;

layout(set = 1, binding = 1) readonly buffer LightStorage
{
    Light lights[];
}
lightStorage;

///////////////////////////////////////////////////////////////////////////////////
//////////////////////////////        END BRDF         ////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

vec3 unlitFunction(vec3 normal, vec3 viewDirection, vec3 objectColor)
{
    return objectColor * vec3(0.05, 0.05, 0.05);
}

float spotLightDensityFunction(float intensity, float innerAngle, float outerAngle, float distance, float angle)
{
    float t = clamp((angle - outerAngle) / (innerAngle - outerAngle), 0.0, 1.0);

    float clight = intensity * pow(1 / max(distance, 0.01), 2) * ((t));
    return clight;
}

float LinearizeDepth(float sampledDepth, float zNear, float zFar)
{
    return (zNear * zFar) / (zFar + sampledDepth * (zNear - zFar));
}

vec2 random2(vec2 st)
{
    st = vec2(
        dot(st, vec2(127.1, 311.7)),
        dot(st, vec2(269.5, 183.3)));
    return fract(sin(st) * 43758.5453123);
}

vec4 cascadeColor[4] = vec4[](
    vec4(1, 0, 0, 1),
    vec4(0, 1, 0, 1),
    vec4(0, 0, 1, 1),
    vec4(1, 1, 0, 1));

vec3 LambertDiffuse(vec3 lightDirection, vec3 viewDirection, vec3 objectColor)
{
    return objectColor / pi;
}

vec3 SchlickFresnel(vec3 F0, float NdotH)
{
    return F0 + (1 - F0) * pow(1 - NdotH, 5);
}

float Distribution(float NdotH, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;

    float NdotH2 = NdotH * NdotH;

    float nominator = a2;
    float denominator = 1 + NdotH2 * (a2 - 1);
    denominator = pi * denominator * denominator;

    return nominator / max(denominator, 0.000001);
}

float G1(float NdotX, float roughness)
{
    float a = roughness * roughness;

    float nominator = 2 * NdotX;
    float denominator = NdotX * (2 - a) + a;

    return nominator / max(denominator, 0.000001);
}

float GeometryGGX(float NdotV, float NdotL, float roughness)
{
    return G1(NdotV, roughness) * G1(NdotL, roughness);
}

vec3 BRDF(vec3 normal, vec3 lightDirection, vec3 viewDirection, vec3 objectColor, float roughness, float metallic)
{

    vec3 halfWay = normalize(lightDirection + viewDirection);

    float NdotH = max(dot(normal, halfWay), 0.0);
    float NdotV = max(dot(normal, viewDirection), 0.0);
    float NdotL = max(dot(normal, lightDirection), 0.0);

    vec3 fdiffuse = LambertDiffuse(lightDirection, viewDirection, objectColor);

    vec3 F0 = mix(vec3(0.04f), objectColor, metallic);
    vec3 F = SchlickFresnel(F0, NdotH);
    float D = Distribution(NdotH, roughness);
    float G = GeometryGGX(NdotV, NdotL, roughness);

    vec3 nominator = (F * G * D);
    float denominator = 4 * NdotV * NdotL;
    vec3 fspecular = nominator / max(denominator, 0.000001);

    vec3 Ks = F;
    vec3 Kd = (1 - Ks) * (1 - metallic);

    return Kd * fdiffuse + fspecular;
}

vec3 IncomingLight(vec3 lightColor, float intensity)
{
    return lightColor * intensity;
}

void main()
{

    vec3 objectColor = pushConstant.color.rgb;
    float a = pushConstant.color.a;
    if (pushConstant.albedoIndex != UIntMax)
    {
        objectColor = texture(albedoTextures[pushConstant.albedoIndex], Input.uv).rgb;
        a = texture(albedoTextures[pushConstant.albedoIndex], Input.uv).a;
    }

    if (a < 0.1)
        discard;

    float roughness = pushConstant.roughness;
    float metallic = pushConstant.metallic;
    if (pushConstant.roughnessIndex != UIntMax)
    {
        roughness = texture(albedoTextures[pushConstant.roughnessIndex], Input.uv).g;
    }
    if (pushConstant.metallicIndex != UIntMax)
    {
        metallic = texture(albedoTextures[pushConstant.metallicIndex], Input.uv).b;
    }

    // roughness = pushConstant.roughness;
    // metallic = pushConstant.metallic;

    vec3 normal = normalize(Input.normal);

    vec3 viewDirection = normalize(Input.cameraPosition - Input.fragPos);
    vec3 unlit = unlitFunction(normal, viewDirection, objectColor);

    vec3 result = vec3(0);
    int cascadeIndex = 0;

    for (int i = 0; i < Input.lightCount; i++)
    {
        Light light = lightStorage.lights[i];

        vec3 direction = light.direction;
        vec3 position = light.position;
        vec3 color = light.color;
        int type = light.type;
        float intensity = light.intensity;

        objectColor = objectColor * light.color;

        if (type == LightType_SpotLight)
        {
            float outerAngle = light.outerAngle;
            float innerAngle = light.innerAngle;
            vec3 lightDirection = normalize(position - Input.fragPos);
            float angle = dot(direction, -lightDirection);
            float radius = distance(position, Input.fragPos);

            // float shadow = 1.f;
            // vec3 fragToLight = Input.fragPos - position;
            // float currentDepth = length(fragToLight);
            // vec3 uvs = normalize(fragToLight);
            // uvs.z *= -1;
            // float bias = max(0.05 * (1.0 - dot(normal, lightDirection)), 0.005);

            // float closestDepth = texture(shadowMap, uvs).r * 1000.f;
            // closestDepth = LinearizeDepth(closestDepth, 0.01f, 1000.f);

            // if (currentDepth > closestDepth)
            // {
            //     shadow = 0.f;
            // }

            float diffuse = max(dot(lightDirection, normal), 0.0);
            float density = spotLightDensityFunction(intensity, innerAngle, outerAngle, radius, angle);

            // vec3 lit = litFunction(lightDirection, normal, viewDirection, objectColor, color, roughness, metallic);
            // result += density * lit;
        }
        else if (type == LightType_PointLight)
        {
            vec3 lightDirection = normalize(position - Input.fragPos);
            float radius = distance(position, Input.fragPos);

            float shadow = 0.f;
            vec3 fragToLight = Input.fragPos - position;
            float currentDepth = length(fragToLight);
            vec3 uvs = (fragToLight);
            float bias = max(0.005 * (1.0 - dot(normal, lightDirection)), 0.0005);

            float closestDepth = texture(cubeShadowMap[lightStorage.lights[i].shadowMapIndex], uvs).r * 100.f;

            if (currentDepth > closestDepth + bias)
            {
                shadow = 1.f;
            }

            result += (BRDF(normal, lightDirection, viewDirection, objectColor, roughness, metallic) * IncomingLight(light.color, light.intensity) * max(dot(normal, lightDirection), 0.0) * (1 - shadow)) / (radius * radius);
        }
        else if (type == LightType_DirectionalLight)
        {
            vec3 fragPos[4];
            fragPos[0] = Input.directionalPos1.xyz / Input.directionalPos1.w;
            fragPos[1] = Input.directionalPos2.xyz / Input.directionalPos2.w;
            fragPos[2] = Input.directionalPos3.xyz / Input.directionalPos3.w;
            fragPos[3] = Input.directionalPos4.xyz / Input.directionalPos4.w;

            if (-1.f < fragPos[3].x && fragPos[3].x < 1.f && -1.f < fragPos[3].y && fragPos[3].y < 1.f && 0.f < fragPos[3].z && fragPos[3].z < 1.f)
            {
                cascadeIndex = 3;
            }
            if (-1.f < fragPos[2].x && fragPos[2].x < 1.f && -1.f < fragPos[2].y && fragPos[2].y < 1.f && 0.f < fragPos[2].z && fragPos[2].z < 1.f)
            {
                cascadeIndex = 2;
            }
            if (-1.f < fragPos[1].x && fragPos[1].x < 1.f && -1.f < fragPos[1].y && fragPos[1].y < 1.f && 0.f < fragPos[1].z && fragPos[1].z < 1.f)
            {
                cascadeIndex = 1;
            }
            if (-1.f < fragPos[0].x && fragPos[0].x < 1.f && -1.f < fragPos[0].y && fragPos[0].y < 1.f && 0.f < fragPos[0].z && fragPos[0].z < 1.f)
            {
                cascadeIndex = 0;
            }

            fragPos[cascadeIndex].xy = (fragPos[cascadeIndex].xy + 1.f) / 2.f;
            vec2 uv = fragPos[cascadeIndex].xy;

            vec3 lightDirection = light.direction;

            float shadow = 0.f;
            float currentDepth = fragPos[cascadeIndex].z;

            float bias = max(0.005 * (1.0 - dot(normal, lightDirection)), 0.0005);

            if (cascadeIndex <= 2)
            {
                float pixelSize = 1 / 1024.f;
                int size = 2;

                for (int x = -size; x <= size; x++)
                {
                    for (int y = -size; y <= size; y++)
                    {
                        vec2 offset = uv + vec2(x * pixelSize, y * pixelSize);
                        offset = offset + (random2(Input.fragPos.xy) * pixelSize);
                        offset = clamp(offset, vec2(0), vec2(0.998));

                        float closestDepth = texture(layeredShadowMap[light.shadowMapIndex], vec3(offset, cascadeIndex)).r;
                        if (currentDepth > closestDepth + bias)
                        {
                            shadow += 1.f;
                        }
                    }
                }

                shadow /= pow(((2 * size) + 1), 2);
            }
            else
            {
                float closestDepth = texture(layeredShadowMap[light.shadowMapIndex], vec3(uv, cascadeIndex)).r;
                if (currentDepth > closestDepth + bias)
                {
                    shadow += 1.f;
                }
            }

            result += (BRDF(normal, lightDirection, viewDirection, objectColor, roughness, metallic) * IncomingLight(light.color, light.intensity) * max(dot(normal, lightDirection), 0.0) * (1 - shadow));
        }
    }

    vec3 shaded = unlit + result;

    outputColor = vec4(shaded, a);

    if (pushConstant.inputInt == 1)
    {
        outputColor = vec4(normal, 1.0);
    }
}