#include "BRDFData.hlsli"
#include "CalculateBRDF.hlsli"
#include "../../../Shader/CommonDXR.hlsli"

// PhongŒn‚ÌshadingŒvZ
void MatteShade(inout Payload payload, const BRDFData phong, const UtilData data)
{
    float3 L = phong.ambient.xyz * gSceneParam.ambientColor.xyz;

    LightParameter light = GetLight();
    float3 wi = GetLightDirection(data.worldPos, light);
    float nDotWi = dot(data.normal, wi);

    float3 lightScale = EvaluateLight(data.normal, data.worldPos, light);
    L += (phong.diffuse*INVPI + GlossarySpecular(phong, data, wi)) * lightScale * max(0.0f, nDotWi);

    payload.color = L;
}

// ”½ËŒvZ
void ReflectiveShade(inout Payload payload, const BRDFData reflective, const UtilData data)
{
    // ’´‚¦‚Ä‚¢‚éê‡‚Í‘Å‚¿Ø‚è
    if (MAX_RECURSIVE < payload.recursive)
    {
        return;
    }

    MatteShade(payload, reflective, data);

    float3 wi = float3(0.0f, 0.0f, 0.0f);
    float3 fr = ReflectiveSpecular(reflective, data, wi);

    // ”½Ëray‚ğ”ò‚Î‚·
    Payload refPayload;
    refPayload.recursive = payload.recursive + 1;
    refPayload.color = float3(0, 0, 0);
    ShootRay(data.worldPos, wi, refPayload);

    payload.recursive++; // Ray‚Ì”‚ğ‘‚â‚µ‚Ä‚¨‚­

    payload.color += fr * dot(data.normal, wi) * refPayload.color;
}

// PerfectTrans
void PerfectTransShade(inout Payload payload, const BRDFData brdf, const UtilData data)
{
    // ’´‚¦‚Ä‚¢‚éê‡‚Í‘Å‚¿Ø‚è
    if (MAX_RECURSIVE < payload.recursive)
    {
        return;
    }

    MatteShade(payload, brdf, data);

    float3 wi = float3(0.0f, 0.0f, 0.0f);
    float3 wt = float3(0.0f, 0.0f, 0.0f);
    float3 fr = ReflectiveSpecular(brdf, data, wi);

    // ”½Ëray
    Payload refPayload;
    refPayload.recursive = payload.recursive + 1;
    refPayload.color = float3(0, 0, 0);
    ShootRay(data.worldPos, wi, refPayload);

    bool tir = PerfectTir(brdf, data);

    if (tir)
    {
        payload.color += refPayload.color;
    }
    else
    {
        // “§‰ß‚ÌRay‚à”ò‚Î‚·ê‡
        float3 ft = PerfectTransSample(brdf, data, wt);

        // “§‰ßray‚ğ”ò‚Î‚·
        Payload transPayload;
        transPayload.recursive = payload.recursive + 1;
        transPayload.color = float3(0, 0, 0);
        ShootRay(data.worldPos, wt, transPayload);

        payload.color += fr * dot(data.normal, wi) * refPayload.color;
        payload.color += ft * dot(data.normal, wt) * transPayload.color;
    }

    payload.recursive++; // Ray‚Ì”‚ğ‘‚â‚µ‚Ä‚¨‚­
}


// “§‰ßŒvZ
void DielectricShade(inout Payload payload, const BRDFData brdf, const UtilData data)
{
    // ’´‚¦‚Ä‚¢‚éê‡‚Í‘Å‚¿Ø‚è
    if (MAX_RECURSIVE < payload.recursive)
    {
        return;
    }

    MatteShade(payload, brdf, data);

    float3 wi = float3(0.0f, 0.0f, 0.0f);
    float3 wt = float3(0.0f, 0.0f, 0.0f);

    const float temp_kr = FresnelReflector(brdf, data);
    const float3 kr = float3(temp_kr, temp_kr, temp_kr);

    float3 fr = FresnelSample(data, kr, wi);

    // ”½Ëray
    Payload refPayload;
    refPayload.recursive = payload.recursive + 1;
    refPayload.color = float3(0, 0, 0);
    ShootRay(data.worldPos, wi, refPayload);

    bool tir = DielectricTir(brdf, data);

    float nDotWi = dot(data.normal, wi);
    if (tir)
    {
        payload.color += refPayload.color;
    }
    else
    {
        // “§‰ß‚ÌRay‚à”ò‚Î‚·ê‡
        const float temp_kt = 1.0f - temp_kr;
        const float3 kt = float3(temp_kt, temp_kt, temp_kt);

        float3 ft = DielectricSample(brdf, data, kt, wt);

        // “§‰ßray‚ğ”ò‚Î‚·
        Payload transPayload;
        transPayload.recursive = payload.recursive + 1;
        transPayload.color = float3(0, 0, 0);
        ShootRay(data.worldPos, wt, transPayload);

        payload.color += fr * abs(nDotWi) * refPayload.color;
        payload.color += ft * abs(dot(data.normal, wt)) * transPayload.color;
    }

    payload.recursive++; // Ray‚Ì”‚ğ‘‚â‚µ‚Ä‚¨‚­
}

// Trace‚ğ“K‹X”»’f‚µ‚Ä‚­‚ê‚é‚â‚Â
void CalculateTrace(inout Payload payload, const BRDFData brdf, const UtilData data)
{
    // ’´‚¦‚Ä‚¢‚éê‡‚Í‘Å‚¿Ø‚è
    if (MAX_RECURSIVE < payload.recursive)
    {
        return;
    }

    // masked‚Ìê‡­‚µæ‚ÌTrace‚ğs‚¤
    Payload maskedRecursive;
    maskedRecursive.color = float3(1.0f, 1.0f, 1.0f);
    maskedRecursive.recursive = payload.recursive + 1;
    if(brdf.alpha < 0.9f)
    {
        // alpha‚ª¬‚³‚¢ê‡‚Í‹N“® 
        payload.recursive++;
        ShootRay(data.worldPos + WorldRayDirection() * EPSILON, WorldRayDirection(), payload);
        //payload.color = maskedRecursive.color;
        //payload.recursive = maskedRecursive.recursive;
        return;
    }

    if (brdf.index == 0)
    {
        // Phong
        MatteShade(payload, brdf, data);
    }
    else if (brdf.index == 1)
    {
        // ”½Ë
        ReflectiveShade(payload, brdf, data);
    }
    else if (brdf.index == 2)
    {
        // ŠÈˆÕ“§‰ß
        PerfectTransShade(payload, brdf, data);
    }
    else if (brdf.index == 3)
    {
       // “§‰ß
       DielectricShade(payload, brdf, data);
    }
}