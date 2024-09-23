#ifndef __SHADOW_COMMON__
#define __SHADOW_COMMON__

// 射影シャドウ
float3 CalculateProjectionShadow(float4 LVP, Texture2D shadowTex, in SamplerState samp)
{
    // UV空間へ
    float2 shadowUV = LVP.xy / LVP.w;
    shadowUV *= float2(0.5f, -0.5f);
    shadowUV += 0.5f;
	
    // shadowの判定
    float3 shadow = 1.0f;
    if (0.0f < shadowUV.x && shadowUV.x < 1.0f &&
		0.0f < shadowUV.y && shadowUV.y < 1.0f)
    {
        float param = shadowTex.Sample(samp, shadowUV).r;
        shadow = param < 1.0f ? float3(0.5, 0.5, 0.5) : float3(1.0, 1.0, 1.0);
    }
    
    return shadow;
}

// デプスシャドウ
float3 CalculateDepthShadow(float4 LVP, Texture2D shadowTex, in SamplerState samp)
{
    // UV空間へ
    float2 shadowUV = LVP.xy / LVP.w;
    shadowUV *= float2(0.5f, -0.5f);
    shadowUV += 0.5f;
	
    // LightScreenViewでのZ値
    float zLVP = LVP.z / LVP.w;
    
    // shadowの判定
    float3 shadow = 1.0f;
    if (0.0f < shadowUV.x && shadowUV.x < 1.0f &&
		0.0f < shadowUV.y && shadowUV.y < 1.0f)
    {
        // shadowマップのZ値と比較
        float shadowZ = shadowTex.Sample(samp, shadowUV).r;
        
        // LVPよりもshadowmapのZ値が小さいなら遮蔽されてる
        if(zLVP > shadowZ)
        {
            shadow *= 0.5f;
        }
    }
    
    return shadow;
}

// PCF(Percentage Closer Filtering)シャドウ
float3 CalculatePCFShadow(float4 LVP, float offset, Texture2D shadowTex, in SamplerState samp)
{
    // UV空間へ
    float2 shadowUV = LVP.xy / LVP.w;
    shadowUV *= float2(0.5f, -0.5f);
    shadowUV += 0.5f;
	
    // LightScreenViewでのZ値
    float zLVP = LVP.z / LVP.w;
    
    // shadowの判定
    float3 shadow = 1.0f;
    if (0.0f < shadowUV.x && shadowUV.x < 1.0f &&
		0.0f < shadowUV.y && shadowUV.y < 1.0f)
    {       
        // 3x3でサンプル
        float rate = 0.0f;
        for (int i = -1; i <= 1; i++)
        {
            for (int j = -1; j <= 1; j++)
            {
                float2 texOffset = float2(i * offset, j * offset);
                float shadowZ = shadowTex.Sample(samp, shadowUV + texOffset).r;
                rate = lerp(rate, rate + 1.0f, zLVP > (shadowZ + 0.0001f)); // Depth Biasで不自然な影を消す
            }
        }
        rate = rate / 9.0f; 

        // filterに応じてlerp
        shadow = lerp(1.0f, 0.5f, rate);
    }
    
    return shadow;
}

#endif