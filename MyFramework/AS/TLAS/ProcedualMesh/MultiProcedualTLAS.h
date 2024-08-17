#pragma once
#include <vector>
#include <array>
#include <memory>
#include <stdexcept>
#include "../MultiTLASInterface.h"
#include "ProcedualMeshTLAS.h"
#include "../../../Utility/DXUtility.h"
#include "../../../Geometry/GeometryDefinitions.h"

class MaterialManager;

class MultiProcedualTLAS : public MultiTLASInterface
{
public:
	void AddTLAS(std::unique_ptr<ProcedualMeshTLAS>& tlas);

	// ShaderTableÇ÷ÇÃèëÇ´çûÇ›óp
	virtual void WriteShaderTable(ComPtr<ID3D12StateObjectProperties>& rtState, uint8_t* dst, UINT hitgroupRecordSize) override;
	virtual void WriteShaderTableFromIndex(ComPtr<ID3D12StateObjectProperties>& rtState, uint8_t* dst, UINT hitgroupRecordSize, UINT index) override;

protected:
	virtual void InitializeInstanceDesc() override;
	virtual void PreBuildAS() override;
	virtual void MakeResourceAndConstructAS() override;
	virtual void MakeCommand() override;

protected:
	std::vector<std::unique_ptr<ProcedualMeshTLAS>> m_procedualList;
};
