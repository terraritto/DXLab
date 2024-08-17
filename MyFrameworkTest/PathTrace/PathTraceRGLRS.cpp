#include "PathTraceRGLRS.h"
#include "../../MyFramework/RS/RSProcessor.h"

void PathTraceRGLRS::Initialize(DXUTILITY::Device* device)
{
	//Ray Generation Shader用
	// [0] : ディスクリプタ, u0, レイトレーシング結果書き込み用.
	RSProcessor rsProcessor;
	rsProcessor.Add(RangeType::UAV, 0);
	m_localRS = rsProcessor.Create(device, true, L"lrsRayGen");
}
