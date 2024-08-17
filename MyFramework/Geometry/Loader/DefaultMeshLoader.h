#pragma once
#include "MeshLoaderBase.h"

class DefaultMeshLoader : public MeshLoaderBase
{
protected:
	virtual void ParseMesh(std::weak_ptr<DefaultMesh> dstMesh, const aiMesh* srcMesh) override;
	virtual void ParseMaterial(std::unique_ptr<DefaultMeshMaterial>& dstMaterial, const aiMaterial* srcMaterial) override;
	virtual void ParseLight(std::unique_ptr<Light>& dstLight, const aiLight* srcLight) override;
};