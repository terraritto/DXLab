#define NOMINMAX
#include "DefaultMesh.h"
#include <numeric>


DefaultMesh::DefaultMesh()
	: m_vertices()
	, m_indices()
	, m_MaxPos()
	, m_MinPos()
	, m_vertexCount(0)
	, m_indexCount(0)
	, m_vertexStride(0)
	, m_vertexBuffer()
	, m_indexBuffer()
	, m_descriptorVB()
	, m_descriptorIB()
	, m_vertexView()
	, m_indexView()
	, m_blas()
	, m_materialId(-1)
	, m_isClearRowData(true)
{
	float maxValue = std::numeric_limits<float>::infinity();
	float minValue = std::numeric_limits<float>::min();

	m_MaxPos.x = m_MaxPos.y = m_MaxPos.z = minValue;
	m_MinPos.x = m_MinPos.y = m_MinPos.z = maxValue;
}

DefaultMesh::DefaultMesh(bool isClearRowData)
	: DefaultMesh()
{
	m_isClearRowData = isClearRowData;
}
