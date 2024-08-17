#pragma once
#include "../BLASManager.h"

class Test3DGeometryProcessor : public BLASCommandProcessor
{
public:
	virtual void ProcessCommand(std::vector<BLASInterface*> blasArray, std::weak_ptr<DXUTILITY::Device> device) override;
};