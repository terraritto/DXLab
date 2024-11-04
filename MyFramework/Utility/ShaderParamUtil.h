#pragma once
#include "TypeUtility.h"

// CommonŠÖŒW
struct ShaderParameters
{
	XMFLOAT4X4 mtxWorld;
	XMFLOAT4X4 mtxView;
	XMFLOAT4X4 mtxProj;
	XMVECTOR cameraParam;
	int lightIndex;
};

// Shadow—p
struct SceneShadowParameters
{
	XMFLOAT4X4 LVP;
	float offset = 0.1f;
};

struct ShadowParameters
{
	XMFLOAT4X4 mtxWorld;
	XMFLOAT4X4 mtxView;
	XMFLOAT4X4 mtxProj;
};

// “ÁŽê—p
// CommonŠÖŒW
struct ShaderHemisphereParameters
{
	XMFLOAT4X4 mtxWorld;
	XMFLOAT4X4 mtxView;
	XMFLOAT4X4 mtxProj;
	XMVECTOR cameraParam;
	XMVECTOR skyColor;
	XMVECTOR groundColor;
	XMVECTOR groundNormal;
	int lightIndex;
};
