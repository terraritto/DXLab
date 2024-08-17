#include "BRDFData.hlsli"
#include "CalculateBRDF.hlsli"
#include "../../../Shader/CommonDXR.hlsli"

// Phong�n��shading�v�Z
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

// ���ˌv�Z
void ReflectiveShade(inout Payload payload, const BRDFData reflective, const UtilData data)
{
    // �����Ă���ꍇ�͑ł��؂�
    if (MAX_RECURSIVE < payload.recursive)
    {
        return;
    }

    MatteShade(payload, reflective, data);

    float3 wi = float3(0.0f, 0.0f, 0.0f);
    float3 fr = ReflectiveSpecular(reflective, data, wi);

    // ����ray���΂�
    Payload refPayload;
    refPayload.recursive = payload.recursive + 1;
    refPayload.color = float3(0, 0, 0);
    ShootRay(data.worldPos, wi, refPayload);

    payload.recursive++; // Ray�̐��𑝂₵�Ă���

    payload.color += fr * dot(data.normal, wi) * refPayload.color;
}

// PerfectTrans
void PerfectTransShade(inout Payload payload, const BRDFData brdf, const UtilData data)
{
    // �����Ă���ꍇ�͑ł��؂�
    if (MAX_RECURSIVE < payload.recursive)
    {
        return;
    }

    MatteShade(payload, brdf, data);

    float3 wi = float3(0.0f, 0.0f, 0.0f);
    float3 wt = float3(0.0f, 0.0f, 0.0f);
    float3 fr = ReflectiveSpecular(brdf, data, wi);

    // ����ray
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
        // ���߂�Ray����΂��ꍇ
        float3 ft = PerfectTransSample(brdf, data, wt);

        // ����ray���΂�
        Payload transPayload;
        transPayload.recursive = payload.recursive + 1;
        transPayload.color = float3(0, 0, 0);
        ShootRay(data.worldPos, wt, transPayload);

        payload.color += fr * dot(data.normal, wi) * refPayload.color;
        payload.color += ft * dot(data.normal, wt) * transPayload.color;
    }

    payload.recursive++; // Ray�̐��𑝂₵�Ă���
}


// ���ߌv�Z
void DielectricShade(inout Payload payload, const BRDFData brdf, const UtilData data)
{
    // �����Ă���ꍇ�͑ł��؂�
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

    // ����ray
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
        // ���߂�Ray����΂��ꍇ
        const float temp_kt = 1.0f - temp_kr;
        const float3 kt = float3(temp_kt, temp_kt, temp_kt);

        float3 ft = DielectricSample(brdf, data, kt, wt);

        // ����ray���΂�
        Payload transPayload;
        transPayload.recursive = payload.recursive + 1;
        transPayload.color = float3(0, 0, 0);
        ShootRay(data.worldPos, wt, transPayload);

        payload.color += fr * abs(nDotWi) * refPayload.color;
        payload.color += ft * abs(dot(data.normal, wt)) * transPayload.color;
    }

    payload.recursive++; // Ray�̐��𑝂₵�Ă���
}

// Trace��K�X���f���Ă������
void CalculateTrace(inout Payload payload, const BRDFData brdf, const UtilData data)
{
    // �����Ă���ꍇ�͑ł��؂�
    if (MAX_RECURSIVE < payload.recursive)
    {
        return;
    }

    // masked�̏ꍇ�������Trace���s��
    Payload maskedRecursive;
    maskedRecursive.color = float3(1.0f, 1.0f, 1.0f);
    maskedRecursive.recursive = payload.recursive + 1;
    if(brdf.alpha < 0.9f)
    {
        // alpha���������ꍇ�͋N�� 
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
        // ����
        ReflectiveShade(payload, brdf, data);
    }
    else if (brdf.index == 2)
    {
        // �ȈՓ���
        PerfectTransShade(payload, brdf, data);
    }
    else if (brdf.index == 3)
    {
       // ����
       DielectricShade(payload, brdf, data);
    }
}