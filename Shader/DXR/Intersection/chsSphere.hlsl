#include "../../CommonDXR.hlsli"
#include "../BRDF/BRDFData.hlsli"
#include "../BRDF/CalculatePT.hlsli"

struct MyIntersectAttribute {
    float3 normal;
};

struct SphereInfo {
    float3 center;
    float  radius;
};

// Local Root Signature (for HitGroup)
ConstantBuffer<EncodeData> gBRDFInfo : register(b0, space1);
ConstantBuffer<SphereInfo> gSphereInfo : register(b2);

bool IntersectToAABBDetail(float3 aabb[2], out float tmin, out float tmax)
{
    float3 origin = ObjectRayOrigin();
    float3 raydir = ObjectRayDirection();
    float3 tmin3, tmax3;
    int3 sign3 = raydir > 0;
    float3 invRay = 1 / raydir;

    tmin3.x = (aabb[1 - sign3.x].x - origin.x);
    tmax3.x = (aabb[sign3.x].x - origin.x);

    tmin3.y = (aabb[1 - sign3.y].y - origin.y);
    tmax3.y = (aabb[sign3.y].y - origin.y);

    tmin3.z = (aabb[1 - sign3.z].z - origin.z);
    tmax3.z = (aabb[sign3.z].z - origin.z);

    tmin3 *= invRay;
    tmax3 *= invRay;

    tmin = max(max(tmin3.x, tmin3.y), tmin3.z);
    tmax = min(min(tmax3.x, tmax3.y), tmax3.z);

    // åì_ÇÕåªç›ÇÃÉåÉCÇÃîÕàÕì‡Ç≈Ç†ÇÈÇ©.
    return tmax > tmin && tmax >= RayTMin() && tmin <= RayTCurrent();
}

bool IntersectToAABB(float3 center, float3 aabbMin, float3 aabbMax, out float t, out float3 normal)
{
    float tmin, tmax;
    float3 aabb[2] = { aabbMin + center, aabbMax + center};
    if (IntersectToAABBDetail(aabb, tmin, tmax)) {
        t = tmin >= RayTMin() ? tmin : tmax;

        float3 hitPosition = ObjectRayOrigin() + t * ObjectRayDirection();
        float3 distanceToBounds[2] = {
            abs(aabb[0] - hitPosition),
            abs(aabb[1] - hitPosition)
        };
        const float eps = 0.0001;
        if (distanceToBounds[0].x < eps) normal = float3(-1, 0, 0);
        else if (distanceToBounds[0].y < eps) normal = float3(0, -1, 0);
        else if (distanceToBounds[0].z < eps) normal = float3(0, 0, -1);
        else if (distanceToBounds[1].x < eps) normal = float3(1, 0, 0);
        else if (distanceToBounds[1].y < eps) normal = float3(0, 1, 0);
        else if (distanceToBounds[1].z < eps) normal = float3(0, 0, 1);
        return true;
    }
    return false;
}

bool IntersectToSphere(float3 center, float radius, out float thit, out float3 normal) {
    float t;
    float3 m = ObjectRayOrigin() - center; // o-c
    float3 d = ObjectRayDirection(); // d
    float a = dot(d, d); // a = d * d
    float b = 2.0f * dot(m, d); // b = 2(o-c)*d
    float c = dot(m, m) - radius * radius; // (o-c)*(o-c) - r^2
    float disc = b * b - 4.0f * a * c; // discriminant

    if (disc < 0.0f) 
    {
        return false;
    }
    else
    {
        float e = sqrt(disc);
        float denom = 2.0f * a;
        t = (-b - e) / denom; // smaller root

        if (t > EPSILON)
        {
            thit = t;
            normal = (m + t * d) / radius;
            return true;
        }

        t = (-b + e) / denom; // larger root
        if (t > EPSILON)
        {
            thit = t;
            normal = (m + t * d) / radius;
            return true;
        }
    }
    return false;
}


[shader("intersection")]
void IntersectSphere() {
    const float3 center = gSphereInfo.center;
    const float  radius = gSphereInfo.radius;

    float thit = 0.0f;
    float3 normal = 0.0f;

    if (IntersectToSphere(center, radius, thit, normal)) 
    {    
        MyIntersectAttribute attr;
        attr.normal = normal;
        ReportHit(thit, 0, attr);
    }
}

[shader("closesthit")]
void ClosestHitSphere(inout Payload payload, MyIntersectAttribute attrib) {
    float4x3 mtx = ObjectToWorld4x3();

    float3 worldPosition = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();
    float3 worldNormal = normalize(mul(attrib.normal, (float3x3)mtx));
    float3 toEyeDirection = gSceneParam.eyePosition.xyz - worldPosition.xyz;

    // Utilç\íz
    UtilData data;
    data.normal = worldNormal;
    data.worldPos = worldPosition;
    data.dir = WorldRayDirection();

    // Decodeç\íz
    BRDFData brdf;
    Decode(gBRDFInfo, brdf);

    CalculatePT(payload, brdf, data);
}
