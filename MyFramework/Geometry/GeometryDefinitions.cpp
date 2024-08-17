#include "GeometryDefinitions.h"
#include <algorithm>

void GetPlane(std::vector<VertexPNC>& vertices, std::vector<UINT>& indices, float size)
{
    //planeの頂点情報
    VertexPNC srcVertices[] = {
        VertexPNC{ {-1.0f, 0.0f,-1.0f }, { 0.0f, 1.0f, 0.0f }, WHITE },
        VertexPNC{ {-1.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f }, WHITE },
        VertexPNC{ { 1.0f, 0.0f,-1.0f }, { 0.0f, 1.0f, 0.0f }, WHITE },
        VertexPNC{ { 1.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f }, WHITE },
    };
    //四角形分確保
    vertices.resize(4);
    //全頂点をsize倍
    std::transform(
        std::begin(srcVertices), std::end(srcVertices), vertices.begin(),
        [=](auto v) {
            v.Position.x *= size;
            v.Position.z *= size;
            return v;
        }
    );

    //index情報
    indices = { 0, 1, 2, 2, 1, 3 };
}

void GetPlane(std::vector<VertexPNT>& vertices, std::vector<UINT>& indices, float size)
{
    //planeの頂点情報
    VertexPNT srcVertices[] = {
        VertexPNT{ {-1.0f, 0.0f,-1.0f }, { 0.0f, 1.0f, 0.0f }, {0.0f, 0.0f} },
        VertexPNT{ {-1.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f }, {0.0f, 1.0f} },
        VertexPNT{ { 1.0f, 0.0f,-1.0f }, { 0.0f, 1.0f, 0.0f }, {1.0f, 0.0f} },
        VertexPNT{ { 1.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f }, {1.0f, 1.0f} },
    };
    //四角形分確保
    vertices.resize(4);
    //全頂点をsize倍
    std::transform(
        std::begin(srcVertices), std::end(srcVertices), vertices.begin(),
        [=](auto v) {
            v.Position.x *= size;
            v.Position.z *= size;
            return v;
        }
    );

    //index情報
    indices = { 0, 1, 2, 2, 1, 3 };
}

void GetColoredCube(std::vector<VertexPNC>& vertices, std::vector<UINT>& indices, float size)
{
    //初期化
    vertices.clear();
    indices.clear();

    //頂点データ生成
    vertices = {
        // 裏
        { {-1.0f,-1.0f,-1.0f}, { 0.0f, 0.0f, -1.0f }, RED },
        { {-1.0f, 1.0f,-1.0f}, { 0.0f, 0.0f, -1.0f }, YELLOW },
        { { 1.0f, 1.0f,-1.0f}, { 0.0f, 0.0f, -1.0f }, WHITE },
        { { 1.0f,-1.0f,-1.0f}, { 0.0f, 0.0f, -1.0f }, MAGENTA },
        // 右
        { { 1.0f,-1.0f,-1.0f}, { 1.0f, 0.0f, 0.0f }, MAGENTA },
        { { 1.0f, 1.0f,-1.0f}, { 1.0f, 0.0f, 0.0f }, WHITE },
        { { 1.0f, 1.0f, 1.0f}, { 1.0f, 0.0f, 0.0f }, CYAN },
        { { 1.0f,-1.0f, 1.0f}, { 1.0f, 0.0f, 0.0f }, BLUE },
        // 左
        { {-1.0f,-1.0f, 1.0f}, { -1.0f, 0.0f, 0.0f }, BLACK },
        { {-1.0f, 1.0f, 1.0f}, { -1.0f, 0.0f, 0.0f }, GREEN },
        { {-1.0f, 1.0f,-1.0f}, { -1.0f, 0.0f, 0.0f }, YELLOW },
        { {-1.0f,-1.0f,-1.0f}, { -1.0f, 0.0f, 0.0f }, RED },
        // 正面
        { { 1.0f,-1.0f, 1.0f}, { 0.0f, 0.0f, 1.0f}, BLUE },
        { { 1.0f, 1.0f, 1.0f}, { 0.0f, 0.0f, 1.0f}, CYAN },
        { {-1.0f, 1.0f, 1.0f}, { 0.0f, 0.0f, 1.0f}, GREEN },
        { {-1.0f,-1.0f, 1.0f}, { 0.0f, 0.0f, 1.0f}, BLACK },
        // 上
        { {-1.0f, 1.0f,-1.0f}, { 0.0f, 1.0f, 0.0f}, YELLOW },
        { {-1.0f, 1.0f, 1.0f}, { 0.0f, 1.0f, 0.0f}, GREEN },
        { { 1.0f, 1.0f, 1.0f}, { 0.0f, 1.0f, 0.0f}, CYAN },
        { { 1.0f, 1.0f,-1.0f}, { 0.0f, 1.0f, 0.0f}, WHITE },
        // 底
        { {-1.0f,-1.0f, 1.0f}, { 0.0f, -1.0f, 0.0f}, BLACK },
        { {-1.0f,-1.0f,-1.0f}, { 0.0f, -1.0f, 0.0f}, RED },
        { { 1.0f,-1.0f,-1.0f}, { 0.0f, -1.0f, 0.0f}, MAGENTA },
        { { 1.0f,-1.0f, 1.0f}, { 0.0f, -1.0f, 0.0f}, BLUE },
    };
    //indexデータ生成
    indices = {
        0, 1, 2, 2, 3,0,
        4, 5, 6, 6, 7,4,
        8, 9, 10, 10, 11, 8,
        12,13,14, 14,15,12,
        16,17,18, 18,19,16,
        20,21,22, 22,23,20,
    };

    //統一的なscaleを掛ける
    std::transform(
        vertices.begin(), vertices.end(), vertices.begin(),
        [=](auto v) {
            v.Position.x *= size;
            v.Position.y *= size;
            v.Position.z *= size;
            return v;
        }
    );
}

void GetColoredCube(std::vector<VertexPCT>& vertices, std::vector<UINT>& indices, float size)
{
    //初期化
    vertices.clear();
    indices.clear();

    //頂点データ生成
    vertices = {
        // 裏
        { {-1.0f,-1.0f,-1.0f}, RED, {0.0f, 1.0f} },
        { {-1.0f, 1.0f,-1.0f}, YELLOW, {0.0f, 0.0f} },
        { { 1.0f, 1.0f,-1.0f}, WHITE, {1.0f, 0.0f} },
        { { 1.0f,-1.0f,-1.0f}, MAGENTA, {1.0f, 1.0f} },
        // 右
        { { 1.0f,-1.0f,-1.0f}, MAGENTA, {0.0f, 1.0f} },
        { { 1.0f, 1.0f,-1.0f}, WHITE, {0.0f, 0.0f} },
        { { 1.0f, 1.0f, 1.0f}, CYAN, {1.0f, 0.0f} },
        { { 1.0f,-1.0f, 1.0f}, BLUE, {1.0f, 1.0f} },
        // 左
        { {-1.0f,-1.0f, 1.0f}, BLACK, {0.0f, 1.0f} },
        { {-1.0f, 1.0f, 1.0f}, GREEN, {0.0f, 0.0f} },
        { {-1.0f, 1.0f,-1.0f}, YELLOW, {1.0f, 0.0f} },
        { {-1.0f,-1.0f,-1.0f}, RED, {1.0f, 1.0f} },
        // 正面
        { { 1.0f,-1.0f, 1.0f}, BLUE, {0.0f, 1.0f} },
        { { 1.0f, 1.0f, 1.0f}, CYAN, {0.0f, 0.0f} },
        { {-1.0f, 1.0f, 1.0f}, GREEN, {1.0f, 0.0f} },
        { {-1.0f,-1.0f, 1.0f}, BLACK, {1.0f, 1.0f} },
        // 上
        { {-1.0f, 1.0f,-1.0f}, YELLOW, {0.0f, 1.0f} },
        { {-1.0f, 1.0f, 1.0f}, GREEN, {0.0f, 0.0f} },
        { { 1.0f, 1.0f, 1.0f}, CYAN, {1.0f, 0.0f} },
        { { 1.0f, 1.0f,-1.0f}, WHITE, {1.0f,1.0f} },
        // 底
        { {-1.0f,-1.0f, 1.0f}, BLACK, {0.0f, 1.0f} },
        { {-1.0f,-1.0f,-1.0f}, RED, {0.0f, 0.0f} },
        { { 1.0f,-1.0f,-1.0f}, MAGENTA, {1.0f, 0.0f} },
        { { 1.0f,-1.0f, 1.0f}, BLUE, {1.0f, 1.0f} },
    };
    //indexデータ生成
    indices = {
        0, 1, 2, 2, 3,0,
        4, 5, 6, 6, 7,4,
        8, 9, 10, 10, 11, 8,
        12,13,14, 14,15,12,
        16,17,18, 18,19,16,
        20,21,22, 22,23,20,
    };

    //統一的なscaleを掛ける
    std::transform(
        vertices.begin(), vertices.end(), vertices.begin(),
        [=](auto v) {
            v.Position.x *= size;
            v.Position.y *= size;
            v.Position.z *= size;
            return v;
        }
    );
}

//sphere関係の計算
static void SetSphereVertex(
    VertexPN& vert,
    const DirectX::XMVECTOR& position, const DirectX::XMVECTOR& normal, const DirectX::XMFLOAT2& uv) 
{
    XMStoreFloat3(&vert.Position, position);
    XMStoreFloat3(&vert.Normal, normal);
}

static void SetSphereVertex(
    VertexPNT& vert,
    const DirectX::XMVECTOR& position, const DirectX::XMVECTOR& normal, const DirectX::XMFLOAT2& uv) 
{
    XMStoreFloat3(&vert.Position, position);
    XMStoreFloat3(&vert.Normal, normal);
    vert.UV = uv;
}

template<class T>
static void CreateSphereVertices(std::vector<T>& vertices, float radius, int slices, int stacks) 
{
    using namespace DirectX;

    vertices.clear();
    const auto SLICES = float(slices);
    const auto STACKS = float(stacks);
    for (int stack = 0; stack <= stacks; ++stack) {
        for (int slice = 0; slice <= slices; ++slice) {
            XMFLOAT3 p;
            p.y = 2.0f * stack / STACKS - 1.0f;
            float r = std::sqrtf(1 - p.y * p.y);
            float theta = 2.0f * XM_PI * slice / SLICES;
            p.x = r * std::sinf(theta);
            p.z = r * std::cosf(theta);

            XMVECTOR v = XMLoadFloat3(&p) * radius;
            XMVECTOR n = XMVector3Normalize(v);
            XMFLOAT2 uv = {
                float(slice) / SLICES,
                1.0f - float(stack) / STACKS,
            };

            T vtx{};
            SetSphereVertex(vtx, v, n, uv);
            vertices.push_back(vtx);
        }
    }
}

static void CreateSphereIndices(std::vector<UINT>& indices, int slices, int stacks) 
{
    for (int stack = 0; stack < stacks; ++stack) {
        const int sliceMax = slices + 1;
        for (int slice = 0; slice < slices; ++slice) {
            int idx = stack * sliceMax;
            int i0 = idx + (slice + 0) % sliceMax;
            int i1 = idx + (slice + 1) % sliceMax;
            int i2 = i0 + sliceMax;
            int i3 = i1 + sliceMax;

            indices.push_back(i0); indices.push_back(i1); indices.push_back(i2);
            indices.push_back(i2); indices.push_back(i1); indices.push_back(i3);
        }
    }
}

void GetSphere(std::vector<VertexPN>& vertices, std::vector<UINT>& indices, float radius, int slices, int stacks)
{
    vertices.clear();
    indices.clear();

    // 作成
    CreateSphereVertices(vertices, radius, slices, stacks);
    CreateSphereIndices(indices, slices, stacks);
}

D3D12_RAYTRACING_GEOMETRY_DESC GetGeometryDesc(const PolygonMesh& mesh)
{
    //基本的な設定をまとめる
    auto geometryDesc = D3D12_RAYTRACING_GEOMETRY_DESC{};
    geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
    geometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
    
    //Triangleの設定
    auto& triangles = geometryDesc.Triangles;
    triangles.VertexBuffer.StartAddress = mesh.vertexBuffer->GetGPUVirtualAddress();
    triangles.VertexBuffer.StrideInBytes = mesh.vertexStride;
    triangles.VertexCount = mesh.vertexCount;
    triangles.IndexBuffer = mesh.indexBuffer->GetGPUVirtualAddress();
    triangles.IndexCount = mesh.indexCount;
    triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
    triangles.IndexFormat = DXGI_FORMAT_R32_UINT;
    return geometryDesc;
}
