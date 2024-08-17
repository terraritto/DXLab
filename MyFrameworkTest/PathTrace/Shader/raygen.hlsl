#include "../../../Shader/CommonDXR.hlsli"

// Local Root Signature (for RayGen)
RWTexture2D<float4> gOutput : register(u0);

[shader("raygeneration")]
void mainRayGen() {
    float3 col = float3(0.0f, 0.0f, 0.0f);

    // Pixelä÷åW
    uint2 launchIndex = DispatchRaysIndex().xy;
    float2 dims = float2(DispatchRaysDimensions().xy);
    float2 invDims = 1.0f / dims; // âÊñ ÉTÉCÉY

    // ÉVÅ[Éhê∂ê¨
    uint4 seed = SetSeed(launchIndex);

    // Aspectä÷åW
    float aspect = dims.x / dims.y;

    // View/ProjçsóÒ
    matrix mtxViewInv = gSceneParam.mtxViewInv;
    matrix mtxProjInv = gSceneParam.mtxProjInv;

    // Rayä÷åW
    Payload payload;
    payload.color = float3(0.0, 0.0, 0.0);
    payload.recursive = 0;
    payload.weight = float3(1.0, 1.0, 1.0);

    float2 d = (launchIndex.xy + 0.5 + float2(Random(seed), Random(seed))) / dims.xy * 2.0 - 1.0;
    float4 target = mul(mtxProjInv, float4(d.x, -d.y, 1, 1));
    float3 dir = normalize(mul(mtxViewInv, float4(target.xyz, 0)).xyz);
    float3 origin = mul(mtxViewInv, float4(0, 0, 0, 1)).xyz;
    ShootRay(origin, dir, payload);

    float4 resultColor = max(float4(payload.color, 1.0f), 0.0);
    // í~êœ
    gOutput[launchIndex.xy] = gOutput[launchIndex.xy] + resultColor;
}
