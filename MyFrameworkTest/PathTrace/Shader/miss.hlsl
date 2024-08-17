#include "../../../Shader/CommonDXR.hlsli"

[shader("miss")]
void mainMiss(inout Payload payload) 
{
    // とりあえずRayが外れた場合は何もしない
    float4 color = GetSkyBoxColorFromHdr();
    //payload.color = color.xyz * payload.weight * 100.0f;
}

[shader("miss")]
void shadowMiss(inout ShadowPayload payload)
{
    payload.isHit = false;
}