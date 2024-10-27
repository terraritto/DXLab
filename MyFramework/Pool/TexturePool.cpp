#include "TexturePool.h"
#include <filesystem>
#include <string>

#define DUMMY_TEXTURE_WHITE L"White.png"
#define DUMMY_TEXTURE_BLACK L"Black.png"
#define DUMMY_TEXTURE_NORMAL L"normal.jpg"

TexturePool::~TexturePool()
{
    m_textureList.clear();
}

void TexturePool::Setup(std::weak_ptr<DXUTILITY::Device> device)
{
    m_device = device;
    m_textureList.clear();

    RegisterTexture(DUMMY_TEXTURE_WHITE); // Dummy Texture
    RegisterTexture(DUMMY_TEXTURE_BLACK); // Dummy Texture
    RegisterTexture(DUMMY_TEXTURE_NORMAL); // Dummy Texture
}

void TexturePool::RegisterTexture(std::wstring fileName)
{
    auto data = m_textureList.find(fileName);
    if (data != m_textureList.end())
    {
        return;
    }

    // filenameÇÃâ¡çH
    auto fileNameOnly = fileName;
    auto splitChr = fileName.rfind(L"/");
    if (splitChr != std::wstring::npos)
    {
        fileNameOnly = fileName.substr(splitChr + 1, fileName.size() - (splitChr + 1));
    }

    splitChr = fileName.rfind(L"\\");
    if (splitChr != std::wstring::npos)
    {
        fileNameOnly = fileName.substr(splitChr + 1, fileName.size() - (splitChr + 1));
    }

    std::filesystem::path path = std::filesystem::current_path().c_str();
    path += "\\Mesh"; // Meshì‡Ç…Ç†ÇÈëzíË
    std::wstring filePath;
    for (const auto& file : std::filesystem::recursive_directory_iterator(path))
    {
        if (fileNameOnly == file.path().filename())
        {
            filePath = file.path();
            break;
        }
    }
    
    auto texture = DXUTILITY::LoadTextureFromFile(m_device, filePath);
    std::shared_ptr<DXUTILITY::TextureResource> pTexture = std::make_shared<DXUTILITY::TextureResource>(texture);
    m_textureList[fileName] = pTexture;
}

std::weak_ptr<DXUTILITY::TextureResource> TexturePool::GetTexture(std::wstring& fileName, bool isWhite, bool isNormal)
{
    auto data = m_textureList.find(fileName);
    if (data != m_textureList.end())
    {
        return data->second;
    }

    if (isNormal)
    {
        return m_textureList[DUMMY_TEXTURE_NORMAL];
    }

    return m_textureList[isWhite ? DUMMY_TEXTURE_WHITE : DUMMY_TEXTURE_BLACK];
}
