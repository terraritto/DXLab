#pragma once
#include "GeometryDefinitions.h"

class GeometryContainer
{
public:
	GeometryContainer();

	// Geometryの初期化
	virtual void InitializeGeometry(std::weak_ptr<DXUTILITY::Device> device) = 0;
	
	PolygonMesh* GetPolygonMesh() const { return m_polygonMesh.get(); }

	// ファイルが必要な場合の登録
	void SetFileName(const std::wstring& fileName) { m_fileName = fileName; }

protected:
	std::unique_ptr<PolygonMesh> m_polygonMesh;

	std::wstring m_fileName;
};