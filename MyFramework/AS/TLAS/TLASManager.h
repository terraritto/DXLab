#pragma once
#include <vector>
#include <memory>
#include "../../Utility/DXUtility.h"

class TLASInterface;

class TLASCommandProcessor
{
public:
	virtual void ProcessCommand(std::vector<std::unique_ptr<TLASInterface>> blasArray, DXUTILITY::Device* device) {}
};

class TLASManager
{
public:
	// TLASの追加を行う
	void AddTLAS(std::unique_ptr<TLASInterface> tlas);

	// TLAS関係の処理とCommandの実行
	void Initialize();

	// コマンドの設定
	void SetCommandProcessor(std::unique_ptr<TLASCommandProcessor> processor) { m_processor = std::move(processor); }

	// デバイスの設定
	void SetDevice(DXUTILITY::Device* device) { m_device = device; }

	// BLASのリセット
	void ResetTLAS() { m_blasContainer.clear(); }
protected:
	std::vector<std::unique_ptr<TLASInterface>> m_blasContainer;
	DXUTILITY::Device* m_device;
	std::unique_ptr<TLASCommandProcessor> m_processor;
};
