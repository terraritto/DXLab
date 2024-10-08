#ifndef __COMMON_DXR__
#define __COMMON_DXR__

// 頂点関係
struct VSInput
{
	float4 Position : POSITION;
	float4 Normal : NORMAL;
	float2 UV : TEXCOORD0;
	float4 Tangent : TANGENT;
};

// 出力関係
struct VSOutput
{
	float4 Position : SV_POSITION;
	float2 UV : TEXCOORD0;
	float4 WorldPos : WORLD_POS;
	float3x3 TangentBasis : TANGENT_BASIS;
};

struct VSShadowOutput
{
    float4 Position : SV_POSITION;
    float4 Normal : NORMAL;
    float2 UV : TEXCOORD0;
};

// scene関係
struct ShaderParameter
{
	float4x4 world;
	float4x4 view;
	float4x4 proj;
	float4 cameraPos;
	int lightIndex;
};

struct SceneShadowParameter
{
    float4x4 LVP;
    float offset;
};

// ShadowMap関係
struct ShadowParameter
{
    float4x4 world;
    float4x4 view;
    float4x4 proj;
};

// マテリアル関係
struct EncodeData
{
    float4 parameter1;
    float4 parameter2;
    float4 parameter3;
    float4 parameter4;
    int index;
};

struct MaterialParameter
{
	float4 albedo;
	float4 specular;
};

// BRDFのデータを展開
void Decode(const EncodeData Encode, inout MaterialParameter Decode)
{
    Decode.albedo.xyz = Encode.parameter1.xyz;
    Decode.albedo.w = Encode.parameter1.w;
    Decode.specular = Encode.parameter3;	
}

#endif