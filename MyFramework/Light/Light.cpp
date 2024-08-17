#include "Light.h"

int ConvertType(aiLightSourceType type)
{
    switch (type)
    {
    case aiLightSource_POINT:
        return 0;

    case aiLightSource_DIRECTIONAL:
        return 1;

    case aiLightSource_SPOT:
        return 2;

    default:
        break;
    }

    return -1;
}

std::string ConvertLightTypeString(Light& light)
{
    std::string name;
    switch (light.m_type)
    {
    case 0:
        name = "point";
        break;

    case 1:
        name = "directional";
        break;

    case 2:
        name = "spot";
        break;

    default:
        name = "none";
        break;
    }
    return name;
}
