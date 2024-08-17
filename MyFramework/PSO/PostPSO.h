#pragma once
#include "PSOContainer.h"
#include "../Utility/DXUtility.h"

// Postに必要なものをまとめて用意
class PostPSO : public PSOContainer
{
public:
	PostPSO(std::weak_ptr<DXUTILITY::Device> device);

	~PostPSO();

	virtual void Update(double){}

	virtual void WriteCBData() {}

	[[nodiscard]]
	virtual int GetCBSize() const { return 0; }

	void Draw(ComPtr<ID3D12GraphicsCommandList4>& commandList, D3D12_GPU_DESCRIPTOR_HANDLE& handle);

protected:
	std::weak_ptr<DXUTILITY::Device> m_device;

	size_t m_indexCount;

	// Post用パラメータ
	DXUTILITY::DynamicBuffer m_constantBuffer;

	ComPtr<ID3D12Resource> m_vertexBuffer;
	ComPtr<ID3D12Resource> m_indexBuffer;

	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW m_indexBufferView;
};