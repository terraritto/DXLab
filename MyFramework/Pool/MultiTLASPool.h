#pragma once
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include "../AS/TLAS/ProcedualMesh/MultiProcedualTLAS.h"
#include "../AS/TLAS/DefaultMesh/MultiDefaultMeshTLAS.h"
#include "../Utility/DXUtility.h"
#include "../Geometry/DefaultMesh.h"

class DefaultMeshTLAS;
class ProcedualMeshTLAS;

class MultiTLASPool
{
private:
	// destructor
	~MultiTLASPool();
	MultiTLASPool() = default;

public:
	// singleton�̂��߂̎���
	MultiTLASPool(const MultiTLASPool&) = delete;
	MultiTLASPool& operator=(const MultiTLASPool&) = delete;
	MultiTLASPool(MultiTLASPool&&) = delete;
	MultiTLASPool& operator=(MultiTLASPool&&) = delete;

	static MultiTLASPool& GetInstance()
	{
		static MultiTLASPool instance;
		return instance;
	}

	// �ŏ��̃Z�b�g�A�b�v
	void Setup();

	// Descriptor�̎擾
	DXUTILITY::Descriptor& GetDescriptor() { return m_tlasDescriptor; }

	// Size�̎擾
	int GetTLASSize() const { return m_tlasSize; }

	// TLAS�̍\�z
	void ConstructTLAS(std::weak_ptr<DXUTILITY::Device> device);

	// ��������
	void WriteShaderTable(ComPtr<ID3D12StateObjectProperties>& rtState, uint8_t* dst, UINT hitgroupRecordSize);
	void WriteShaderTableFromIndex(ComPtr<ID3D12StateObjectProperties>& rtState, uint8_t* dst, UINT hitgroupRecordSize, UINT index);

public:
	// �o�^
	void AddTLAS(std::unique_ptr<DefaultMeshTLAS>& tlas);
	void AddTLAS(std::unique_ptr<ProcedualMeshTLAS>& tlas);

protected:
	// key: filename data: mesh�̃f�[�^
	std::unordered_map<std::wstring, std::vector<std::shared_ptr<DefaultMesh>>> m_meshDataList;

	std::unique_ptr<MultiDefaultMeshTLAS> m_meshMultiTLAS;
	std::unique_ptr<MultiProcedualTLAS> m_procedualMultiTLAS;

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC m_buildASDesc;
	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO m_prebuildDesc;

	// Resource
	ComPtr<ID3D12Resource> m_asResource;
	ComPtr<ID3D12Resource> m_scratchResource;
	ComPtr<ID3D12Resource> m_updateResource;

	// descriptor
	DXUTILITY::Descriptor m_tlasDescriptor;

	int m_tlasSize = 0;

	std::array<ComPtr<ID3D12Resource>, DXUTILITY::Device::BACK_BUFFER_COUNT> m_instanceBuffers;
};
