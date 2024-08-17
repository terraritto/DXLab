struct VSOutput
{
	float4 Position : SV_POSITION;
	float2 UV : TEXCOORD0;
};

Texture2D colorTexture : register(t0);

cbuffer param : register(b0)
{
	float sampleCount : packoffset(c0.x);
	float strength : packoffset(c0.y);
	float2 uvOffset : packoffset(c0.z);
}

SamplerState samp : register(s0);

float4 main(VSOutput In) : SV_TARGET
{
	float3 color = float3(0.0f, 0.0f, 0.0f);
	float2 uv = In.UV - uvOffset;
	float factor = strength / sampleCount * length(uv);

	for (int j = 0; j < sampleCount; j++)
	{
		float offset = 1 - factor * j;
		color += colorTexture.Sample(samp, uv * offset + uvOffset).rgb;
	}

	return float4(color / sampleCount, 1.0f);
}
