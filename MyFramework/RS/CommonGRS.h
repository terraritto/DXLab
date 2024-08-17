#pragma once
#include "../../MyFramework/RS/GRS/GlobalRS.h"

class CommonGRS : public GlobalRS
{
public:
	virtual void Initialize(DXUTILITY::Device* device) override;
};