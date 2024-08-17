#pragma once
#include <d3d12.h>
#include <DirectXMath.h>
#include <wrl.h>
#include "TLASInterface.h"

class MultiTLASInterface : public TLASInterface
{
public:
	// ShaderTableÇ÷ÇÃèëÇ´çûÇ›óp
	virtual void WriteShaderTable(ComPtr<ID3D12StateObjectProperties>& rtState, uint8_t* dst, UINT hitgroupRecordSize) = 0;
	virtual void WriteShaderTableFromIndex(ComPtr<ID3D12StateObjectProperties>& rtState, uint8_t* dst, UINT hitgroupRecordSize, UINT index) {};

	std::vector<D3D12_RAYTRACING_INSTANCE_DESC> GetInstanceDescList() { return m_instanceDescList; }

protected:
	std::vector<D3D12_RAYTRACING_INSTANCE_DESC> m_instanceDescList;

	friend class MultiTLASPool;
};