#pragma once
#include "GeometryDefinitions.h"

class ProcedualContainer
{
public:
	ProcedualContainer();

	// Geometryの初期化
	virtual void InitializeGeometry(std::weak_ptr<DXUTILITY::Device> device) = 0;

	ProcedualMesh* GetProcedualMesh() const { return m_procedualMesh.get(); }

	void SetScale(const float& scale) { m_scale = scale; }

protected:
	std::unique_ptr<ProcedualMesh> m_procedualMesh;

	// サイズの設定
	float m_scale = 1.0f;
};
