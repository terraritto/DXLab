#pragma once
#include "GeometryContainer.h"

class Plane : public GeometryContainer
{
public:
	virtual void InitializeGeometry(std::weak_ptr<DXUTILITY::Device> device) override;
};