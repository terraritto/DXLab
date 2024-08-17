#pragma once
#ifdef _DEBUG
#pragma comment(lib, "assimp-vc143-mtd.lib")
#else
#pragma comment(lib, "assimp-vc143-mt.lib")
#endif

#include <string>
#include <vector>
#include <memory>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "../../Geometry/DefaultMesh.h"
#include "../../Material/DefaultMeshMaterial.h"
#include "../../Light/Light.h"
#include "../../Utility/DXUtility.h"

class MeshLoaderBase
{
public:
	bool LoadMesh(std::weak_ptr<DXUTILITY::Device> device, const std::wstring& fileName);

protected:
	bool SetupMesh(std::weak_ptr<DXUTILITY::Device> device, std::vector<std::weak_ptr<DefaultMesh>>& meshList);

protected:
	virtual void ParseMesh(std::weak_ptr<DefaultMesh> dstMesh, const aiMesh* srcMesh) = 0;
	virtual void ParseMaterial(std::unique_ptr<DefaultMeshMaterial>& dstMaterial, const aiMaterial* srcMaterial) = 0;
	virtual void ParseLight(std::unique_ptr<Light>& dstLight, const aiLight* srcLight) = 0;

protected:
	std::weak_ptr<DXUTILITY::Device> m_device;
};