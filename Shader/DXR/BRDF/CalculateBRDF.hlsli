#include "BRDFData.hlsli"
#include "../../../Shader/CommonDXR.hlsli"

// @Lambert begin
float3 LambertSample(const BRDFData brdf, const UtilData data, out float3 wi, out float pdf)
{
    float3 u,v,w = data.normal;
    OrthonormalBasis(u, v, w);

    float3 sr = SampleHemisphere();
    wi = normalize(sr.r * u + sr.g * v + sr.b * w);
    pdf = max(dot(data.normal, wi), 0.0) * INVPI;
    pdf = pdf <= 0.0 ? 0.001 : pdf;
    return brdf.diffuse * INVPI;
}

float3 EvaluateLambert(const BRDFData brdf)
{
    return brdf.diffuse * INVPI;
}
// @Lambert end

// @Phong begin
float3 PhongSample(const BRDFData phong, const UtilData data, out float3 wi, out float pdf)
{
    // Sampleï˚å¸åvéZ
    float3 u, v, w = data.normal;
    OrthonormalBasis(u, v, w);

    float3 sr = SampleHemisphere();
    wi = normalize(sr.r * u + sr.g * v + sr.b * w);
    
    float3 L = phong.diffuse * INVPI;

    float3 wo = -WorldRayDirection();
    
    // îΩéÀÉxÉNÉgÉãÇÃåvéZ
    float nDotWi = dot(data.normal, wi);
    float3 rvec = normalize(-wi + 2.0 * data.normal * nDotWi);
    
    // PhongÇÃíÜêgÇåvéZ
    float rDotWo = dot(rvec, wo);

    // PhongÇÃåvéZ
    float lobe = pow(rDotWo, phong.specular.w);
    if (rDotWo > 0.0)
    {
        L += phong.specular.xyz * lobe * (phong.specular.w + 2.0f) * INVPI2;
    }
    
    // pdfÇÃåvéZ
    pdf = lobe * (phong.specular.w + 1.0) * INVPI2;
    
    return L;
}

float3 EvaluatePhong(const BRDFData phong, const UtilData data, const float3 wi)
{    
    float3 L = phong.diffuse * INVPI;

    float3 wo = -WorldRayDirection();
    
    // îΩéÀÉxÉNÉgÉãÇÃåvéZ
    float nDotWi = dot(data.normal, wi);
    float3 rvec = normalize(-wi + 2.0 * data.normal * nDotWi);
    
    // PhongÇÃíÜêgÇåvéZ
    float rDotWo = dot(rvec, wo);

    // PhongÇÃåvéZ
    if (rDotWo > 0.0)
    {
        L += phong.specular.xyz * saturate(pow(rDotWo, phong.specular.w))
            * (phong.specular.w + 2.0f) * INVPI2;
;
    }
        
    return L;
}
// @Phong End

// @Blinn-Phong begin
float3 BlinnPhongSample(const BRDFData phong, const UtilData data, out float3 wi, out float pdf)
{
    // Sampleï˚å¸åvéZ
    float3 u, v, w = data.normal;
    OrthonormalBasis(u, v, w);

    float3 sr = SampleHemisphere();
    wi = normalize(sr.r * u + sr.g * v + sr.b * w);
    
    float3 L = phong.diffuse * INVPI;
    
    float3 wo = -WorldRayDirection();
    float3 wh = normalize(wi + wo);
    float rDotWo = dot(wh, data.normal);

    float lobe = pow(rDotWo, phong.specular.w);
    float coef = (phong.specular.w + 8.0) / (8.0 * PI);
    if (rDotWo > 0.0)
    {
        L += phong.specular.xyz * saturate(lobe) * coef;
    }
    
    // pdfÇÃåvéZ
    float WiDotH = dot(wh, wi);
    pdf = pow(sr.z, phong.specular.w) / (4.0 * WiDotH);
    
    return L;
}

float3 EvaluateBlinnPhong(const BRDFData phong, const UtilData data, const float3 wi)
{
    float3 L = phong.diffuse * INVPI;
        
    float3 wo = -WorldRayDirection();
    float3 wh = normalize(wi + wo);
    float rDotWo = dot(wh, data.normal);

    float coef = (phong.specular.w + 8.0) / (8.0 * PI);
    if (rDotWo > 0.0)
    {
        L += phong.specular.xyz * saturate(pow(rDotWo, phong.specular.w)) * coef;
    }
            
    return L;
}
// @Blinn-Phong End

// @Blinn Start
float3 BlinnSample(const BRDFData brdf, const UtilData data, out float3 L, out float pdf)
{
    // Sampleï˚å¸åvéZ
    float3 u, v, w = data.normal;
    OrthonormalBasis(u, v, w);

    float3 sr = SampleHemisphere();
    L = normalize(sr.r * u + sr.g * v + sr.b * w);
    
    float3 Lo = brdf.diffuse * INVPI;
            
    float3 V = -WorldRayDirection();
    float3 H = normalize(L + V);
    float3 N = data.normal;
    
    float NoH = dot(N, H);
    float VoH = dot(V, H);
    float NoL = dot(N, L);
    float NoV = dot(N, V);
    
    // DçÄ
    float x = acos(NoH) * brdf.specular.w;
    float D = exp(-x * x);
    
    // GçÄ
    float G = 0.0;
    if (NoV < NoL)
    {
        if (2.0 * NoV * NoH < VoH)
        {
            G = 2.0 * NoH / VoH;
        }
        else
        {
            G = 1.0 / NoV;
        }
    }
    else
    {
        if (2.0 * NoL * NoH < VoH)
        {
            G = 2.0 * NoH * NoL / (VoH * NoV);
        }
        else
        {
            G = 1.0 / NoV;
        }
    }
    
    // FçÄ
    float c = VoH;
    float g = sqrt(brdf.etaIn * brdf.etaIn + c * c - 1.0);
    float coef1 = pow((c * g + c) - 1, 2.0f);
    float coef2 = pow((c * g - c) + 1, 2.0f);
    float F = pow(g - c, 2.0) / pow(g + c, 2.0);
    F *= 1.0 + coef1 / coef2;

    if (NoH > 0.0)
    {
        Lo += saturate(brdf.specular.xyz * (D * G * F) / NoV);
    }
    
    pdf = D * NoL / (4.0 * dot(V, L));
    
    return Lo;
}

float3 EvaluateBlinn(const BRDFData brdf, const UtilData data, const float3 L)
{
    float3 Lo = brdf.diffuse * INVPI;
    
    float3 V = -WorldRayDirection();
    float3 H = normalize(L + V);
    float3 N = data.normal;
    
    float NoH = dot(N, H);
    float VoH = dot(V, H);
    float NoL = dot(N, L);
    float NoV = dot(N, V);
    
    // DçÄ
    float x = acos(NoH) * brdf.specular.w;
    float D = exp(-x * x);
    
    // GçÄ
    float G = 0.0;
    if(NoV < NoL)
    {
        if(2.0 * NoV * NoH < VoH)
        {
            G = 2.0 * NoH / VoH;
        }
        else
        {
            G = 1.0 / NoV;
        }
    }
    else
    {
        if (2.0 * NoL * NoH < VoH)
        {
            G = 2.0 * NoH * NoL / (VoH * NoV);
        }
        else
        {
            G = 1.0 / NoV;
        }
    }
    
    // FçÄ
    float c = VoH;
    float g = sqrt(brdf.etaIn * brdf.etaIn + c * c - 1.0);
    float coef1 = pow((c * g + c) - 1, 2.0f);
    float coef2 = pow((c * g - c) + 1, 2.0f);
    float F = pow(g - c, 2.0) / pow(g + c, 2.0);
    F *= 1.0 + coef1 / coef2;

    if (NoH > 0.0)
    {
        Lo += saturate(brdf.specular.xyz * (D * G * F) / NoV);
    }
            
    return Lo;
}
// @Blinn End

float3 GlossarySpecular(const BRDFData phong, const UtilData data, const float3 wi)
{
    float3 L = float3(0.0f, 0.0f, 0.0f);

    float3 wo = -WorldRayDirection();
    float nDotWi = dot(data.normal, wi);
    float3 rvec = normalize(-wi + 2.0 * data.normal * nDotWi);
    float rDotWo = dot(rvec, wo);

    if (rDotWo > 0.0)
    {
        L = phong.specular.xyz * saturate(pow(rDotWo, phong.specular.w));
    }

    return L;
}

float3 BlinnPhong(const BRDFData phong, const UtilData data, const float3 wi)
{
    float3 L = float3(0.0f, 0.0f, 0.0f);

    float3 wo = -WorldRayDirection();
    float3 wh = normalize(wi + wo);
    float rDotWo = dot(wh, data.normal);

    if (rDotWo > 0.0)
    {
        L = phong.specular.xyz * pow(rDotWo, phong.specular.w);
    }

    return L;
}

// Reflective
float3 ReflectiveSpecular(const BRDFData reflective, const UtilData data, inout float3 wi)
{
    float3 wo = -WorldRayDirection();
    float nDotWo = dot(data.normal, wo);
    wi = normalize(-wo + 2.0 * data.normal * nDotWo);
    return (reflective.reflectColor / abs(dot(data.normal, wi)));
}

// PerfectTransmit(BTDF)
bool PerfectTir(const BRDFData brdf, const UtilData data)
{
    float nDotD = dot(data.normal, -data.dir);
    float eta = brdf.etaIn;

    if (nDotD < 0.0f)
    {
        eta = 1.0f / eta;
    }

    return ((1.0f - (1.0f - nDotD * nDotD) / (eta * eta)) < 0.0f);
}

float3 PerfectTransSample(const BRDFData brdf, const UtilData data, inout float3 wt)
{
    float3 normal = data.normal;
    float3 wo = -data.dir;
    float nDotWo = dot(data.normal, wo);
    float eta = brdf.etaIn;

    if (nDotWo < 0.0f)
    {
        nDotWo = -nDotWo;
        normal = -normal;
        eta = 1.0f / eta;
    }


    float temp = 1.0f - (1.0f - nDotWo * nDotWo) / (eta * eta);
    float cos_theta_2 = sqrt(temp);
    wt = -wo / eta - (cos_theta_2 - (nDotWo / eta)) * data.normal;
    return brdf.etaOut / abs(dot(data.normal, wt));
}

// Fresnel
float FresnelReflector(const BRDFData brdf, const UtilData data)
{
    float3 normal = data.normal;
    float nDotD = dot(normal, -data.dir);
    float eta;

    if (nDotD < 0.0f)
    {
        normal = -normal;
        eta = brdf.etaOut / brdf.etaIn;
    }
    else
    {
        eta = brdf.etaIn / brdf.etaOut;
    }
    
    const float cos_theta_i = -dot(normal, data.dir);
    float temp = 1.0f - (1.0f - cos_theta_i * cos_theta_i) / (eta * eta);
    const float cos_theta_t = sqrt(1.0f - (1.0f - cos_theta_i * cos_theta_i) / (eta * eta));
    const float r_parallel = (eta * cos_theta_i - cos_theta_t) / (eta * cos_theta_i + cos_theta_t);
    const float r_perpendicular = (cos_theta_i - eta * cos_theta_t) / (cos_theta_i + eta * cos_theta_t);
    return 0.5f * ((r_parallel * r_parallel) + (r_perpendicular * r_perpendicular));
}

float3 FresnelSample(const UtilData data, const float3 kr, inout float3 wr)
{
    float3 wo = -data.dir;
    float nDotWo = dot(data.normal, wo);
    wr = normalize(-wo + 2.0 * data.normal * nDotWo);
    return kr / abs(dot(data.normal, wr));
}

// tir
bool DielectricTir(const BRDFData brdf, const UtilData data)
{
    float nDotD = dot(data.normal, -data.dir);
    float eta;

    if (nDotD < 0.0f)
    {
        eta = brdf.etaOut / brdf.etaIn;
    }
    else
    {
        eta = brdf.etaIn / brdf.etaOut;
    }

    return (1.0f - (1.0f - nDotD * nDotD) / (eta * eta) < 0.0f);
}

float DielectricReflector(const BRDFData brdf, const UtilData data)
{
    return 1.0f - FresnelReflector(brdf, data);
}

float3 DielectricSample(const BRDFData brdf, const UtilData data, const float3 kt, inout float3 wt)
{
    float3 normal = data.normal;
    float3 wo = -data.dir;
    float nDotWo = dot(normal, wo);
    float eta;

    if (nDotWo < 0.0f)
    {
        nDotWo = -nDotWo;
        normal = -normal;
        eta = brdf.etaOut / brdf.etaIn;
    }
    else
    {
        eta = brdf.etaIn / brdf.etaOut;
    }

    float temp = 1.0f - (1.0f - nDotWo * nDotWo) / (eta * eta);
    float cos_theta_2 = sqrt(temp);
    wt = -wo / eta - (cos_theta_2 - (nDotWo / eta)) * normal;
    return kt / (eta * eta) / abs(dot(data.normal, wt));
}
