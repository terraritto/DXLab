#include "PolygonMeshBLAS.h"

void PolygonMeshBLAS::InitializeGeometry()
{
}

void PolygonMeshBLAS::AllocateGeometryResource()
{
	m_geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
	m_geometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

	auto& triangles = m_geometryDesc.Triangles;
	PolygonMesh* mesh = m_polygonMesh->GetPolygonMesh();
	triangles.VertexBuffer.StartAddress = mesh->vertexBuffer->GetGPUVirtualAddress();
	triangles.VertexBuffer.StrideInBytes = mesh->vertexStride;
	triangles.VertexCount = mesh->vertexCount;
	triangles.IndexBuffer = mesh->indexBuffer->GetGPUVirtualAddress();
	triangles.IndexCount = mesh->indexCount;
	triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
	triangles.IndexFormat = DXGI_FORMAT_R32_UINT;
}

void PolygonMeshBLAS::InitializeGeometryDesc()
{
}

void PolygonMeshBLAS::PreBuildAS()
{
}

void PolygonMeshBLAS::MakeResourceAndConstructAS()
{
}

void PolygonMeshBLAS::MakeCommand()
{
}
