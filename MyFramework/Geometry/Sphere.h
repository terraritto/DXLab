#pragma once
#include "GeometryContainer.h"

class Sphere : public GeometryContainer
{
public:
	virtual void InitializeGeometry(std::weak_ptr<DXUTILITY::Device> device) override;
};