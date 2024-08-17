#include "ShaderTableProcessor.h"
#include <stdexcept>
#include <algorithm>
#include <d3d12.h>
#include "../Utility/MathUtility.h"
#include "../Geometry/GeometryDefinitions.h"
#include "../Pool/MultiTLASPool.h"

void ShaderTableProcessor::Initialize(std::weak_ptr<DXUTILITY::Device> device, ComPtr<ID3D12StateObject> so)
{
	m_raygen = std::make_unique<ShaderData>(); m_raygen->Reset();
	m_miss = std::make_unique<ShaderData>(); m_miss->Reset();
	m_hitGroup = std::make_unique<ShaderData>(); m_hitGroup->Reset();

	// èâä˙âªä÷åWÇÃê›íË
	m_device = device;
	m_soResource = so;

	// ShaderTableÇÃåvéZ
	CalculateShaderTableSize();
	AllocateShaderData();
	SetShaderAddress();
}

void ShaderTableProcessor::InitializeDispatchRayDesc(const UINT width, const UINT height, const UINT depth)
{
	for (int i = 0; i < m_dispatchRayDescList.size(); i++)
	{
		m_dispatchRayDescList[i].Width = width;
		m_dispatchRayDescList[i].Height = height;
		m_dispatchRayDescList[i].Depth = depth;
	}
}

void ShaderTableProcessor::CalculateShaderTableSize()
{
	m_recordSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
	m_recordSize = Utility::RoundUp
	(
		m_recordSize,
		D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT
	);
}

void ShaderTableProcessor::SetShaderAddress()
{
}

void ShaderTableProcessor::AllocateShaderData()
{
}

uint8_t* ShaderTableProcessor::WriteAllShaderRecord(uint8_t* dst, UINT recordSize)
{
	ComPtr<ID3D12StateObjectProperties> rtsoProps;
	m_soResource.As(&rtsoProps);

	MultiTLASPool& TLASPool = MultiTLASPool::GetInstance();
	TLASPool.WriteShaderTable(rtsoProps, dst, recordSize);

	return dst;
}

uint8_t* ShaderTableProcessor::WriteAllShaderRecordFromIndex(uint8_t* dst, UINT recordSize, UINT index)
{
	ComPtr<ID3D12StateObjectProperties> rtsoProps;
	m_soResource.As(&rtsoProps);

	MultiTLASPool& TLASPool = MultiTLASPool::GetInstance();
	TLASPool.WriteShaderTableFromIndex(rtsoProps, dst, recordSize, index);

	return dst;
}
