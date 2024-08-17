#include "PathTraceRGLRS.h"
#include "../../MyFramework/RS/RSProcessor.h"

void PathTraceRGLRS::Initialize(DXUTILITY::Device* device)
{
	//Ray Generation Shader�p
	// [0] : �f�B�X�N���v�^, u0, ���C�g���[�V���O���ʏ������ݗp.
	RSProcessor rsProcessor;
	rsProcessor.Add(RangeType::UAV, 0);
	m_localRS = rsProcessor.Create(device, true, L"lrsRayGen");
}
