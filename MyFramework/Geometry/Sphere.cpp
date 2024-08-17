#include "Sphere.h"
#include "GeometryDefinitions.h"

void Sphere::InitializeGeometry(std::weak_ptr<DXUTILITY::Device> device)
{
	//特に弄らないのでNone
	const auto flags = D3D12_RESOURCE_FLAG_NONE;
	//GPUからのみアクセス
	const auto heapType = D3D12_HEAP_TYPE_DEFAULT;

	//planeの生成
	std::vector<VertexPN> vertices;
	std::vector<UINT> indices;
	GetSphere(vertices, indices, 0.5f, 32, 48);

	//サイズ計算
	auto vstride = UINT(sizeof(VertexPN));
	auto istride = UINT(sizeof(UINT));
	auto vbSphereSize = vstride * vertices.size();
	auto ibPlaneSize = istride * indices.size();

	//ここからbufferにデータを放り込む
	m_polygonMesh->vertexBuffer = DXUTILITY::CreateBuffer(device, vbSphereSize, vertices.data(), heapType, flags, L"planeVB");
	m_polygonMesh->indexBuffer = DXUTILITY::CreateBuffer(device, ibPlaneSize, indices.data(), heapType, flags, L"planeIB");
	m_polygonMesh->vertexCount = UINT(vertices.size());
	m_polygonMesh->indexCount = UINT(indices.size());
	m_polygonMesh->vertexStride = vstride;

	//descriptorの生成もやる
	//SRVでshaderからもアクセスできるように
	//Vertex
	m_polygonMesh->descriptorVB = CreateStructuredSRV(
		device,
		m_polygonMesh->vertexBuffer,
		m_polygonMesh->vertexCount,
		0,
		vstride
	);
	//Index
	m_polygonMesh->descriptorIB = CreateStructuredSRV(
		device,
		m_polygonMesh->indexBuffer,
		m_polygonMesh->indexCount,
		0,
		istride
	);
}
