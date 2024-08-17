#pragma once
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include "../Geometry/DefaultMesh.h"

class MeshPool
{
private:
	// destructor
	~MeshPool();
	MeshPool() = default;

public:
	// singleton‚Ì‚½‚ß‚ÌÀ‘•
	MeshPool(const MeshPool&) = delete;
	MeshPool& operator=(const MeshPool&) = delete;
	MeshPool(MeshPool&&) = delete;
	MeshPool& operator=(MeshPool&&) = delete;

	static MeshPool& GetInstance()
	{
		static MeshPool instance;
		return instance;
	}

public:
	// “o˜^
	std::vector<std::shared_ptr<DefaultMesh>>& RegisterMesh(std::wstring fileName, const int size);

	// æ“¾
	std::vector<std::shared_ptr<DefaultMesh>>* GetMesh(std::wstring fileName);

protected:
	// key: filename data: mesh‚Ìƒf[ƒ^
	std::unordered_map<std::wstring, std::vector<std::shared_ptr<DefaultMesh>>> m_meshDataList;
};
