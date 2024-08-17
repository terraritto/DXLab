#pragma once
#include "ShaderTableInterface.h"
#include "../Utility/TypeUtility.h"
#include "../Utility/DXUtility.h"

class ShaderTableProcessor : public ShaderTableInterface
{
public:
	virtual void Initialize(std::weak_ptr<DXUTILITY::Device> device, ComPtr<ID3D12StateObject> so);

	// �������Ăяo���p
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

	// PolygonMesh�̏ꍇ�̌v�Z
	uint8_t* WriteAllShaderRecord(uint8_t* dst, UINT recordSize);
	uint8_t* WriteAllShaderRecordFromIndex(uint8_t* dst, UINT recordSize, UINT index);

protected:
	// �K�v��ShaderData
	std::unique_ptr<ShaderData> m_raygen;
	std::unique_ptr<ShaderData> m_miss;
	std::unique_ptr<ShaderData> m_hitGroup;

	// record�̃T�C�Y
	UINT m_recordSize;

protected:
	// ShaderTable�̃f�[�^
	ComPtr<ID3D12Resource> m_shaderTable; // �P�̎g�p
	DXUTILITY::DynamicBuffer m_dynamicShaderTable; // ���I�p
	
	std::array<D3D12_DISPATCH_RAYS_DESC, DXUTILITY::Device::BACK_BUFFER_COUNT> m_dispatchRayDescList;

protected:
	// �O������̐ݒ�
	// SO
	ComPtr<ID3D12StateObject> m_soResource;
	// Result
	DXUTILITY::Descriptor* m_resultDescriptor;
	// Device
	std::weak_ptr<DXUTILITY::Device> m_device;
};
