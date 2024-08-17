#pragma once
#include <DirectXMath.h>
#include "../Utility/DXUtility.h"
#include "../Utility/TypeUtility.h"

using XMFLOAT3 = DirectX::XMFLOAT3;
using XMFLOAT4 = DirectX::XMFLOAT4;

// Color
constexpr XMFLOAT4 WHITE    = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
constexpr XMFLOAT4 RED      = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
constexpr XMFLOAT4 GREEN    = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
constexpr XMFLOAT4 BLUE     = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
constexpr XMFLOAT4 BLACK    = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
constexpr XMFLOAT4 YELLOW   = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
constexpr XMFLOAT4 MAGENTA  = XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f);
constexpr XMFLOAT4 CYAN     = XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f);

// �ʒu�݂̂̒��_
struct PositionVertex
{
	PositionVertex(float x, float y, float z) { m_position.x = x; m_position.y = y; m_position.z = z; }
	XMFLOAT3 m_position;
};

// �ʒu,�@��,�F�̒��_
struct VertexPNC 
{
    XMFLOAT3 Position;
    XMFLOAT3 Normal;
    XMFLOAT4 Color;
};

// �ʒu,�@��,uv�̒��_
struct VertexPNT
{
    XMFLOAT3 Position;
    XMFLOAT3 Normal;
    XMFLOAT2 UV;
};

// �ʒu,�F,uv�̒��_
struct VertexPCT
{
    XMFLOAT3 Position;
    XMFLOAT4 Color;
    XMFLOAT2 UV;
};

// �ʒu,�@��,uv�̒��_
struct VertexPT
{
    XMFLOAT3 Position;
    XMFLOAT2 UV;
};

// �ʒu,�@���̒��_
struct VertexPN
{
    XMFLOAT3 Position;
    XMFLOAT3 Normal;
};

// Mesh�p���_
struct VertexMesh
{
    XMFLOAT3 Position;
    XMFLOAT3 Normal;
    XMFLOAT2 TexCoord;
    XMFLOAT3 Tangent;

    VertexMesh() = default;

    VertexMesh(XMFLOAT3 pos, XMFLOAT3 n, XMFLOAT2 uv, XMFLOAT3 tan)
        : Position(pos)
        , Normal(n)
        , TexCoord(uv)
        , Tangent(tan)
    {}

    static const D3D12_INPUT_LAYOUT_DESC InputLayout;

private:
    static constexpr int InputElementCount = 4;
    static const D3D12_INPUT_ELEMENT_DESC InputElements[InputElementCount];
};

// todo: ����
// �O�p�`�|���S���pGeometry
struct PolygonMesh
{
    ComPtr<ID3D12Resource> vertexBuffer;
    ComPtr<ID3D12Resource> indexBuffer;

    // �W�I���g�������Q�Ƃ���f�B�X�N���v�^
    DXUTILITY::Descriptor descriptorVB;
    DXUTILITY::Descriptor descriptorIB;

    // View
    D3D12_VERTEX_BUFFER_VIEW vertexView;
    D3D12_INDEX_BUFFER_VIEW indexView;

    UINT vertexCount = 0;
    UINT indexCount = 0;
    UINT vertexStride = 0;

    // Material
    UINT materialIndex = 0;

    // BLAS �p�o�b�t�@
    ComPtr<ID3D12Resource> blas;
};

// ���O�Ńq�b�g���v�Z����悤��Mesh�p
struct ProcedualMesh
{
    ComPtr<ID3D12Resource> aabbBuffer;
    ComPtr<ID3D12Resource> blas;
};

// Plane
void GetPlane(std::vector<VertexPNC>& vertices, std::vector<UINT>& indices, float size = 10.0f);
void GetPlane(std::vector<VertexPNT>& vertices, std::vector<UINT>& indices, float size = 10.0f);

// Cube
void GetColoredCube(std::vector<VertexPNC>& vertices, std::vector<UINT>& indices, float size = 1.0f);
void GetColoredCube(std::vector<VertexPCT>& vertices, std::vector<UINT>& indices, float size = 1.0f);

// Sphere
void GetSphere(std::vector<VertexPN>& vertices, std::vector<UINT>& indices, float radius = 1.0f, int slices = 16, int stacks = 24);

// polygon mesh��񂩂� D3D12_RAYTRACING_GEOMETRY_DESC ���\������.
// �\�������l�͕W���I�Ȑݒ�ɂȂ邽�߁A�擾��K�X�ύX���邱��.
D3D12_RAYTRACING_GEOMETRY_DESC GetGeometryDesc(const PolygonMesh& mesh);
