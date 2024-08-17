#pragma once
#include "../Utility/TypeUtility.h"
#include "../Utility/DXUtility.h"
#include "MaterialDefinitions.h"
#include <string>

class MaterialBase
{
public:
	MaterialBase() {}
	// メモリの確保を行う
	virtual void AllocateConstantBuffer(std::weak_ptr<DXUTILITY::Device> Device);
	virtual void AllocateBuffer(std::weak_ptr<DXUTILITY::Device> Device);

	// ShaderTable書き込み用
	virtual UINT WriteMaterialParamForShaderTable(uint8_t* dst, int index = 0);

	// 総メモリサイズ
	virtual const int GetTotalBufferSize() const = 0;

	// マテリアル名
	auto GetMaterialName() const { return m_materialName; }
	void SetMaterialName(std::string name) { m_materialName = name; }
	
	D3D12_GPU_DESCRIPTOR_HANDLE GetDescriptor(int index);

protected:
	// データの書き込みを担当する
	// パラメータは基底を呼び出しつつ初期化するように
	virtual void WriteData(uint8_t* mapped) = 0;

	virtual void WriteDataFromIndex(int index)
	{
		// 書き込み
		if (m_materialCB.IsInitialize())
		{
			auto constantBuffer = m_materialCB.Get(index);
			void* dst = nullptr;

			constantBuffer->Map(0, nullptr, &dst);
			if (dst)
			{
				WriteData(static_cast<uint8_t*>(dst));
				constantBuffer->Unmap(0, nullptr);
			}
		}

		// Descriptorの方を書き込み
		if (m_materialDescriptorCB.IsInitialize())
		{
			void* buffer = m_materialDescriptorCB.Map(index);
			if (buffer)
			{
				WriteData(static_cast<uint8_t*>(buffer));
				m_materialDescriptorCB.Unmap(index);
			}
		}
	}

protected:
	// マテリアルのコンスタントバッファ
	DXUTILITY::DynamicConstantBuffer m_materialCB;
	
	// Descriptor付きConstantBuffer
	DXUTILITY::DynamicBuffer m_materialDescriptorCB;

	// マテリアル名
	std::string m_materialName;

protected:
	// CPUへの書き込みのためにMaterialManagerは許可
	friend class MaterialManager;

	// Materialの内部書き込みのためAnalyticは許可
	friend class AnalyticBase;
};