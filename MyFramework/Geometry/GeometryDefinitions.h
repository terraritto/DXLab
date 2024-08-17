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

// 位置のみの頂点
struct PositionVertex
{
	PositionVertex(float x, float y, float z) { m_position.x = x; m_position.y = y; m_position.z = z; }
	XMFLOAT3 m_position;
};

// 位置,法線,色の頂点
struct VertexPNC 
{
    XMFLOAT3 Position;
    XMFLOAT3 Normal;
    XMFLOAT4 Color;
};

// 位置,法線,uvの頂点
struct VertexPNT
{
    XMFLOAT3 Position;
    XMFLOAT3 Normal;
    XMFLOAT2 UV;
};

// 位置,色,uvの頂点
struct VertexPCT
{
    XMFLOAT3 Position;
    XMFLOAT4 Color;
    XMFLOAT2 UV;
};

// 位置,法線,uvの頂点
struct VertexPT
{
    XMFLOAT3 Position;
    XMFLOAT2 UV;
};

// 位置,法線の頂点
struct VertexPN
{
    XMFLOAT3 Position;
    XMFLOAT3 Normal;
};

// Mesh用頂点
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

// todo: 消す
// 三角形ポリゴン用Geometry
struct PolygonMesh
{
    ComPtr<ID3D12Resource> vertexBuffer;
    ComPtr<ID3D12Resource> indexBuffer;

    // ジオメトリ情報を参照するディスクリプタ
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

    // BLAS 用バッファ
    ComPtr<ID3D12Resource> blas;
};

// 自前でヒットを計算するようなMesh用
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

// polygon mesh情報から D3D12_RAYTRACING_GEOMETRY_DESC を構成する.
// 構成される値は標準的な設定になるため、取得後適宜変更すること.
D3D12_RAYTRACING_GEOMETRY_DESC GetGeometryDesc(const PolygonMesh& mesh);
