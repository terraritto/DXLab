#ifndef __SHADOW_COMMON__
#define __SHADOW_COMMON__

// �ˉe�V���h�E
float3 CalculateProjectionShadow(float4 LVP, Texture2D shadowTex, in SamplerState samp)
{
    // UV��Ԃ�
    float2 shadowUV = LVP.xy / LVP.w;
    shadowUV *= float2(0.5f, -0.5f);
    shadowUV += 0.5f;
	
    // shadow�̔���
    float3 shadow = 1.0f;
    if (0.0f < shadowUV.x && shadowUV.x < 1.0f &&
		0.0f < shadowUV.y && shadowUV.y < 1.0f)
    {
        float param = shadowTex.Sample(samp, shadowUV).r;
        shadow = param < 1.0f ? float3(0.5, 0.5, 0.5) : float3(1.0, 1.0, 1.0);
    }
    
    return shadow;
}

// �f�v�X�V���h�E
float3 CalculateDepthShadow(float4 LVP, Texture2D shadowTex, in SamplerState samp)
{
    // UV��Ԃ�
    float2 shadowUV = LVP.xy / LVP.w;
    shadowUV *= float2(0.5f, -0.5f);
    shadowUV += 0.5f;
	
    // LightScreenView�ł�Z�l
    float zLVP = LVP.z / LVP.w;
    
    // shadow�̔���
    float3 shadow = 1.0f;
    if (0.0f < shadowUV.x && shadowUV.x < 1.0f &&
		0.0f < shadowUV.y && shadowUV.y < 1.0f)
    {
        // shadow�}�b�v��Z�l�Ɣ�r
        float shadowZ = shadowTex.Sample(samp, shadowUV).r;
        
        // LVP����shadowmap��Z�l���������Ȃ�Օ�����Ă�
        if(zLVP > shadowZ)
        {
            shadow *= 0.5f;
        }
    }
    
    return shadow;
}

// PCF(Percentage Closer Filtering)�V���h�E
float3 CalculatePCFShadow(float4 LVP, float offset, Texture2D shadowTex, in SamplerState samp)
{
    // UV��Ԃ�
    float2 shadowUV = LVP.xy / LVP.w;
    shadowUV *= float2(0.5f, -0.5f);
    shadowUV += 0.5f;
	
    // LightScreenView�ł�Z�l
    float zLVP = LVP.z / LVP.w;
    
    // shadow�̔���
    float3 shadow = 1.0f;
    if (0.0f < shadowUV.x && shadowUV.x < 1.0f &&
		0.0f < shadowUV.y && shadowUV.y < 1.0f)
    {       
        // 3x3�ŃT���v��
        float rate = 0.0f;
        for (int i = -1; i <= 1; i++)
        {
            for (int j = -1; j <= 1; j++)
            {
                float2 texOffset = float2(i * offset, j * offset);
                float shadowZ = shadowTex.Sample(samp, shadowUV + texOffset).r;
                rate = lerp(rate, rate + 1.0f, zLVP > (shadowZ + 0.0001f)); // Depth Bias�ŕs���R�ȉe������
            }
        }
        rate = rate / 9.0f; 

        // filter�ɉ�����lerp
        shadow = lerp(1.0f, 0.5f, rate);
    }
    
    return shadow;
}

#endif