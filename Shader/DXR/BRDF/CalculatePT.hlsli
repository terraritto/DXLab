#include "BRDFData.hlsli"
#include "CalculateBRDF.hlsli"
#include "../../../Shader/CommonDXR.hlsli"

// Light関係
void SimpleLightCalculate(inout Payload payload, const BRDFData brdf, const UtilData data)
{
    float3 L = brdf.diffuse.xyz;
    payload.color += L * payload.weight;
}

// 評価用
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

// Traceを適宜判断してくれるやつ
void CalculatePT(inout Payload payload, const BRDFData brdf, const UtilData data)
{
    // 超えている場合は打ち切り
    if (MAX_RECURSIVE < payload.recursive)
    {
        return;
    }

    float russian = max(payload.weight.x, max(payload.weight.y, payload.weight.z));

    uint2 launchIndex = DispatchRaysIndex().xy;
    uint4 seed = SetSeed(launchIndex);

    if(russian < Random(seed))
    {
        // ロシアンルーレットによる打ち切り
        return;
    }

    payload.weight /= russian <= 0.0 ? 1.0 : russian;

    // Blend Alphaの処理
    if(brdf.alpha < 0.9f)
    {
        // alphaが小さい場合は起動
        // 貫通したことにする
        payload.recursive++;
        ShootRay(data.worldPos + WorldRayDirection() * EPSILON, WorldRayDirection(), payload);
        return;
    }

    // ライトの場合はEmissiveを足して終了
    if (brdf.index == 10)
    {
       if (payload.recursive == 0)
        {
            // NEEのため光源に当たった初回時のみ足す
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
        // 寄与を計算
        float cosine1 = abs(dot(data.normal, lightDir));
        float cosine2 = abs(dot(info.normal, -lightDir));
        
        // bsdfの算出
        wi = lightDir;
        float3 color = Evaluate(brdf, data, wi);
        
        // G項を計算
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

        // weightを計算後、次のRayを飛ばす
        payload.weight *= color * NoWI / pdf;
        ShootRay(data.worldPos, normalize(wi), payload);
    }
}