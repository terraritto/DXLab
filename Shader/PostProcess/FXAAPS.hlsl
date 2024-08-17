#define FXAA_EDGE_THRESHOLD 1/8
#define FXAA_EDGE_THRESHOLD_MIN 1/32
#define FXAA_SUBPIX_TRIM 1/8
#define FXAA_SUBPIX_TRIM_SCALE 1.0/(1.0-FXAA_SUBPIX_TRIM)
#define FXAA_SUBPIX_CAP 7.0/8.0
#define FXAA_SEARCH_THRESHOLD 1/8
#define FXAA_SEARCH_STEPS 16

struct VSOutput
{
	float4 Position : SV_POSITION;
	float2 UV : TEXCOORD0;
};

Texture2D colorTexture : register(t0);

cbuffer param : register(b0)
{
	float2 inv : packoffset(c0.x);
	float isUseFXAA : packoffset(c0.z);
}

SamplerState samp : register(s0);

// 輝度への変換
float Luma(float3 color)
{
	return color.y * (0.587 / 0.299) + color.x;
}

// テクスチャのオフセット
float3 TextureOffset(Texture2D tex, float2 uv, float2 offset)
{
	return tex.Sample(samp, uv + offset * inv).rgb;
}

float4 main(VSOutput In) : SV_TARGET
{
	float3 color = float3(0.0f, 0.0f, 0.0f);
	float2 uv = In.UV;
	color = colorTexture.Sample(samp, uv).rgb;
	
	if (isUseFXAA < 0.5f)
	{
		return float4(color, 0.0f);
	}

	// Local Contrast Check
	float3 rgbN = TextureOffset(colorTexture, uv, float2(0.0, -1.0));
	float3 rgbW = TextureOffset(colorTexture, uv, float2(-1.0, 0.0));
	float3 rgbM = TextureOffset(colorTexture, uv, float2(0.0, 0.0));
	float3 rgbE = TextureOffset(colorTexture, uv, float2(1.0, 0.0));
	float3 rgbS = TextureOffset(colorTexture, uv, float2(0.0, 1.0));
	float lumaN = Luma(rgbN);
	float lumaW = Luma(rgbW);
	float lumaM = Luma(rgbM);
	float lumaE = Luma(rgbE);
	float lumaS = Luma(rgbS);
	float rangeMin = min(lumaM, min(min(lumaN, lumaW), min(lumaS, lumaE)));
	float rangeMax = max(lumaM, max(max(lumaN, lumaW), max(lumaS, lumaE)));
	float range = rangeMax - rangeMin;
	if (range < max(FXAA_EDGE_THRESHOLD_MIN, rangeMax * FXAA_EDGE_THRESHOLD))
	{
		return float4(color, 1.0f);
	}

	// Sub-pixel Aliasing Test
	float lumaL = (lumaN + lumaW + lumaE + lumaS) * 0.25;
	float rangeL = abs(lumaL - lumaM);
	float blendL = max(0.0, (rangeL / range) - FXAA_SUBPIX_TRIM) * FXAA_SUBPIX_TRIM_SCALE;
	blendL = min(FXAA_SUBPIX_CAP, blendL);

	// lowpass
	float3 rgbL = rgbN + rgbW + rgbM + rgbE + rgbS;
	float3 rgbNW = TextureOffset(colorTexture, uv, float2(-1.0, -1.0));
	float3 rgbNE = TextureOffset(colorTexture, uv, float2(1.0, -1.0));
	float3 rgbSW = TextureOffset(colorTexture, uv, float2(-1.0, 1.0));
	float3 rgbSE = TextureOffset(colorTexture, uv, float2(1.0, 1.0));
	rgbL += rgbNW + rgbNE + rgbSW + rgbSE;
	rgbL *= 1.0 / 9.0;

	// Vertical/Horizontal Edge Test
	float lumaNW = Luma(rgbNW);
	float lumaNE = Luma(rgbNE);
	float lumaSW = Luma(rgbSW);
	float lumaSE = Luma(rgbSE);

	float edgeVert =
		abs((0.25 * lumaNW) + (-0.5 * lumaN) + (0.25 * lumaNE)) +
		abs((0.50 * lumaW) +  (-1.0 * lumaM) + (0.50 * lumaE))  +
		abs((0.25 * lumaSW) + (-0.5 * lumaS) + (0.25 * lumaSE));
	float edgeHorz =
		abs((0.25 * lumaNW) + (-0.5 * lumaW) + (0.25 * lumaSW)) +
		abs((0.50 * lumaN)  + (-1.0 * lumaM) + (0.50 * lumaS))  +
		abs((0.25 * lumaNE) + (-0.5 * lumaE) + (0.25 * lumaSE));
	bool horzSpan = edgeHorz >= edgeVert;
	
	if (!horzSpan) { lumaN = lumaW; }
	if (!horzSpan) { lumaS = lumaE; }
	float gradientN = abs(lumaN - lumaM);
	float gradientS = abs(lumaS - lumaM);
	lumaN = (lumaN + lumaM) * 0.5;
	lumaS = (lumaS + lumaM) * 0.5;

	// gradientを選ぶ
    float lengthSign = horzSpan ? -inv.y : -inv.x;
	bool pairN = gradientN >= gradientS;
	if (!pairN) { lumaN = lumaS; }
	if (!pairN) { gradientN = gradientS; }
	if (!pairN) { lengthSign *= -1.0; }
	float2 posN;
	posN.x = uv.x + (horzSpan ? 0.0f : lengthSign * 0.5f);
	posN.y = uv.y + (horzSpan ? lengthSign * 0.5f : 0.0f);

	gradientN *= FXAA_SEARCH_THRESHOLD;

	// End of edge Search
	float2 posP = posN;
	float2 offNP = horzSpan ? float2(inv.x, 0.0f) : float2(0.0f, inv.y);
	float2 lumaEndN = lumaN;
	float2 lumaEndP = lumaN;
	bool doneN = false, doneP = false;

	// Searchは加速なし
	posN += offNP * float2(-1.0f, -1.0f);
	posP += offNP * float2(1.0f, 1.0f);

	for (int i = 0; i < FXAA_SEARCH_STEPS; i++)
	{
		if (!doneN) { lumaEndN = Luma(colorTexture.Sample(samp, posN.xy).rgb); }
		if (!doneP) { lumaEndP = Luma(colorTexture.Sample(samp, posP.xy).rgb); }

		doneN = doneN || (abs(lumaEndN - lumaN) >= gradientN);
		doneP = doneP || (abs(lumaEndP - lumaN) >= gradientN);
		if (doneN && doneP) { break; }

		if (!doneN) { posN -= offNP; }
		if (!doneP) { posP += offNP; }
	}

	// + or -側のどっちの中心かを処理
	float dstN = horzSpan ? uv.x - posN.x : uv.y - posN.y;
	float dstP = horzSpan ? posP.x - uv.x : posP.y - uv.y;
	bool directionN = dstN < dstP;
	lumaEndN = directionN ? lumaEndN : lumaEndP;

	bool a = (lumaM - lumaN) < 0.0f;
	bool b = (lumaEndN - lumaN) < 0.0f;
	if (a == b)
	{
		lengthSign = 0.0f;
	}

	// SubPixelのOffsetでフィルタ
	float spanLength = dstP + dstN;
	dstN = directionN ? dstN : dstP;
	float subPixelOffset = (0.5f + (dstN * (-1.0f / spanLength))) * lengthSign;
	float2 lastUV;
	lastUV.x = uv.x + (horzSpan ? 0.0f : subPixelOffset);
	lastUV.y = uv.y + (horzSpan ? subPixelOffset: 0.0f);
	color = colorTexture.Sample(samp, lastUV).rgb;
	color = lerp(rgbL, color, blendL);

	return float4(color, 1.0f);
}