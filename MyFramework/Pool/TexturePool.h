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

	// singletonのための実装
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
	// 登録
	void RegisterTexture(std::wstring fileName);

	// 取得
	std::weak_ptr<DXUTILITY::TextureResource> GetTexture(std::wstring& fileName, bool isWhite = true, bool isNormal = false);

protected:
	std::weak_ptr<DXUTILITY::Device> m_device;

	// key: filename data: textureのデータ
	std::unordered_map<std::wstring, std::shared_ptr<DXUTILITY::TextureResource>> m_textureList;
};
