#pragma once
#include "../../MyFramework/RS/LRS/LocalRS.h"

class AabbLRS : public LocalRS
{
public:
	virtual void Initialize(DXUTILITY::Device* device) override;
};
