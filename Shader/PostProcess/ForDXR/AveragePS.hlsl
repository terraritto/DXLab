struct VSOutput
{
	float4 Position : SV_POSITION;
	float2 UV : TEXCOORD0;
};

Texture2D colorTexture : register(t0);
cbuffer param : register(b0)
{
	int spp;
}

SamplerState samp : register(s0);

float4 main(VSOutput In) : SV_TARGET
{
	float3 color = colorTexture.Sample(samp, In.UV).rgb / float(spp);
	return float4(color, 1.0f);
}
