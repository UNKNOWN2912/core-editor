#version 450

struct BezierCurve
{
    vec2 start;
    vec2 control;
    vec2 end;
};

layout(set = 1, binding = 0) readonly buffer BezierStorage
{
    BezierCurve curves[];
}
bezier;

layout(location = 0) out vec4 outcolor;

layout(location = 0) in InputData
{
    vec2 uv;
    vec4 forgroundColor;
    vec4 backgroundColor;
    flat uint startIndex;
    flat uint count;
}
indata;

vec2 Bezier(vec2 A, vec2 B, vec2 C, float t)
{
    float a = (1 - t) * (1 - t);
    float b = 2 * t * (1 - t);
    float c = t * t;

    vec2 res = (a * A) + (b * B) + (c * C);

    return res;
}

struct RootValue
{
    float root0;
    float root1;
};

RootValue BezierLineIntersection(float yDistance, vec2 p0, vec2 p1, vec2 p2)
{
    float A = p0.y + (-2 * p1.y) + p2.y;
    float B = 2.0 * (p1.y - p0.y);
    float C = p0.y - yDistance;

    float B2 = B * B;

    float root0 = (-B + sqrt(B2 - (4 * A * C))) / (2 * A);
    float root1 = (-B - sqrt(B2 - (4 * A * C))) / (2 * A);

    RootValue value;
    value.root0 = root0;
    value.root1 = root1;

    return value;
}

void main()
{
    uint start = indata.startIndex;
    uint count = indata.count;

    vec2 uv = (indata.uv);

    float d = 3.4e32;

    outcolor = vec4(1);

    uint increments = 10;

    uint hitCount = 0;
    for (uint i = start; i < start + count; i++)
    {
        BezierCurve curve = bezier.curves[i];

        vec2 startPoint = (curve.start);
        vec2 controlPoint = (curve.control + vec2(0.0001));
        vec2 endPoint = (curve.end);

        RootValue rootValue = BezierLineIntersection(uv.y, startPoint, controlPoint, endPoint);

        if (0.0 < rootValue.root0 && rootValue.root0 < 1.0)
        {
            vec2 i0Point = Bezier(startPoint, controlPoint, endPoint, rootValue.root0);
            if (uv.x < i0Point.x)
                hitCount++;
        }
        if (0.0 < rootValue.root1 && rootValue.root1 < 1.0)
        {
            vec2 i1Point = Bezier(startPoint, controlPoint, endPoint, rootValue.root1);
            if (uv.x < i1Point.x)
                hitCount++;
        }
    }

    float p = smoothstep(0.0045, 0.005, abs(d));

    bool even = (hitCount % 2) != 0;
    if (!even)
        discard;

    vec4 a = indata.forgroundColor;
    outcolor = mix(vec4(0), a, float(even));
}