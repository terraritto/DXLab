#include "MeshPool.h"

MeshPool::~MeshPool()
{
    // ÉfÅ[É^ÇãÛÇ…Ç∑ÇÈ
    m_meshDataList.clear();
}

std::vector<std::shared_ptr<DefaultMesh>>& MeshPool::RegisterMesh(std::wstring fileName, const int size)
{
    auto data = m_meshDataList.find(fileName);
    if (data != m_meshDataList.end())
    {
        return data->second;
    }

    // Ç»ÇØÇÍÇŒí«â¡
    m_meshDataList[fileName] = {};
    std::vector<std::shared_ptr<DefaultMesh>>& meshList = m_meshDataList[fileName];

    for (int i = 0; i < size; i++)
    {
        std::shared_ptr<DefaultMesh> mesh = std::make_shared<DefaultMesh>();
        meshList.emplace_back(mesh);
    }

    return meshList;
}

std::vector<std::shared_ptr<DefaultMesh>>* MeshPool::GetMesh(std::wstring fileName)
{
    auto data = m_meshDataList.find(fileName);
    if (data != m_meshDataList.end())
    {
        return &data->second;
    }

    return nullptr;
}
