#include "CommonGRS.h"
#include "../../MyFramework/RS/RSProcessor.h"

void CommonGRS::Initialize(DXUTILITY::Device* device)
{
	// [0]   : �f�B�X�N���v�^, t0, TLAS �p.
	// [0]   : �T���v���[, s0, �e�N�X�`���Q�� �p.
	// [0]   : �萔�o�b�t�@, b0, �V�[�����p.
	// [1]   : �f�B�X�N���v�^, t1, �w�i�e�N�X�`���p(hdr)
	// [1]   : �f�B�X�N���v�^, u1, RandomIndex�p
	// [2]   : �f�B�X�N���v�^, t2, Random�f�[�^ �p
	// [2,1] : �f�B�X�N���v�^, t2(space1), Light�f�[�^ �p

	RSProcessor rsProcessor;
	rsProcessor.Add(RangeType::SRV, 0);
	rsProcessor.Add(RootType::CBV, 0);
	rsProcessor.Add(RangeType::CBV, 1);
	rsProcessor.Add(RangeType::CBV, 1, 1);
	rsProcessor.Add(RangeType::SRV, 1);
	rsProcessor.Add(RangeType::SRV, 2);
	rsProcessor.Add(RangeType::UAV, 1);

	// static sampler
	rsProcessor.AddStaticSampler(0);

	// GRS�𐶐�
	m_globalRS = rsProcessor.Create(device, false, L"RootSignatureGlobal");
}
