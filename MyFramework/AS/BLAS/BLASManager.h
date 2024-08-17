#pragma once
#include <vector>
#include <map>
#include <memory>
#include "BLASInterface.h"
#include "../../Utility/DXUtility.h"

class BLASInterface;

class BLASCommandProcessor
{
public:
	virtual void ProcessCommand(std::vector<BLASInterface*> blasArray, std::weak_ptr<DXUTILITY::Device> device) {}
};

class BLASManager
{
public:
	// BLASの追加を行う
	void AddBLAS(std::string name, std::unique_ptr<BLASInterface> blas);
	
	// BLAS関係の処理とCommandの実行
	void Initialize();

	// コマンドの設定
	void SetCommandProcessor(std::unique_ptr<BLASCommandProcessor> processor) { m_processor = std::move(processor); }

	// デバイスの設定
	void SetDevice(std::weak_ptr<DXUTILITY::Device> device) { m_device = device; }
	
	// 検索してblasを取得
	BLASInterface* GetBlas(const std::string name) const
	{
		auto blas = m_blasContainer.find(name);
		if (blas != m_blasContainer.end())
		{
			return blas->second.get();
		}

		return nullptr;
	}

	// BLASのリセット
	void ResetBLAS() { m_blasContainer.clear(); }
protected:
	// blasのデータ
	std::map<std::string, std::unique_ptr<BLASInterface>> m_blasContainer;
	
	std::weak_ptr<DXUTILITY::Device> m_device;
	std::unique_ptr<BLASCommandProcessor> m_processor;
};
