#include "DefaultMeshMaterial.h"
#include "../Pool/TexturePool.h"

DefaultMeshMaterial::DefaultMeshMaterial()
	: MaterialBase(),
	m_albedo(DirectX::XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f)),
	m_specular(DirectX::XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f)),
	m_specularPow(0.0f),
	m_alpha(1.0f),
	m_isAlpha(false),
	m_parameter()
{
	this->m_texturePath.resize(3);
}


void DefaultMeshMaterial::SetColorTexture(const std::wstring filePath)
{
	this->m_texturePath[0] = filePath;
	TexturePool::GetInstance().RegisterTexture(filePath);
}

std::weak_ptr<DXUTILITY::TextureResource> DefaultMeshMaterial::GetColorTexture()
{
	return TexturePool::GetInstance().GetTexture(this->m_texturePath[0]);
}

void DefaultMeshMaterial::SetDiffuseTexture(const std::wstring filePath)
{
	this->m_texturePath[1] = filePath;
	TexturePool::GetInstance().RegisterTexture(filePath);
}

std::weak_ptr<DXUTILITY::TextureResource> DefaultMeshMaterial::GetDiffuseTexture()
{
	return TexturePool::GetInstance().GetTexture(this->m_texturePath[1], false);
}

void DefaultMeshMaterial::SetNormalTexture(const std::wstring filePath)
{
	this->m_texturePath[2] = filePath;
	TexturePool::GetInstance().RegisterTexture(filePath);
}

std::weak_ptr<DXUTILITY::TextureResource> DefaultMeshMaterial::GetNormalTexture()
{
	return TexturePool::GetInstance().GetTexture(this->m_texturePath[2], false, true);
}
