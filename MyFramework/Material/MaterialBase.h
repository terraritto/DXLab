#pragma once
#include "../Utility/TypeUtility.h"
#include "../Utility/DXUtility.h"
#include "MaterialDefinitions.h"
#include <string>

class MaterialBase
{
public:
	MaterialBase() {}
	// �������̊m�ۂ��s��
	virtual void AllocateConstantBuffer(std::weak_ptr<DXUTILITY::Device> Device);
	virtual void AllocateBuffer(std::weak_ptr<DXUTILITY::Device> Device);

	// ShaderTable�������ݗp
	virtual UINT WriteMaterialParamForShaderTable(uint8_t* dst, int index = 0);

	// ���������T�C�Y
	virtual const int GetTotalBufferSize() const = 0;

	// �}�e���A����
	auto GetMaterialName() const { return m_materialName; }
	void SetMaterialName(std::string name) { m_materialName = name; }
	
	D3D12_GPU_DESCRIPTOR_HANDLE GetDescriptor(int index);

protected:
	// �f�[�^�̏������݂�S������
	// �p�����[�^�͊����Ăяo������������悤��
	virtual void WriteData(uint8_t* mapped) = 0;

	virtual void WriteDataFromIndex(int index)
	{
		// ��������
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

		// Descriptor�̕�����������
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
	// �}�e���A���̃R���X�^���g�o�b�t�@
	DXUTILITY::DynamicConstantBuffer m_materialCB;
	
	// Descriptor�t��ConstantBuffer
	DXUTILITY::DynamicBuffer m_materialDescriptorCB;

	// �}�e���A����
	std::string m_materialName;

protected:
	// CPU�ւ̏������݂̂��߂�MaterialManager�͋���
	friend class MaterialManager;

	// Material�̓����������݂̂���Analytic�͋���
	friend class AnalyticBase;
};