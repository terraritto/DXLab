#ifndef __COMMON_DXR__
#define __COMMON_DXR__

#include "Constant.hlsli"
#include "Light.hlsli"

struct SceneCB {
    matrix mtxView;       // ビュー行列.
    matrix mtxProj;       // プロジェクション行列.
    matrix mtxViewInv;    // ビュー逆行列.
    matrix mtxProjInv;    // プロジェクション逆行列.
    float4 ambientColor;   // 環境光.
    float4 eyePosition;    // 視点.
    uint lightNum; // ライトの数
    uint spp; //サンプル数
};

struct Payload {
    float3 color;
    float3 weight;
    int    recursive;
};
struct ShadowPayload {
    bool isHit;
};

struct MyAttribute {
    float2 barys;
};

// Global Root Signature
RaytracingAccelerationStructure gRtScene : register(t0);
ConstantBuffer<SceneCB> gSceneParam : register(b0);
ConstantBuffer<LightData> gLightList : register(b1);
ConstantBuffer<SphereAreaLightData> gAreaLightList : register(b1, space1);
SamplerState gSampler: register(s0);
Texture2D<float4> gHDRBackground : register(t1);
StructuredBuffer<float2> gRandomTable : register(t2);
RWByteAddressBuffer	gRandomSeed : register(u1);

// Common Function
// *--- Sky Sphere関係 Begin ---*
// CubeMapからの参照
/*
float4 GetSkyBoxColorFromCubeMap()
{
    return gBackground.SampleLevel(gSampler, WorldRayDirection(), 0.0);
}
*/

// HDRからの参照
float4 GetSkyBoxColorFromHdr()
{
    float3 worldRay = WorldRayDirection();
    float theta = acos(worldRay.y);
    float phi = atan2(worldRay.x, worldRay.z);
    phi = phi < 0.0f ? phi + PI2 : phi;

    float2 uv;
    uv.x = phi / PI2;
    uv.y = theta / PI;

    return gHDRBackground.SampleLevel(gSampler, uv, 0.0);
}
// *--- Sky Sphere関係 End ---*

// *--- Noise関係 Begin ---*
// 単純なNoise,GLSLでよくある奴
float GetRandomNumber(float2 uv, int Seed)
{
    return frac(sin(dot(uv.xy, float2(12.9898, 78.233)) + Seed) * 43758.5453);
}

// 乱数をテーブルから取得
float2 GetRandomFor2D()
{
    uint adr;
    gRandomSeed.InterlockedAdd(0, 1, adr);
    float2 x1 = gRandomTable[(adr) % 65536];

    gRandomSeed.InterlockedAdd(0, 1, adr);
    float2 x2 = gRandomTable[(adr) % 65536];

    return float2(GetRandomNumber(x1, gSceneParam.spp),GetRandomNumber(x2, gSceneParam.spp));
}

// PCG実装
uint4 SetSeed(uint2 pixelCoords)
{ 
    uint adr;
    gRandomSeed.InterlockedAdd(0, 1, adr);
    return uint4(pixelCoords.xy, adr, 0); 
}

uint4 PCG(uint4 v)
{
    v = v * 1664525u + 101390422u;
 
    v.x += v.y * v.w;
    v.y += v.z * v.x;
    v.z += v.x * v.y;
    v.w += v.y * v.z;
 
    v = v ^ (v >> 16u);
    v.x += v.y * v.w;
    v.y += v.z * v.x;
    v.z += v.x * v.y;
    v.w += v.y * v.z;
 
    return v;
}

float ToFloat(uint x)
{ 
    return asfloat(0x3f800000 | (x >> 9)) - 1.0f; 
}

float Random(inout uint4 seed)
{
    seed.w++;
    return ToFloat(PCG(seed).x);
}

float3 CosineSampling()
{
    float2 rand2d = GetRandomFor2D();

    float phi = PI2 * rand2d.y;
    float theta = 0.5 * acos(rand2d.x);

    float cos_theta = cos(theta);
    float sin_theta = sin(theta);
    
    return float3(sin_theta * cos(phi), cos_theta, sin(phi) * sin_theta);
}

// todo: 3Dに変換するコードを書く
float3 SampleHemisphere()
{
    uint2 launchIndex = DispatchRaysIndex().xy;
    uint4 seed = SetSeed(launchIndex);
    float2 sample2d = float2(Random(seed), Random(seed));

    float cos_phi = cos(PI2 * sample2d.x);
    float sin_phi = sin(PI2 * sample2d.x);
    
    float theta = acos(sample2d.y);
    float cos_theta = cos(PI * theta);
    float sin_theta = sin(PI * theta);

    return float3(sin_theta * cos_phi, sin_theta * sin_phi, cos_theta);
}

// *--- Noise関係 End ---*

// *--- 座標変換関係 Start ---*
void OrthonormalBasis(out float3 u, out float3 v, inout float3 w)
{
    // wにnormalを設定済み
    if(abs(w.x) > EPSILON)
    {
        u = normalize(cross(float3(0.0, 1.0, 0.0), w));
    }
    else
    {
        u = normalize(cross(float3(1.0, 0.0, 0.0), w));
    }

    v = cross(w, u);
}
// *--- 座標変換関係 End ---*

inline float3 CalcBarycentrics(float2 barys)
{
    return float3(
        1.0 - barys.x - barys.y,
        barys.x,
        barys.y);
}

inline float2 CalcHitAttribute2(float2 vertexAttribute[3], float2 barycentrics)
{
    float2 ret;
    float3 factor = CalcBarycentrics(barycentrics);
    ret = factor.x * vertexAttribute[0];
    ret += factor.y * (vertexAttribute[1]);
    ret += factor.z * (vertexAttribute[2]);
    return ret;
}

float3 CalcHitAttribute3(float3 vertexAttribute[3], float2 barycentrics)
{
    float3 ret;
    float3 factor = CalcBarycentrics(barycentrics);
    ret = factor.x * vertexAttribute[0];
    ret += factor.y * (vertexAttribute[1]);
    ret += factor.z * (vertexAttribute[2]);
    return ret;
}

inline bool checkRecursiveLimit(inout Payload payload) {
    payload.recursive++;
    if (payload.recursive >= MAX_RECURSIVE) {
        payload.color = float3(0, 0, 0);
        return true;
    }
    return false;
}

void ShootRay(float3 origin, float3 dir, inout Payload payload)
{
    RayDesc rayDesc;
    rayDesc.Origin = origin;
    rayDesc.Direction = dir;
    rayDesc.TMin = 0.01;
    rayDesc.TMax = 100000;

    TraceRay(
        gRtScene,
        RAY_FLAG_NONE,
        0xFF,
        0, // ray index
        1, // MultiplierForGeometryContrib
        0, // miss index
        rayDesc,
        payload);
}

bool ShootShadowRay(float3 origin, float3 direction, float tMax = 10000)
{
    RayDesc rayDesc;
    rayDesc.Origin = origin;
    rayDesc.Direction = direction;
    rayDesc.TMin = 0.001f;
    rayDesc.TMax = tMax;

    ShadowPayload payload;
    payload.isHit = true;

    RAY_FLAG flags = RAY_FLAG_SKIP_CLOSEST_HIT_SHADER;
    flags |= RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH;

    TraceRay(
        gRtScene,
        flags,
        0xFF,
        0, // ray index
        1, // MultiplierForGeometryContrib
        1, // miss index
        rayDesc,
        payload);

    return payload.isHit;
}

// ランダムにライトを抽出
LightParameter GetLight() {
    int index = clamp(0, gSceneParam.lightNum - 1, floor(GetRandomFor2D().x * gSceneParam.lightNum));
    LightParameter lightParam = gLightList.buffer[index];
    return lightParam;
}

// AreaLightの処理
float3 LightSampling(out LightSampleInfo info, inout float weight)
{
    // ランダムに選択
    int index = clamp(0, gSceneParam.lightNum - 1, int(GetRandomFor2D().x * gSceneParam.lightNum));
    SphereAreaLight light = gAreaLightList.buffer[index];
    
    // PDFを計算(pdf = 1 / (num * area))
    weight = 1.0f / (SurfaceAreaSphere(light) * float(gSceneParam.lightNum));

    return AreaSamplingSphere(GetRandomFor2D(), light, info);
}

#endif