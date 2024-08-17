#include "Shader/RayMarching/Common.hlsli"
#include "Shader/RayMarching/Intersection.hlsli"

struct VSOutput
{
	float4 Position : SV_POSITION;
	float2 UV : TEXCOORD0;
};

Texture2D colorTexture : register(t0);
SamplerState samp : register(s0);

float4 main(VSOutput In) : SV_TARGET
{
	float3 color = colorTexture.Sample(samp, In.UV).rgb;
	
	if (In.UV.r < 0.5f)
	{
		color = float3(0.0f, 0.0f, 0.0f);
	}

	return float4(color,1.0f);
}
