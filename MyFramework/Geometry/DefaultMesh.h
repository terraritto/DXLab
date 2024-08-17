#pragma once
#include <vector>
#include "GeometryDefinitions.h"

// �f�t�H���g�̃��b�V��
struct DefaultMesh
{
    DefaultMesh();
    DefaultMesh(bool isClearRowData);

    // *---- vertex/ index ----*
    // vertex / index �f�[�^
    std::vector<VertexMesh> m_vertices;
    std::vector<UINT> m_indices;
    std::vector <XMFLOAT3> m_positions;
    std::vector<XMFLOAT3> m_normals;
    std::vector<XMFLOAT3> m_tangents;
    std::vector<XMFLOAT2> m_uvs;

    // Max/Min�̒l
    XMFLOAT3 m_MaxPos, m_MinPos;

    // ���_/index��
    UINT m_vertexCount;
    UINT m_indexCount;
    
    // stride�T�C�Y
    UINT m_vertexStride;

    // *---- memory ----*
    // �m�ۗp
    ComPtr<ID3D12Resource> m_vertexBuffer;
    ComPtr<ID3D12Resource> m_indexBuffer;

    // �W�I���g�������Q�Ƃ���f�B�X�N���v�^
    DXUTILITY::Descriptor m_descriptorVB;
    DXUTILITY::Descriptor m_descriptorIB;

    // *---- For DX12 ----*
    // View
    D3D12_VERTEX_BUFFER_VIEW m_vertexView;
    D3D12_INDEX_BUFFER_VIEW m_indexView;

    // *---- For DXR ----*
    // BLAS �p�o�b�t�@
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
    UINT m_materialId; // �}�e���A���ԍ�

    // *----��������p ----*
    bool m_isClearRowData; // ���̏����c�����ǂ���

    // ���O
    std::string m_name;
};