#pragma once
#include "../../MyFramework/ShaderTable/ShaderTableProcessor.h"

class PathTraceShaderTable : public ShaderTableProcessor
{
protected:
	virtual void CalculateShaderTableSize() override;
	virtual void SetShaderAddress() override;
	virtual void AllocateShaderData() override;
};
