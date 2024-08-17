#pragma once

class ShaderTableInterface
{
protected:
	virtual void CalculateShaderTableSize() = 0;
	virtual void AllocateShaderData() = 0;
	virtual void SetShaderAddress() = 0;
};
