#version 450

layout(location = 0) out vec4 albedo;

layout(location = 0) in InData
{
    vec2 uv;
    vec3 normal;
    vec3 fragPos;
    flat int lightCount;
}
inData;

float f(float d)
{
    return sqrt(d);
}

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

layout(set = 1, binding = 1) readonly buffer LightStorage
{
    Light lights[];
}
lightStorage;

void main()
{

    vec3 skyColor1 = vec3(0.941, 0.969, 0.992);
    vec3 skyColor2 = vec3(0.051, 0.369, 0.545);
    vec3 color = skyColor2;

    vec3 pos = normalize(inData.fragPos);
    float d = abs(pos.y);

    color = vec3(d);
    color = mix(skyColor1, skyColor2, f(d));

    vec3 sunColor = vec3(1.0, 0.834, 0.573);

    for (int i = 0; i < inData.lightCount; i++)
    {
        Light light = lightStorage.lights[i];

        vec3 lightDirection = normalize(light.direction);
        float t = clamp(distance(pos, lightDirection), 0, 1);
        color += sunColor * pow(abs(1 - t), 4);
    }

    albedo = vec4(color, 1.0);

    // albedo = vec4(0, 0, 0, 1);
}