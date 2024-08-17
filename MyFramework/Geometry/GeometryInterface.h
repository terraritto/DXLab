#pragma once
#include "../Utility/DXUtility.h"

class GeometryInterface
{
public:
	virtual void InitializeGeometry(std::weak_ptr<DXUTILITY::Device> device) = 0;
};
