#include "GeometryContainer.h"

GeometryContainer::GeometryContainer()
	: m_polygonMesh(std::make_unique<PolygonMesh>())
{
}
