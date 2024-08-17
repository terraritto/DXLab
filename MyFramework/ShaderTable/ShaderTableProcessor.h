#pragma once
#include "ShaderTableInterface.h"
#include "../Utility/TypeUtility.h"
#include "../Utility/DXUtility.h"

class ShaderTableProcessor : public ShaderTableInterface
{
public:
	virtual void Initialize(std::weak_ptr<DXUTILITY::Device> device, ComPtr<ID3D12StateObject> so);

	// 初期化呼び出し用
	void InitializeDispatchRayDesc(const UINT width, const UINT height, const UINT depth);
	void InitializeResultBuffer(DXUTILITY::Descriptor* descriptor) { m_resultDescriptor = descriptor; }

	// Getter
	D3D12_DISPATCH_RAYS_DESC& GetDispatchRayDesc(const int index) { return m_dispatchRayDescList[index]; }

protected:
	virtual void CalculateShaderTableSize() override;
	virtual void SetShaderAddress() override;
	virtual void AllocateShaderData() override;

protected:
	struct ShaderData
	{
		UINT m_size;
		UINT m_recordSize;
		UINT m_region;

		void Reset()
		{
			m_size = 0;
			m_recordSize = 0;
			m_region = 0;
		}

		ShaderData() : m_size(0), m_recordSize(0), m_region(0) {}
	};

	template<class... Args>
	UINT GetTotalRegionFromShaderData(Args... args) {
		UINT sum = 0;
		for (ShaderData* i : std::initializer_list<ShaderData*>{ args... }) {
			sum += i->m_region;
		}
		return sum;
	}

	// PolygonMeshの場合の計算
	uint8_t* WriteAllShaderRecord(uint8_t* dst, UINT recordSize);
	uint8_t* WriteAllShaderRecordFromIndex(uint8_t* dst, UINT recordSize, UINT index);

protected:
	// 必要なShaderData
	std::unique_ptr<ShaderData> m_raygen;
	std::unique_ptr<ShaderData> m_miss;
	std::unique_ptr<ShaderData> m_hitGroup;

	// recordのサイズ
	UINT m_recordSize;

protected:
	// ShaderTableのデータ
	ComPtr<ID3D12Resource> m_shaderTable; // 単体使用
	DXUTILITY::DynamicBuffer m_dynamicShaderTable; // 動的用
	
	std::array<D3D12_DISPATCH_RAYS_DESC, DXUTILITY::Device::BACK_BUFFER_COUNT> m_dispatchRayDescList;

protected:
	// 外部からの設定
	// SO
	ComPtr<ID3D12StateObject> m_soResource;
	// Result
	DXUTILITY::Descriptor* m_resultDescriptor;
	// Device
	std::weak_ptr<DXUTILITY::Device> m_device;
};
