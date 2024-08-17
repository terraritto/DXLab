#include "Test3DGeometryProcessor.h"
#include "../BLASInterface.h"
#include "../../../include/d3dx12.h"

void Test3DGeometryProcessor::ProcessCommand(std::vector<BLASInterface*> blasArray, std::weak_ptr<DXUTILITY::Device> device)
{
	auto deviceData = device.lock();
	if (deviceData == nullptr) { return; }

	auto command = deviceData->CreateCommandList();

	std::vector<D3D12_RESOURCE_BARRIER> barrier;
	for (int i = 0; i < blasArray.size(); i++)
	{
		// 基底コマンドを実行
		blasArray[i]->MakeCommandFromList(command);
		barrier.push_back(CD3DX12_RESOURCE_BARRIER::UAV(blasArray[i]->GetASResource().Get()));
	}

	command->ResourceBarrier(barrier.size(), barrier.data());
	command->Close();

	deviceData->ExecuteCommandList(command);

	// 待ち
	deviceData->waitForIdleGpu();
}
