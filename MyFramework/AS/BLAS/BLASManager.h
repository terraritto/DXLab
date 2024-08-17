#pragma once
#include <vector>
#include <map>
#include <memory>
#include "BLASInterface.h"
#include "../../Utility/DXUtility.h"

class BLASInterface;

class BLASCommandProcessor
{
public:
	virtual void ProcessCommand(std::vector<BLASInterface*> blasArray, std::weak_ptr<DXUTILITY::Device> device) {}
};

class BLASManager
{
public:
	// BLAS�̒ǉ����s��
	void AddBLAS(std::string name, std::unique_ptr<BLASInterface> blas);
	
	// BLAS�֌W�̏�����Command�̎��s
	void Initialize();

	// �R�}���h�̐ݒ�
	void SetCommandProcessor(std::unique_ptr<BLASCommandProcessor> processor) { m_processor = std::move(processor); }

	// �f�o�C�X�̐ݒ�
	void SetDevice(std::weak_ptr<DXUTILITY::Device> device) { m_device = device; }
	
	// ��������blas���擾
	BLASInterface* GetBlas(const std::string name) const
	{
		auto blas = m_blasContainer.find(name);
		if (blas != m_blasContainer.end())
		{
			return blas->second.get();
		}

		return nullptr;
	}

	// BLAS�̃��Z�b�g
	void ResetBLAS() { m_blasContainer.clear(); }
protected:
	// blas�̃f�[�^
	std::map<std::string, std::unique_ptr<BLASInterface>> m_blasContainer;
	
	std::weak_ptr<DXUTILITY::Device> m_device;
	std::unique_ptr<BLASCommandProcessor> m_processor;
};
