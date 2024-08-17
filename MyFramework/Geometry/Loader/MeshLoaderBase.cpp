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
	
	m_device = device; // キャッシュ
	
	auto path = DXUTILITY::ToUTF8(fileName);

	// Importerの設定
	Assimp::Importer importer;
	int flag = 0;
	flag |= aiProcess_Triangulate; // 三角形強制
	flag |= aiProcess_PreTransformVertices;
	flag |= aiProcess_CalcTangentSpace; // tangent計算を有効
	flag |= aiProcess_GenSmoothNormals; // SmoothNormalにする
	flag |= aiProcess_GenUVCoords; // UV生成
	flag |= aiProcess_FlipUVs; // UVのy軸を反転させるように
	flag |= aiProcess_RemoveRedundantMaterials; // 参照されていないマテリアルを削除
	flag |= aiProcess_OptimizeMeshes; // メッシュ数を最適化

	// 読み込み
	auto pScene = importer.ReadFile(path, flag);
	if (pScene == nullptr) { return false; }

	// メッシュのメモリ確保
	std::vector<std::weak_ptr<DefaultMesh>> meshList;

	// メッシュ構築
	auto poolMeshList = MeshPool::GetInstance().RegisterMesh(fileName, pScene->mNumMeshes);

	// マテリアルのメモリ確保
	int materialIndex = -1;
	if (pScene->HasMaterials())
	{
		materialIndex = MaterialManager::GetInstance().GetMaterialTail();

		// マテリアルデータを変換
		for (size_t i = 0; i < pScene->mNumMaterials; i++)
		{
			const auto material = pScene->mMaterials[i];
			std::unique_ptr<DefaultMeshMaterial> dstMaterial = std::make_unique<DefaultMeshMaterial>();
			ParseMaterial(dstMaterial, material);
			MaterialManager::GetInstance().AddMaterial(std::move(dstMaterial));
		}
	}

	// メッシュデータを変換
	for (size_t i = 0; i < poolMeshList.size(); i++)
	{
		const auto mesh = pScene->mMeshes[i];
		ParseMesh(poolMeshList[i], mesh);
		poolMeshList[i]->m_materialId = materialIndex == -1 ? materialIndex : materialIndex + poolMeshList[i]->m_materialId;
		meshList.push_back(poolMeshList[i]);
	}

	// ライトの確保
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

	// クリア
	pScene = nullptr;

	// meshのセットアップ
	if (SetupMesh(device, meshList) == false)
	{
		return false;
	}

	return true;

}

// Meshのセットアップをする
bool MeshLoaderBase::SetupMesh(std::weak_ptr<DXUTILITY::Device> device, std::vector<std::weak_ptr<DefaultMesh>>& meshList)
{
	//特に弄らないのでNone
	const auto flags = D3D12_RESOURCE_FLAG_NONE;
	//GPUからのみアクセス
	const auto heapType = D3D12_HEAP_TYPE_UPLOAD;

	// strideの計算
	auto vstride = UINT(sizeof(VertexMesh));
	auto istride = UINT(sizeof(UINT));

	for (int i = 0; i < meshList.size(); i++)
	{
		if (meshList[i].expired())
		{
			// 破棄データが存在している
			return false;
		}

		auto MeshData = meshList[i].lock();

		//サイズ計算
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
		MeshData->m_vertexStride = sizeof(XMFLOAT3); // vertexをDXRではPosだけ流す

		// descriptorの生成
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
