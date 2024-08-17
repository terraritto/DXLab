#include "SelfDefMaterial.h"
#include <stdexcept>

void SelfDefMaterial::AllocateSpecificTexture(std::weak_ptr<DXUTILITY::Device> device, const std::wstring& fileName)
{
	DXUTILITY::TextureResource textureResource;
	textureResource = DXUTILITY::LoadTextureFromFile(device, fileName);

	if (textureResource.res.Get() == nullptr)
	{
		throw std::logic_error("Not found Texture:" + *fileName.c_str());
	}

	m_specialTexture.push_back(textureResource);
}

void SelfDefMaterial::WriteSpecificTexureForShaderTable(uint8_t* dst)
{
	for (auto& texture : m_specialTexture)
	{
		dst += DXUTILITY::WriteGPUDescriptor(dst, texture.srv);
	}
}
