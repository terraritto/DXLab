#include "CommonGRS.h"
#include "../../MyFramework/RS/RSProcessor.h"

void CommonGRS::Initialize(DXUTILITY::Device* device)
{
	// [0]   : ディスクリプタ, t0, TLAS 用.
	// [0]   : サンプラー, s0, テクスチャ参照 用.
	// [0]   : 定数バッファ, b0, シーン情報用.
	// [1]   : ディスクリプタ, t1, 背景テクスチャ用(hdr)
	// [1]   : ディスクリプタ, u1, RandomIndex用
	// [2]   : ディスクリプタ, t2, Randomデータ 用
	// [2,1] : ディスクリプタ, t2(space1), Lightデータ 用

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

	// GRSを生成
	m_globalRS = rsProcessor.Create(device, false, L"RootSignatureGlobal");
}
