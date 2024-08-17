struct VSOutput
{
	float4 Position : SV_POSITION;
	float2 UV : TEXCOORD0;
};

Texture2D colorTexture : register(t0);

cbuffer param : register(b0)
{
	float3 backColorUpper : packoffset(c0.x);
	float3 backColorLower : packoffset(c1.x);
	float backThreshold : packoffset(c0.w);
	float starThreshold : packoffset(c1.w);
	float3 sunColor : packoffset(c2.x);
	float2 uvOffset : packoffset(c3.x);
	float sphereSize : packoffset(c3.z);
}

SamplerState samp : register(s0);

float random(in float2 st) {
	return frac(sin(dot(st.xy,
		float2(11.9898, 78.233)))
		* 43758.5453123);
}

float noise(in float2 st) {
	float2 i = floor(st);
	float2 f = frac(st);

	float a = random(i);
	float b = random(i + float2(1.0, 0.0));
	float c = random(i + float2(0.0, 1.0));
	float d = random(i + float2(1.0, 1.0));

	float2 u = f * f * (3.0 - 2.0 * f);

	// Mix 4 coorners percentages
	return lerp(a, b, u.x) +
		(c - a) * u.y * (1.0 - u.x) +
		(d - b) * u.x * u.y;
}


float4 main(VSOutput In) : SV_TARGET
{
	// star
	float starColor = pow(noise(float2(In.UV) * 1000.0), 40.0) * 5.0;
	float3 star = noise(In.UV * noise(float2(1.0,1.0)) * 1000.0) * starColor;
	star = clamp(0.0, 1.0, star);
	star = lerp(0.0, star, In.UV.y < starThreshold);

	// background
	float back = smoothstep(backThreshold, 1.0f, In.UV.y);
	float3 backColor = lerp(backColorUpper, backColorLower, back) + star;

	// ‹…‚Ì’Šo
	const float screenRatio = 720.0 / 1280.0;
	float2 uvRatio = In.UV * float2(1.0, screenRatio);
	float sphereLen = length(uvRatio - uvOffset) < sphereSize;
	
	// ü‚ð“ü‚ê‚é
	float divideThreshold = smoothstep(0.0, 0.01, (uvRatio - uvOffset).y);// ”¼•ª
	float3 divideColor = lerp(sunColor,
		lerp(backColor, sunColor ,abs(sin(uvRatio.y * 200))),
		divideThreshold);

	float3 sun = lerp(backColor, divideColor, sphereLen);

	float3 color = sun;
	return float4(color, 1.0f);
}
