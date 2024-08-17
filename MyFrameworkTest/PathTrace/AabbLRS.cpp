#include "AabbLRS.h"
#include "../../MyFramework/RS/RSProcessor.h"

void AabbLRS::Initialize(DXUTILITY::Device* device)
{
	// [0] : 定数バッファ, b1, Gemetry用定数バッファ
	// [1] : 定数バッファ, b2, BRDF用定数バッファ
	RSProcessor rsProcessor;
	rsProcessor.Add(RootType::CBV, 0, 1);
	rsProcessor.Add(RootType::CBV, 2);

	m_localRS = rsProcessor.Create(device, true, L"lrsAABB");
}
