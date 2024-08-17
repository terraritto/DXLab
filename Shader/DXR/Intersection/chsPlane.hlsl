#include "../../CommonDXR.hlsli"
#include "../BRDF/BRDFData.hlsli"
#include "../BRDF/CalculateTrace.hlsli"

struct MyIntersectAttribute {
    float3 normal;
};

struct PlaneInfo {
    float3 center;
    float3 normal;
};

// Local Root Signature (for HitGroup)
ConstantBuffer<EncodeData> gBRDFInfo : register(b0, space1);
ConstantBuffer<PlaneInfo> gPlaneInfo : register(b2);

bool IntersectToPlane(float3 center, float3 normal, out float thit) 
{
    float t = dot((center - ObjectRayOrigin()), normal) / dot(ObjectRayDirection(), normal);

    if (t > EPSILON)
    {
        thit = t;
        return true;
    }

    return false;
}


[shader("intersection")]
void IntersectPlane() {
    const float3 center = gPlaneInfo.center;
    const float3 normal = gPlaneInfo.normal;

    float thit = 0.0f;

    if (IntersectToPlane(center, normal, thit)) 
    {    
        // HitéûÇÕNormalÇìnÇ∑ÇÊÇ§Ç…
        MyIntersectAttribute attr;
        attr.normal = normal;
        ReportHit(thit, 0, attr);
    }
}

[shader("closesthit")]
void ClosestHitPlane(inout Payload payload, MyIntersectAttribute attrib) {
    float4x3 mtx = ObjectToWorld4x3();

    float3 worldPosition = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();
    float3 worldNormal = mul(attrib.normal, (float3x3)mtx);

    // Utilç\íz
    UtilData data;
    data.normal = worldNormal;
    data.worldPos = worldPosition;
    data.dir = WorldRayDirection();

    // Decodeç\íz
    BRDFData brdf;
    Decode(gBRDFInfo, brdf);

    // åvéZ
    CalculateTrace(payload, brdf, data);
}
