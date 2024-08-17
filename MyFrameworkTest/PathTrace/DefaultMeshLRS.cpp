#include "DefaultMeshLRS.h"
#include "../../MyFramework/RS/RSProcessor.h"

void DefaultMeshLRS::Initialize(DXUTILITY::Device* device)
{
	const UINT regSpace = 1;

	// [0]: ディスクリプタ, t0, インデックスバッファ.
	// [1]: ディスクリプタ, t1, 頂点バッファ.
	// [2]: 定数バッファ, b0, 定数バッファ.
	RSProcessor rsProcessor;
	rsProcessor.Add(RangeType::SRV, 0, regSpace);
	rsProcessor.Add(RangeType::SRV, 1, regSpace);
	rsProcessor.Add(RangeType::SRV, 2, regSpace); // normal
	rsProcessor.Add(RangeType::SRV, 2, regSpace + 1); // tangent
	rsProcessor.Add(RangeType::SRV, 3, regSpace);
	rsProcessor.Add(RootType::CBV, 0, regSpace);
	rsProcessor.Add(RangeType::SRV, 4, regSpace);	  // diffuse map
	rsProcessor.Add(RangeType::SRV, 4, regSpace + 1); // normal map

	// 反射/屈折用
	m_localRS = rsProcessor.Create(device, true, L"DefaultMesh(LRS)");
}
