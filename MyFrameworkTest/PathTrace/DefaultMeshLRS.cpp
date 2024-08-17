#include "DefaultMeshLRS.h"
#include "../../MyFramework/RS/RSProcessor.h"

void DefaultMeshLRS::Initialize(DXUTILITY::Device* device)
{
	const UINT regSpace = 1;

	// [0]: �f�B�X�N���v�^, t0, �C���f�b�N�X�o�b�t�@.
	// [1]: �f�B�X�N���v�^, t1, ���_�o�b�t�@.
	// [2]: �萔�o�b�t�@, b0, �萔�o�b�t�@.
	RSProcessor rsProcessor;
	rsProcessor.Add(RangeType::SRV, 0, regSpace);
	rsProcessor.Add(RangeType::SRV, 1, regSpace);
	rsProcessor.Add(RangeType::SRV, 2, regSpace); // normal
	rsProcessor.Add(RangeType::SRV, 2, regSpace + 1); // tangent
	rsProcessor.Add(RangeType::SRV, 3, regSpace);
	rsProcessor.Add(RootType::CBV, 0, regSpace);
	rsProcessor.Add(RangeType::SRV, 4, regSpace);	  // diffuse map
	rsProcessor.Add(RangeType::SRV, 4, regSpace + 1); // normal map

	// ����/���ܗp
	m_localRS = rsProcessor.Create(device, true, L"DefaultMesh(LRS)");
}
