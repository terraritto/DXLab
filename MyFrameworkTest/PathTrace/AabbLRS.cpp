#include "AabbLRS.h"
#include "../../MyFramework/RS/RSProcessor.h"

void AabbLRS::Initialize(DXUTILITY::Device* device)
{
	// [0] : �萔�o�b�t�@, b1, Gemetry�p�萔�o�b�t�@
	// [1] : �萔�o�b�t�@, b2, BRDF�p�萔�o�b�t�@
	RSProcessor rsProcessor;
	rsProcessor.Add(RootType::CBV, 0, 1);
	rsProcessor.Add(RootType::CBV, 2);

	m_localRS = rsProcessor.Create(device, true, L"lrsAABB");
}
