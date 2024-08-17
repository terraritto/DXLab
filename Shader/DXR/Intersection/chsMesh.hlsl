#include "../../CommonDXR.hlsli"
#include "../BRDF/BRDFData.hlsli"
#include "../BRDF/CalculatePT.hlsli"

struct Vertex {
    float3 Position;
    float3 Normal;
    float3 Tangent;
    float2 UV;
};

// Local Root Signature (for HitGroup)
StructuredBuffer<uint>   indexBuffer : register(t0,space1);
StructuredBuffer<float3> positionBuffer : register(t1,space1);
StructuredBuffer<float3> normalBuffer : register(t2, space1);
StructuredBuffer<float3> tangentBuffer : register(t2, space2);
StructuredBuffer<float2> uvBuffer : register(t3, space1);
ConstantBuffer<EncodeData> gBRDFInfo : register(b0, space1);

Texture2D<float4> diffuseMap : register(t4, space1);
Texture2D<float4> NormalMap : register(t4, space2);

// í∏ì_èÓïÒÇÃíäèo
Vertex GetHitVertex(MyAttribute attrib)
{
    Vertex v = (Vertex)0;
    uint start = PrimitiveIndex() * 3; // Triangle List ÇÃÇΩÇﬂ.

    float3 positions[3], normals[3], tangents[3];
    float2 uvs[3];
    for (int i = 0; i < 3; ++i) {
        uint index = indexBuffer[start + i];
        positions[i] = positionBuffer[index];
        normals[i] = normalBuffer[index];
        tangents[i] = tangentBuffer[index];
        uvs[i] = uvBuffer[index];
    }

    v.Position = CalcHitAttribute3(positions, attrib.barys);
    v.Normal = normalize(CalcHitAttribute3(normals, attrib.barys));
    v.Tangent = normalize(CalcHitAttribute3(tangents, attrib.barys));
    v.UV = CalcHitAttribute2(uvs, attrib.barys);
    return v;
}

[shader("closesthit")]
void ClosestHitMesh(inout Payload payload, MyAttribute attrib) 
{
    float4x3 mtx = ObjectToWorld4x3();
    Vertex vtx = GetHitVertex(attrib); // í∏ì_éÊìæ

    float3 worldPosition = mul(float4(vtx.Position, 1.0f), mtx).xyz;
    
    // Normalç\íz
    float3 N = mul(vtx.Normal, (float3x3)mtx);
    float3 T = mul(vtx.Tangent, (float3x3)mtx);
    float3 B = normalize(cross(N, T));
    float3x3 TangentBasis = float3x3(T, B, N);

    float3 normal = NormalMap.SampleLevel(gSampler, vtx.UV, 0).xyz * 2.0f - 1.0f;
    normal = mul(transpose(TangentBasis), N);

    // Utilç\íz
    UtilData data;
    data.normal = normal;
    data.worldPos = worldPosition;
    data.dir = WorldRayDirection();

    // Decodeç\íz
    BRDFData brdf;
    Decode(gBRDFInfo, brdf);
    float4 diffuse = diffuseMap.SampleLevel(gSampler, vtx.UV, 0);
    brdf.diffuse.xyz *= diffuse.xyz; 
    brdf.alpha *= diffuse.w;

    // åvéZ
    CalculatePT(payload, brdf, data);
}