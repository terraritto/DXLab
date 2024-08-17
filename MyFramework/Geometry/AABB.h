#pragma once
#include "ProcedualContainer.h"

class AABB : public ProcedualContainer
{
public:
	virtual void InitializeGeometry(std::weak_ptr<DXUTILITY::Device> device) override;
};
