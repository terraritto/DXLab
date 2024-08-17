#pragma once

class GlobalSettings
{
public:
	// MSAA
	bool isUseMSAA = false;
	UINT msaaSample = 8;

	// SSAA
	bool isUseSSAA = false;
	UINT ssaaScale = 2;

	// ShadowMap
	float lightDistance = 10.0f;
};
