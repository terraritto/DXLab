#pragma once
#include "../../MyFramework/RS/LRS/LocalRS.h"

class PathTraceRGLRS : public LocalRS
{
public:
	virtual void Initialize(DXUTILITY::Device* device) override;
};
