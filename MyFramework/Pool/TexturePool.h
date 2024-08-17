#pragma once
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include "../Utility/DXUtility.h"

class TexturePool
{
private:
	// destructor
	~TexturePool();
	TexturePool() = default;

public:
	void Setup(std::weak_ptr<DXUTILITY::Device> device);

	// singleton‚Ì‚½‚ß‚ÌÀ‘•
	TexturePool(const TexturePool&) = delete;
	TexturePool& operator=(const TexturePool&) = delete;
	TexturePool(TexturePool&&) = delete;
	TexturePool& operator=(TexturePool&&) = delete;

	static TexturePool& GetInstance()
	{
		static TexturePool instance;
		return instance;
	}

public:
	// “o˜^
	void RegisterTexture(std::wstring fileName);

	// æ“¾
	std::weak_ptr<DXUTILITY::TextureResource> GetTexture(std::wstring& fileName, bool isWhite = true, bool isNormal = false);

protected:
	std::weak_ptr<DXUTILITY::Device> m_device;

	// key: filename data: texture‚Ìƒf[ƒ^
	std::unordered_map<std::wstring, std::shared_ptr<DXUTILITY::TextureResource>> m_textureList;
};
