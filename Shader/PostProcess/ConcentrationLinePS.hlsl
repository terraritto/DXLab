struct VSOutput
{
	float4 Position : SV_POSITION;
	float2 UV : TEXCOORD0;
};

Texture2D colorTexture : register(t0);

cbuffer param : register(b0)
{
	float3 lineColor : packoffset(c0);
	float time : packoffset(c0.w);
	float speed : packoffset(c1.x);
	float noiseScale : packoffset(c1.y);
	float2 uvOffset : packoffset(c1.z);
	float lineRegion : packoffset(c2);
}

SamplerState samp : register(s0);

float2 GradientNoiseDir(float2 p)
{
	p = p % 289;
	float x = (34 * p.x + 1) * p.x % 289 + p.y;
	x = (34 * x + 1) * x % 289;
	x = frac(x / 41) * 2 - 1;
	return normalize(float2(x - floor(x + 0.5), abs(x) - 0.5));
}

float GradientNoise(float2 p)
{
	float2 ip = floor(p);
	float2 fp = frac(p);
	float d00 = dot(GradientNoiseDir(ip), fp);
	float d01 = dot(GradientNoiseDir(ip + float2(0, 1)), fp - float2(0, 1));
	float d10 = dot(GradientNoiseDir(ip + float2(1, 0)), fp - float2(1, 0));
	float d11 = dot(GradientNoiseDir(ip + float2(1, 1)), fp - float2(1, 1));
	fp = fp * fp * fp * (fp * (fp * 6 - 15) + 10);
	return lerp(lerp(d00, d01, fp.y), lerp(d10, d11, fp.y), fp.x);
}

float GradientNoiseScale(float2 UV, float Scale)
{
	return GradientNoise(UV * Scale) + 0.5;
}

float2 ToPolar(float2 xy)
{
	float r = length(xy);
	float rad = atan2(xy.x, xy.y);
	return float2(r, rad);
}

float4 main(VSOutput In) : SV_TARGET
{
	float2 uv = ToPolar(In.UV - uvOffset);
	float length = uv.r;
	float rad = (uv.g / (3.1415926535) + 1.0f) / 2.0f;

	float noise = GradientNoiseScale(
		float2(rad, rad) + float2(time * speed, 0.0f),noiseScale
	);

	float y = 1.0f - length + noise;
	float region = smoothstep(lineRegion, 1.0f, y);

	float3 color = colorTexture.Sample(samp, In.UV).rgb;
	color = lerp(lineColor, color, region.r);
	return float4(color, 1.0f);
}