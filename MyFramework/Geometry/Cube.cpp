#include "Cube.h"
#include "GeometryDefinitions.h"

void Cube::InitializeGeometry(std::weak_ptr<DXUTILITY::Device> device)
{
	//���ɘM��Ȃ��̂�None
	const auto flags = D3D12_RESOURCE_FLAG_NONE;
	//GPU����̂݃A�N�Z�X
	const auto heapType = D3D12_HEAP_TYPE_DEFAULT;

	//Cube�̐���
	std::vector<VertexPNC> vertices;
	std::vector<UINT> indices;
	GetColoredCube(vertices, indices);

	//�T�C�Y�v�Z
	auto vstride = UINT(sizeof(VertexPNC));
	auto istride = UINT(sizeof(UINT));
	auto vbCubeSize = vstride * vertices.size();
	auto ibCubeSize = istride * indices.size();

	//��������buffer�Ƀf�[�^����荞��
	m_polygonMesh->vertexBuffer = DXUTILITY::CreateBuffer(device, vbCubeSize, vertices.data(), heapType, flags, L"cubeVB");
	m_polygonMesh->indexBuffer = DXUTILITY::CreateBuffer(device, ibCubeSize, indices.data(), heapType, flags, L"cubeIB");
	m_polygonMesh->vertexCount = UINT(vertices.size());
	m_polygonMesh->indexCount = UINT(indices.size());
	m_polygonMesh->vertexStride = vstride;

	//descriptor�̐��������
	//SRV��shader������A�N�Z�X�ł���悤��
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
