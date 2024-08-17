#pragma once
#include <d3d12.h>
#include <DirectXMath.h>
#include "ASInterface.h"
#include "../Utility/DXUtility.h"
#include "../Utility/TypeUtility.h"

class ASBase : public ASInterface
{
public:
	// AS�̃��\�[�X
	ComPtr<ID3D12Resource> GetASResource()
	{
		return m_asResource;
	}

public:
	virtual void Initialize(std::weak_ptr<DXUTILITY::Device> device)
	{
		// device�̗p��
		m_device = device;

		PreInitializeAS();
		InitializeAS();
	}

	// shader���̓o�^(�K�v�ȏꍇ�̂�)
	virtual void SetShaderName(const wchar_t* name) {}

	// �O������w�莮��Command���s�̏ꍇ�͂�����
	virtual void MakeCommandFromList(DXUTILITY::Device::ComPtr<ID3D12GraphicsCommandList4> CommandList) {}

protected:
	virtual void PreInitializeAS()
	{
	}

	virtual void InitializeAS()
	{
		PreBuildAS();
		MakeResourceAndConstructAS();
		MakeCommand();
	}

protected:
	virtual void PreBuildAS() override {}
	virtual void MakeResourceAndConstructAS() override {}
	virtual void MakeCommand() override {}

protected:
	// Resource
	ComPtr<ID3D12Resource> m_asResource;
	ComPtr<ID3D12Resource> m_scratchResource;
	ComPtr<ID3D12Resource> m_updateResource;

	// desc(BLAS�̂݁H)
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC m_buildASDesc;
	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO m_prebuildDesc;
	
	// Device
	std::weak_ptr<DXUTILITY::Device> m_device;
};
