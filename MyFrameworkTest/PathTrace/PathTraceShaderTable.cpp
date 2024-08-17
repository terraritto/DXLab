#include "PathTraceShaderTable.h"
#include "../../MyFramework/Utility/MathUtility.h"
#include "../../MyFramework/Pool/MultiTLASPool.h"
#include <stdexcept>

void PathTraceShaderTable::CalculateShaderTableSize()
{
	auto recordAlign = D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT;
	MultiTLASPool& TLASPool = MultiTLASPool::GetInstance();

	// record Sizeの計算
	// RayGeneration シェーダーでは、 Shader Identifier と
	// ローカルルートシグネチャによる u0 のディスクリプタを使用
	m_raygen->m_recordSize += D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
	m_raygen->m_recordSize += sizeof(D3D12_GPU_DESCRIPTOR_HANDLE);
	m_raygen->m_recordSize = Utility::RoundUp(m_raygen->m_recordSize, recordAlign);

	// ヒットグループでは、 Shader Identifier の他に
	// ローカルルートシグネチャによる VB/IB のディスクリプタを使用.
	// Mesh 1:IB 2:pos 3: normal 4:uv 5:Material 6:texture
	// Procedual 1: Material 2: Analytic
	m_hitGroup->m_recordSize += D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
	m_hitGroup->m_recordSize += sizeof(D3D12_GPU_DESCRIPTOR_HANDLE) * 6; // Descriptor x 6
	m_hitGroup->m_recordSize = Utility::RoundUp(m_hitGroup->m_recordSize, recordAlign);

	// Missシェーダーではローカルルートシグネチャ未使用
	m_miss->m_recordSize += D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
	m_miss->m_recordSize = Utility::RoundUp(m_miss->m_recordSize, recordAlign);

	// 使用する各シェーダーの個数より、シェーダーテーブルのサイズを求める.
	//  RayGen : 1
	//  Miss : 2
	//  HitGroup: 3 (sphere x 100 + Triangle)
	const int hitgroupNum = TLASPool.GetTLASSize();
	m_raygen->m_size = m_raygen->m_recordSize;
	m_miss->m_size = m_miss->m_recordSize * 2;
	m_hitGroup->m_size = m_hitGroup->m_recordSize * hitgroupNum;

	// Tableのalign
	auto tableAlign = D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT;
	m_raygen->m_region = Utility::RoundUp(m_raygen->m_size, tableAlign);
	m_miss->m_region = Utility::RoundUp(m_miss->m_size, tableAlign);
	m_hitGroup->m_region = Utility::RoundUp(m_hitGroup->m_size, tableAlign);

	// shader table
	auto tableSize = GetTotalRegionFromShaderData(m_raygen.get(), m_miss.get(), m_hitGroup.get());
	m_dynamicShaderTable.Initialize(m_device, tableSize, L"ShaderTable");
}

void PathTraceShaderTable::SetShaderAddress()
{
	for (int i = 0; i < m_dispatchRayDescList.size(); i++)
	{
		auto startAddress = m_dynamicShaderTable.Get(i)->GetGPUVirtualAddress();
		auto& desc = m_dispatchRayDescList[i];

		auto& shaderRecordRG = desc.RayGenerationShaderRecord;
		shaderRecordRG.StartAddress = startAddress;
		shaderRecordRG.SizeInBytes = m_raygen->m_size;
		startAddress += m_raygen->m_region;

		auto& shaderRecordMS = desc.MissShaderTable;
		shaderRecordMS.StartAddress = startAddress;
		shaderRecordMS.SizeInBytes = m_miss->m_size;
		shaderRecordMS.StrideInBytes = m_miss->m_recordSize;
		startAddress += m_miss->m_region;

		auto& shaderRecordHG = desc.HitGroupTable;
		shaderRecordHG.StartAddress = startAddress;
		shaderRecordHG.SizeInBytes = m_hitGroup->m_size;
		shaderRecordHG.StrideInBytes = m_hitGroup->m_recordSize;
	}
}

void PathTraceShaderTable::AllocateShaderData()
{
	ComPtr<ID3D12StateObjectProperties> rtsoProps;
	m_soResource.As(&rtsoProps);

	// shader recordの書き込み初期位置
	for (int i = 0; i < m_dispatchRayDescList.size(); i++)
	{
		void* mapped = m_dynamicShaderTable.Map(i);
		uint8_t* pStart = static_cast<uint8_t*>(mapped);

		// Ray Generation
		auto rgsStart = pStart;
		{
			uint8_t* p = rgsStart;
			auto id = rtsoProps->GetShaderIdentifier(L"mainRayGen");
			if (id == nullptr)
			{
				throw std::logic_error("Not found Shader Identifier.");
			}
			p += DXUTILITY::WriteShaderIdentifier(p, id);

			// ローカルルートシグネチャで u0 (出力先) を設定しているため
			// 対応するディスクリプタを書き込む.
			p += DXUTILITY::WriteGPUDescriptor(p, *m_resultDescriptor);
		}

		// miss shader
		auto missStart = pStart + m_raygen->m_region;
		{
			auto recordStart = missStart;
			uint8_t* p = missStart;
			auto id = rtsoProps->GetShaderIdentifier(L"mainMiss");
			if (id == nullptr)
			{
				throw std::logic_error("Not found Shader Identifier.");
			}
			p += DXUTILITY::WriteShaderIdentifier(p, id);

			// 次の位置へ
			recordStart += m_miss->m_recordSize;

			// shadowを判定
			id = rtsoProps->GetShaderIdentifier(L"shadowMiss");
			if (id == nullptr)
			{
				throw std::logic_error("Not found Shader Identifier.");
			}
			p += DXUTILITY::WriteShaderIdentifier(p, id);
		}

		// hitgroup shader
		auto hitgroupStart = pStart + m_raygen->m_region + m_miss->m_region;
		{
			uint8_t* p = hitgroupStart;

			p = WriteAllShaderRecordFromIndex(p, m_hitGroup->m_recordSize, i);
		}

		m_dynamicShaderTable.Unmap(i);
	}
}
