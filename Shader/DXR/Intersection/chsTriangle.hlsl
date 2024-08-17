#include "../../CommonDXR.hlsli"
#include "../BRDF/BRDFData.hlsli"
#include "../BRDF/CalculateTrace.hlsli"

struct MyIntersectAttribute {
    float3 normal;
};

struct TriangleInfo {
    float3 vert[3];
};

// Local Root Signature (for HitGroup)
ConstantBuffer<EncodeData> gBRDFInfo : register(b0, space1);
ConstantBuffer<TriangleInfo> gTriangleInfo : register(b2);

bool IntersectToTriangle(float3 vert[3], out float thit, out float3 normal) {
    float3 o = ObjectRayOrigin(); // origin
    float3 dir = ObjectRayDirection(); // dir

    float a = vert[0].x - vert[1].x, b = vert[0].x - vert[2].x;
    float c = dir.x, d = vert[0].x - o.x;
    float e = vert[0].y - vert[1].y, f = vert[0].y - vert[2].y;
    float g = dir.y, h = vert[0].y - o.y;
    float i = vert[0].z - vert[1].z, j = vert[0].z - vert[2].z;
    float k = dir.z, l = vert[0].z - o.z;

    float m = f * k - g * j, n = h * k - g * l;
    float p = f * l - h * j, q = g * i - e * k;
    float s = e * j - f * i;

    float invDenom = 1.0 / (a * m + b * q + c * s);

    float e1 = d * m - b * n - c * p;
    float beta = e1 * invDenom;

    // first check
    if (beta < 0.0)
    {
        return false;
    }

    float r = e * l - h * i;
    float e2 = a * n + d * q + c * r;
    float gamma = e2 * invDenom;

    // second check
    if (gamma < 0.0)
    {
        return false;
    }

    // third check
    if (beta + gamma > 1.0)
    {
        return false;
    }

    float e3 = a * p - b * r + d * s;
    float t = e3 * invDenom;

    // final check
    if (t < EPSILON)
    {
        return false;
    }

    //hit
    thit = t;
    normal = cross(vert[1]-vert[0], vert[2]-vert[0]);

    return true;
}


[shader("intersection")]
void IntersectTriangle() {
    float thit = 0.0f;
    float3 normal = 0.0f;

    if (IntersectToTriangle(gTriangleInfo.vert, thit, normal)) 
    {    
        // HitŽž‚ÍNormal‚ð“n‚·‚æ‚¤‚É
        MyIntersectAttribute attr;
        attr.normal = normal;
        ReportHit(thit, 0, attr);
    }
}

[shader("closesthit")]
void ClosestHitTriangle(inout Payload payload, MyIntersectAttribute attrib) {
    float4x3 mtx = ObjectToWorld4x3();

    float3 worldPosition = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();
    float3 worldNormal = normalize(mul(attrib.normal, (float3x3)mtx));

    // Util\’z
    UtilData data;
    data.normal = worldNormal;
    data.worldPos = worldPosition;
    data.dir = WorldRayDirection();

    // Decode\’z
    BRDFData brdf;
    Decode(gBRDFInfo, brdf);
    
    CalculateTrace(payload, brdf, data);
}
