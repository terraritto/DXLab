#pragma once
#include <d3d12.h>
#include <DirectXMath.h>
#include <wrl.h>
#include "../ASBase.h"

class TLASInterface : public ASBase
{
public:
	D3D12_RAYTRACING_INSTANCE_DESC& GetInstanceDesc() { return m_instanceDesc; }

public:
	void SetInstanceId(const int id) { m_instanceId = id; }
	const int GetInstanceId() const { return m_instanceId; }

	void SetInstanceHGIndex(const int index) { m_instanceHGIndex = index; }
	const int GetInstanceHGIndex() const { return m_instanceHGIndex; }

	void SetTransform(const XMMATRIX transform) { m_transform = transform; }
	const XMMATRIX GetTransform() const { return m_transform; }

	void SetShaderIdentifier(const std::wstring shaderId) { m_shaderIdentifier = shaderId; }
	const std::wstring GetShaderIdentifier() const { return m_shaderIdentifier; }

	void SetMaterialIndex(int index) { m_materialIndex = index; }
	const int GetMaterialIndex() const { return m_materialIndex; }

protected:
	virtual void PreInitializeAS() override
	{
		InitializeInstanceDesc();
	}

protected:
	virtual void InitializeInstanceDesc(){}

protected:
	// desc
	D3D12_RAYTRACING_INSTANCE_DESC m_instanceDesc;

protected:
	// transform
	XMMATRIX m_transform;

	// instanceópId
	int m_instanceId;

	// instanceópHG Index
	int m_instanceHGIndex;

	// Shader Identifier(Shader TableèëÇ´çûÇ›éûÇ…óòóp)
	std::wstring m_shaderIdentifier;

	// material
	int m_materialIndex = -1;
};