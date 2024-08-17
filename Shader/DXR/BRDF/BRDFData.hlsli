#ifndef __BRDFDATA__
#define __BRDFDATA__

#include "../../../Shader/Constant.hlsli"


struct EncodeData
{
	float4 parameter1;
	float4 parameter2;
	float4 parameter3;
	float4 parameter4;
	int index;
};

struct MeshMaterialData
{
	float4 albedo;
	float4 specular;
};

// 便利パラメータの集まり
struct UtilData
{
	float3 normal;
	float3 worldPos;
	float3 dir;
};

// BRDF計算用のデータ
struct BRDFData
{
	float3 ambient;
	float3 diffuse;
	float4 specular; // xyz: color, a: pow
	float3 reflectColor;
	float etaIn; float etaOut;
	int index;
	float alpha;
};

// BRDFのデータを展開
void Decode(const EncodeData Encode, inout BRDFData Decode)
{
	Decode.ambient = Encode.parameter1.xyz; Decode.alpha = Encode.parameter1.w;
	Decode.diffuse = Encode.parameter2.xyz * Encode.parameter2.w; // (Cd * kd)/Pi
	Decode.specular = Encode.parameter3;
	
	Decode.reflectColor = Encode.parameter4.xyz;

	Decode.etaIn = Encode.parameter4.w; Decode.etaOut = Encode.parameter4.w;
	Decode.index = Encode.index;
}

// BRDFのデータを展開
void Decode(const MeshMaterialData Encode, inout BRDFData Decode)
{
	Decode.ambient = 1.0f;
	Decode.diffuse = Encode.albedo.xyz; // Cd/Pi 
	Decode.alpha = Encode.albedo.w;
	Decode.specular = Encode.specular;
	Decode.index = 0;
}

#endif