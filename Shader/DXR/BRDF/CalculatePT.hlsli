#include "BRDFData.hlsli"
#include "CalculateBRDF.hlsli"
#include "../../../Shader/CommonDXR.hlsli"

// Light�֌W
void SimpleLightCalculate(inout Payload payload, const BRDFData brdf, const UtilData data)
{
    float3 L = brdf.diffuse.xyz;
    payload.color += L * payload.weight;
}

// �]���p
float3 Evaluate(const BRDFData brdf, const UtilData data, const float3 wi)
{
    float3 result = float3(0.0, 0.0, 0.0);
    
    switch (brdf.index)
    {
    case 0:
        result = EvaluateLambert(brdf);
        break;

    case 1:
        result = EvaluatePhong(brdf, data, wi);
        break;
        
    case 2:
        result = EvaluateBlinnPhong(brdf, data, wi);
        break;

    case 3:
        result = EvaluateBlinn(brdf, data, wi);
        break;
    }
    
    return result;
}

float3 EvaluateSample(const BRDFData brdf, const UtilData data, out float3 wi, out float pdf)
{
    float3 result = float3(0.0, 0.0, 0.0);
    
    switch (brdf.index)
    {
    case 0:
        result = LambertSample(brdf, data, wi, pdf);
        break;

    case 1:
        result = PhongSample(brdf, data, wi, pdf);
        break;
        
    case 2:
        result = BlinnPhongSample(brdf, data, wi, pdf);
        break;

    case 3:
        result = BlinnSample(brdf, data, wi, pdf);
        break;
    }
    
    return result;
}

// Trace��K�X���f���Ă������
void CalculatePT(inout Payload payload, const BRDFData brdf, const UtilData data)
{
    // �����Ă���ꍇ�͑ł��؂�
    if (MAX_RECURSIVE < payload.recursive)
    {
        return;
    }

    float russian = max(payload.weight.x, max(payload.weight.y, payload.weight.z));

    uint2 launchIndex = DispatchRaysIndex().xy;
    uint4 seed = SetSeed(launchIndex);

    if(russian < Random(seed))
    {
        // ���V�A�����[���b�g�ɂ��ł��؂�
        return;
    }

    payload.weight /= russian <= 0.0 ? 1.0 : russian;

    // Blend Alpha�̏���
    if(brdf.alpha < 0.9f)
    {
        // alpha���������ꍇ�͋N��
        // �ђʂ������Ƃɂ���
        payload.recursive++;
        ShootRay(data.worldPos + WorldRayDirection() * EPSILON, WorldRayDirection(), payload);
        return;
    }

    // ���C�g�̏ꍇ��Emissive�𑫂��ďI��
    if (brdf.index == 10)
    {
       if (payload.recursive == 0)
        {
            // NEE�̂��ߌ����ɓ����������񎞂̂ݑ���
            SimpleLightCalculate(payload, brdf, data);
        }
        return;
    }
    
    // NEE
    float lightPdf = 1.0;
    float3 wi;
    LightSampleInfo info;
    float3 lightPos = LightSampling(info, lightPdf);
    float3 lightDir = normalize(lightPos - data.worldPos);
    float lightDistance = length(lightPos - data.worldPos);
        
    bool isHit = ShootShadowRay(data.worldPos + data.normal * EPSILON,
                                lightDir, lightDistance - 0.01f);

    if(isHit == false)
    {        
        // ��^���v�Z
        float cosine1 = abs(dot(data.normal, lightDir));
        float cosine2 = abs(dot(info.normal, -lightDir));
        
        // bsdf�̎Z�o
        wi = lightDir;
        float3 color = Evaluate(brdf, data, wi);
        
        // G�����v�Z
        float G = cosine1 * cosine2 / (lightDistance * lightDistance);
       payload.color += payload.weight * (color * G / lightPdf) * info.Le;
    }
    
    // Next Ray
    if (brdf.index != 10)
    {
        payload.recursive++;
        
        // Lambert
        float pdf = 1.0;
        float3 color = EvaluateSample(brdf, data, wi, pdf);

        float NoWI = max(dot(data.normal, wi), 0.001);

        // weight���v�Z��A����Ray���΂�
        payload.weight *= color * NoWI / pdf;
        ShootRay(data.worldPos, normalize(wi), payload);
    }
}