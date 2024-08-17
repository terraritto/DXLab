#include "DefaultMeshLoader.h"
#include <random>

void DefaultMeshLoader::ParseMesh(std::weak_ptr<DefaultMesh> dstMesh, const aiMesh* srcMesh)
{
	auto mesh = dstMesh.lock();
	if (mesh == nullptr) { return; }

	mesh->m_name = srcMesh->mName.C_Str();

	// マテリアルIDの設定
	mesh->m_materialId = srcMesh->mMaterialIndex;

	aiVector3D zero3D(0.0f, 0.0f, 0.0f);

	// 領域確保
	mesh->m_vertices.resize(srcMesh->mNumVertices);
	mesh->m_positions.resize(srcMesh->mNumVertices);
	mesh->m_normals.resize(srcMesh->mNumVertices);
	mesh->m_uvs.resize(srcMesh->mNumVertices);
	mesh->m_tangents.resize(srcMesh->mNumVertices);
	mesh->m_indices.resize(srcMesh->mNumFaces * 3);

	// Vertexのデータを入れていく
	for (auto i = 0u; i < srcMesh->mNumVertices; i++)
	{
		auto pos = &(srcMesh->mVertices[i]);
		auto normal = &(srcMesh->mNormals[i]);
		auto uv = (srcMesh->HasTextureCoords(0)) ? &(srcMesh->mTextureCoords[0][i]) : &zero3D;
		auto tangent = (srcMesh->HasTangentsAndBitangents()) ? &(srcMesh->mTangents[i]) : &zero3D;

		mesh->m_vertices[i] = VertexMesh
		(
			XMFLOAT3(pos->x, pos->y, pos->z),
			XMFLOAT3(normal->x, normal->y, normal->z),
			XMFLOAT2(uv->x, uv->y),
			XMFLOAT3(tangent->x, tangent->y, tangent->z)
		);

		mesh->m_positions[i] = XMFLOAT3(pos->x, pos->y, pos->z);
		mesh->m_normals[i] = XMFLOAT3(normal->x, normal->y, normal->z);
		mesh->m_uvs[i] = XMFLOAT2(uv->x, uv->y);
		mesh->m_tangents[i] = XMFLOAT3(tangent->x, tangent->y, tangent->z);

		auto UpdateMinValue = [](float& target, const float& value)
			{
				if (value <= target)
				{
					target = value;
				}
			};

		auto UpdateMaxValue = [](float& target, const float& value)
			{
				if (target <= value)
				{
					target = value;
				}
			};

		UpdateMinValue(mesh->m_MinPos.x, pos->x);
		UpdateMinValue(mesh->m_MinPos.y, pos->y);
		UpdateMinValue(mesh->m_MinPos.z, pos->z);

		UpdateMaxValue(mesh->m_MaxPos.x, pos->x);
		UpdateMaxValue(mesh->m_MaxPos.y, pos->y);
		UpdateMaxValue(mesh->m_MaxPos.z, pos->z);
	}

	// indexのデータを入れていく
	for (auto i = 0u; i < srcMesh->mNumFaces; i++)
	{
		const auto& face = srcMesh->mFaces[i];
		assert(face.mNumIndices == 3); // 三角形のはず

		mesh->m_indices[i * 3 + 0] = face.mIndices[0];
		mesh->m_indices[i * 3 + 1] = face.mIndices[1];
		mesh->m_indices[i * 3 + 2] = face.mIndices[2];
	}
}

void DefaultMeshLoader::ParseMaterial(std::unique_ptr<DefaultMeshMaterial>& dstMaterial, const aiMaterial* srcMaterial)
{
	if (dstMaterial == nullptr)
	{
		return;
	}

	aiVector3D color(0.0f, 0.0f, 0.0f);

	// diffuse
	if (srcMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS)
	{
		dstMaterial->SetAlbedo(DirectX::XMVectorSet(color.x, color.y, color.z, 1.0f));
	}

	// specular
	if (srcMaterial->Get(AI_MATKEY_COLOR_SPECULAR, color) == AI_SUCCESS)
	{
		dstMaterial->SetSpecular(DirectX::XMVectorSet(color.x, color.y, color.z, 0.0f));
	}

	// shininess
	float shininess = 0.0f;
	if (srcMaterial->Get(AI_MATKEY_SHININESS, shininess) == AI_SUCCESS)
	{
		dstMaterial->SetSpecularPow(std::max<float>(shininess, 1.0f));
	}

	// opacity
	float opacity = 1.0f;
	if (srcMaterial->Get(AI_MATKEY_OPACITY, opacity) == AI_SUCCESS)
	{
		dstMaterial->SetAlpha(opacity);
	}

	// diffuse map
	aiString path;
	aiTextureOp op;
	aiTextureMapMode MapMode;
	unsigned int uvindex;
	
	// color
	if (srcMaterial->Get(AI_MATKEY_TEXTURE_DIFFUSE(0), path) == AI_SUCCESS)
	{
		dstMaterial->SetColorTexture(DXUTILITY::ToWstring(path.C_Str()));
	}

	// diffuse
	if (srcMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &path, nullptr, &uvindex, nullptr, &op, &MapMode) == AI_SUCCESS)
	{
		// g: roughness b: metallic
		dstMaterial->SetDiffuseTexture(DXUTILITY::ToWstring(path.C_Str()));
	}

	// normal
	//if (srcMaterial->Get(AI_MATKEY_TEXTURE_NORMALS(0), path) == AI_SUCCESS)
	if(srcMaterial->GetTexture(aiTextureType_HEIGHT, 0, &path) == AI_SUCCESS)
	{
		dstMaterial->SetNormalTexture(DXUTILITY::ToWstring(path.C_Str()));
	}
}

void DefaultMeshLoader::ParseLight(std::unique_ptr<Light>& dstLight, const aiLight* srcLight)
{
	dstLight->m_type = ConvertType(srcLight->mType);
	dstLight->m_name = srcLight->mName.C_Str();

	std::mt19937 mt(rand());
	std::uniform_real_distribution<> randDistribution(0.0, 1.0);

	dstLight->m_color = DirectX::XMVectorSet(randDistribution(mt), randDistribution(mt), randDistribution(mt), 1.0f);

	switch (srcLight->mType)
	{
	case aiLightSource_POINT:
		dstLight->m_position = DirectX::XMVectorSet(srcLight->mPosition.x, srcLight->mPosition.y, srcLight->mPosition.z, 1.0f);
		break;

	case aiLightSource_DIRECTIONAL:
		dstLight->m_direction = DirectX::XMVectorSet(srcLight->mDirection.x, srcLight->mDirection.y, srcLight->mDirection.z, 1.0f);
		dstLight->m_color = DirectX::g_XMOne;
		break;

	case aiLightSource_SPOT:
		break;
	}
}
