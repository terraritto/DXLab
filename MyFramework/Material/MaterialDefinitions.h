#pragma once
#include "../Utility/TypeUtility.h"

enum class BRDFType
{
	Phong = 0,
	Reflective = 1,
	SpecularTrans = 2,
	Dielectric = 3,
	SimpleLight = 10,

	PT_Lambert = 0,
	PT_Phong = 1,
	PT_BlinnPhong = 2,
	PT_Test = 3,
};
