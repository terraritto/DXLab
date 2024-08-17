#pragma once
#include <vector>
#include <memory>
#include "../../Utility/DXUtility.h"

class TLASInterface;

class TLASCommandProcessor
{
public:
	virtual void ProcessCommand(std::vector<std::unique_ptr<TLASInterface>> blasArray, DXUTILITY::Device* device) {}
};

class TLASManager
{
public:
	// TLAS�̒ǉ����s��
	void AddTLAS(std::unique_ptr<TLASInterface> tlas);

	// TLAS�֌W�̏�����Command�̎��s
	void Initialize();

	// �R�}���h�̐ݒ�
	void SetCommandProcessor(std::unique_ptr<TLASCommandProcessor> processor) { m_processor = std::move(processor); }

	// �f�o�C�X�̐ݒ�
	void SetDevice(DXUTILITY::Device* device) { m_device = device; }

	// BLAS�̃��Z�b�g
	void ResetTLAS() { m_blasContainer.clear(); }
protected:
	std::vector<std::unique_ptr<TLASInterface>> m_blasContainer;
	DXUTILITY::Device* m_device;
	std::unique_ptr<TLASCommandProcessor> m_processor;
};
