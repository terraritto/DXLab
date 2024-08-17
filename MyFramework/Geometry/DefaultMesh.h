#pragma once
#include <vector>
#include "GeometryDefinitions.h"

// デフォルトのメッシュ
struct DefaultMesh
{
    DefaultMesh();
    DefaultMesh(bool isClearRowData);

    // *---- vertex/ index ----*
    // vertex / index データ
    std::vector<VertexMesh> m_vertices;
    std::vector<UINT> m_indices;
    std::vector <XMFLOAT3> m_positions;
    std::vector<XMFLOAT3> m_normals;
    std::vector<XMFLOAT3> m_tangents;
    std::vector<XMFLOAT2> m_uvs;

    // Max/Minの値
    XMFLOAT3 m_MaxPos, m_MinPos;

    // 頂点/index数
    UINT m_vertexCount;
    UINT m_indexCount;
    
    // strideサイズ
    UINT m_vertexStride;

    // *---- memory ----*
    // 確保用
    ComPtr<ID3D12Resource> m_vertexBuffer;
    ComPtr<ID3D12Resource> m_indexBuffer;

    // ジオメトリ情報を参照するディスクリプタ
    DXUTILITY::Descriptor m_descriptorVB;
    DXUTILITY::Descriptor m_descriptorIB;

    // *---- For DX12 ----*
    // View
    D3D12_VERTEX_BUFFER_VIEW m_vertexView;
    D3D12_INDEX_BUFFER_VIEW m_indexView;

    // *---- For DXR ----*
    // BLAS 用バッファ
    ComPtr<ID3D12Resource> m_blas;

    ComPtr<ID3D12Resource> m_positionBuffer;
    ComPtr<ID3D12Resource> m_normalBuffer;
    ComPtr<ID3D12Resource> m_tangentBuffer;
    ComPtr<ID3D12Resource> m_uvBuffer;

    DXUTILITY::Descriptor m_descriptorPosition;
    DXUTILITY::Descriptor m_descriptorNormal;
    DXUTILITY::Descriptor m_descriptorTangent;
    DXUTILITY::Descriptor m_descriptorUv;

    // *---- material ----*
    UINT m_materialId; // マテリアル番号

    // *----処理分岐用 ----*
    bool m_isClearRowData; // 生の情報を残すかどうか

    // 名前
    std::string m_name;
};