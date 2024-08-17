#include "MeshLoaderBase.h"
#include "../../Utility/DXUtility.h"
#include "../../Pool/MeshPool.h"
#include "../../Pool/LightPool.h"
#include "../../Material/MaterialManager.h"

bool MeshLoaderBase::LoadMesh(std::weak_ptr<DXUTILITY::Device> device, const std::wstring& fileName)
{
	if (fileName.empty()) { return false; }

	auto deviceData = device.lock();
	if (deviceData == nullptr) { return false; }
	
	m_device = device; // �L���b�V��
	
	auto path = DXUTILITY::ToUTF8(fileName);

	// Importer�̐ݒ�
	Assimp::Importer importer;
	int flag = 0;
	flag |= aiProcess_Triangulate; // �O�p�`����
	flag |= aiProcess_PreTransformVertices;
	flag |= aiProcess_CalcTangentSpace; // tangent�v�Z��L��
	flag |= aiProcess_GenSmoothNormals; // SmoothNormal�ɂ���
	flag |= aiProcess_GenUVCoords; // UV����
	flag |= aiProcess_FlipUVs; // UV��y���𔽓]������悤��
	flag |= aiProcess_RemoveRedundantMaterials; // �Q�Ƃ���Ă��Ȃ��}�e���A�����폜
	flag |= aiProcess_OptimizeMeshes; // ���b�V�������œK��

	// �ǂݍ���
	auto pScene = importer.ReadFile(path, flag);
	if (pScene == nullptr) { return false; }

	// ���b�V���̃������m��
	std::vector<std::weak_ptr<DefaultMesh>> meshList;

	// ���b�V���\�z
	auto poolMeshList = MeshPool::GetInstance().RegisterMesh(fileName, pScene->mNumMeshes);

	// �}�e���A���̃������m��
	int materialIndex = -1;
	if (pScene->HasMaterials())
	{
		materialIndex = MaterialManager::GetInstance().GetMaterialTail();

		// �}�e���A���f�[�^��ϊ�
		for (size_t i = 0; i < pScene->mNumMaterials; i++)
		{
			const auto material = pScene->mMaterials[i];
			std::unique_ptr<DefaultMeshMaterial> dstMaterial = std::make_unique<DefaultMeshMaterial>();
			ParseMaterial(dstMaterial, material);
			MaterialManager::GetInstance().AddMaterial(std::move(dstMaterial));
		}
	}

	// ���b�V���f�[�^��ϊ�
	for (size_t i = 0; i < poolMeshList.size(); i++)
	{
		const auto mesh = pScene->mMeshes[i];
		ParseMesh(poolMeshList[i], mesh);
		poolMeshList[i]->m_materialId = materialIndex == -1 ? materialIndex : materialIndex + poolMeshList[i]->m_materialId;
		meshList.push_back(poolMeshList[i]);
	}

	// ���C�g�̊m��
	if (pScene->HasLights())
	{
		for (size_t i = 0; i < pScene->mNumLights; i++)
		{
			const auto light = pScene->mLights[i];
			std::unique_ptr<Light> dstLight = std::make_unique<Light>();
			ParseLight(dstLight, light);
			LightPool::GetInstance().RegisterLight(std::move(dstLight));
		}
	}

	// �N���A
	pScene = nullptr;

	// mesh�̃Z�b�g�A�b�v
	if (SetupMesh(device, meshList) == false)
	{
		return false;
	}

	return true;

}

// Mesh�̃Z�b�g�A�b�v������
bool MeshLoaderBase::SetupMesh(std::weak_ptr<DXUTILITY::Device> device, std::vector<std::weak_ptr<DefaultMesh>>& meshList)
{
	//���ɘM��Ȃ��̂�None
	const auto flags = D3D12_RESOURCE_FLAG_NONE;
	//GPU����̂݃A�N�Z�X
	const auto heapType = D3D12_HEAP_TYPE_UPLOAD;

	// stride�̌v�Z
	auto vstride = UINT(sizeof(VertexMesh));
	auto istride = UINT(sizeof(UINT));

	for (int i = 0; i < meshList.size(); i++)
	{
		if (meshList[i].expired())
		{
			// �j���f�[�^�����݂��Ă���
			return false;
		}

		auto MeshData = meshList[i].lock();

		//�T�C�Y�v�Z
		auto vbSize = vstride * MeshData->m_vertices.size();
		auto ibSize = istride * MeshData->m_indices.size();

		//vertex
		MeshData->m_vertexBuffer = DXUTILITY::CreateBuffer(device, vbSize, MeshData->m_vertices.data(), heapType, flags);
		MeshData->m_vertexCount = UINT(MeshData->m_vertices.size());
		D3D12_VERTEX_BUFFER_VIEW vbView;
		vbView.BufferLocation = MeshData->m_vertexBuffer->GetGPUVirtualAddress();
		vbView.SizeInBytes = vbSize;
		vbView.StrideInBytes = vstride;
		MeshData->m_vertexView = vbView;
		MeshData->m_vertexStride = vstride;

		// index
		MeshData->m_indexBuffer = DXUTILITY::CreateBuffer(device, ibSize, MeshData->m_indices.data(), heapType, flags);
		MeshData->m_indexCount = UINT(MeshData->m_indices.size());
		D3D12_INDEX_BUFFER_VIEW ibView;
		ibView.BufferLocation = MeshData->m_indexBuffer->GetGPUVirtualAddress();
		ibView.Format = DXGI_FORMAT_R32_UINT;
		ibView.SizeInBytes = ibSize;
		MeshData->m_indexView = ibView;

		// for DXR
		auto vbSizeF3 = UINT(sizeof(XMFLOAT3)) * MeshData->m_vertices.size();
		auto vbSizeF2 = UINT(sizeof(XMFLOAT2)) * MeshData->m_vertices.size();
		MeshData->m_positionBuffer = DXUTILITY::CreateBuffer(device, vbSizeF3, MeshData->m_positions.data(), heapType, flags);
		MeshData->m_normalBuffer = DXUTILITY::CreateBuffer(device, vbSizeF3, MeshData->m_normals.data(), heapType, flags);
		MeshData->m_tangentBuffer = DXUTILITY::CreateBuffer(device, vbSizeF3, MeshData->m_tangents.data(), heapType, flags);
		MeshData->m_uvBuffer = DXUTILITY::CreateBuffer(device, vbSizeF2, MeshData->m_uvs.data(), heapType, flags);
		MeshData->m_vertexStride = sizeof(XMFLOAT3); // vertex��DXR�ł�Pos��������

		// descriptor�̐���
		// Vertex
		MeshData->m_descriptorVB = DXUTILITY::CreateStructuredSRV(
			device,
			MeshData->m_vertexBuffer,
			MeshData->m_vertexCount,
			0,
			vstride
		);

		// Index
		MeshData->m_descriptorIB = DXUTILITY::CreateStructuredSRV(
			device,
			MeshData->m_indexBuffer,
			MeshData->m_indexCount,
			0,
			istride
		);

		// Pos
		MeshData->m_descriptorPosition = DXUTILITY::CreateStructuredSRV(
			device,
			MeshData->m_positionBuffer,
			MeshData->m_vertexCount,
			0,
			sizeof(XMFLOAT3)
		);

		// Normal
		MeshData->m_descriptorNormal = DXUTILITY::CreateStructuredSRV(
			device,
			MeshData->m_normalBuffer,
			MeshData->m_vertexCount,
			0,
			sizeof(XMFLOAT3)
		);

		// Normal
		MeshData->m_descriptorTangent = DXUTILITY::CreateStructuredSRV(
			device,
			MeshData->m_tangentBuffer,
			MeshData->m_vertexCount,
			0,
			sizeof(XMFLOAT3)
		);

		// Pos
		MeshData->m_descriptorUv = DXUTILITY::CreateStructuredSRV(
			device,
			MeshData->m_uvBuffer,
			MeshData->m_vertexCount,
			0,
			sizeof(XMFLOAT2)
		);

		if (MeshData->m_isClearRowData)
		{
			MeshData->m_vertices.clear();
			MeshData->m_indices.clear();
			MeshData->m_positions.clear();
			MeshData->m_normals.clear();
			MeshData->m_uvs.clear();
		}
	}

	return true;
}
