#include "../../../Shader/CommonDXR.hlsli"

[shader("miss")]
void mainMiss(inout Payload payload) 
{
    // �Ƃ肠����Ray���O�ꂽ�ꍇ�͉������Ȃ�
    float4 color = GetSkyBoxColorFromHdr();
    //payload.color = color.xyz * payload.weight * 100.0f;
}

[shader("miss")]
void shadowMiss(inout ShadowPayload payload)
{
    payload.isHit = false;
}