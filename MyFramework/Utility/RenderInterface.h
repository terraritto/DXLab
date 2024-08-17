#pragma once

#include <exception>
#include <stdexcept>
#include <functional>
#include <fstream>
#include <vector>

#include "DXUtility.h"
#include "InputManager.h"
#include "TimeManager.h"
#include "../Material/MaterialManager.h"
#include "../Pool/TexturePool.h"
#include "../Pool/MultiTLASPool.h"
#include "../Pool/LightPool.h"

#include <wrl.h>
#include <DirectXMath.h>

class RenderInterface
{
public:
	// Pointer
	template<class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	// Vector
	using XMFLOAT2 = DirectX::XMFLOAT2;
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMFLOAT4 = DirectX::XMFLOAT4;
	using XMVECTOR = DirectX::XMVECTOR;

	// Int Vector
	using XMUINT3 = DirectX::XMUINT3;
	using XMUINT4 = DirectX::XMUINT4;

	// Matrix
	using XMFLOAT3X4 = DirectX::XMFLOAT3X4;
	using XMFLOAT4X4 = DirectX::XMFLOAT4X4;
	using XMMATRIX	 = DirectX::XMMATRIX;

	friend class Win32Application;

public:
	RenderInterface(UINT width, UINT height, const std::wstring& title)
		: m_width(width)
		, m_height(height)
		, m_title(title)
		, m_input(std::make_shared<InputManager>())
		, m_time(std::make_shared<TimeManager>())
	{
		DirectX::XMUINT2 displaySize = DirectX::XMUINT2(m_width, m_height);
		m_input->SetDisplaySize(displaySize);
	}

public:
	virtual void Initialize() = 0;
	virtual void Destroy() = 0;
	virtual void Update(double DeltaTime) = 0;
	virtual void OnRender() = 0;

public:
	// Getter
	const UINT GetWidth() const { return m_width; }
	const UINT GetHeight() const { return m_height; }
	const float GetAspect() const { return float(m_width) / float(m_height); }
	const wchar_t* GetTitle() const { return m_title.c_str(); }

protected:
	// DeviceÇÃèâä˙âª
	bool InitializeGraphicsDevice(HWND hwnd)
	{
		m_device = std::make_shared<DXUTILITY::Device>();
		if (m_device->Initialize() == false)
		{
			return false;
		}

		if (m_device->CreateSwapchain(GetWidth(), GetHeight(), hwnd) == false)
		{
			return false;
		}

		MaterialManager::GetInstance().Setup(m_device);
		TexturePool::GetInstance().Setup(m_device);
		MultiTLASPool::GetInstance().Setup();

		return true;
	}

	// DeviceÇÃîjä¸
	void TerminateGraphicsDevice()
	{
		MaterialManager::GetInstance().Uninitialize(m_device);
		LightPool::GetInstance().Uninitialize(m_device);

		if (m_device)
		{
			m_device->Destroy();
		}
		m_device.reset();
	}

	// DX12ÇÃdevice
	std::shared_ptr<DXUTILITY::Device> m_device;
	std::shared_ptr<InputManager> m_input;
	std::shared_ptr<TimeManager> m_time;

protected:
	// rootsignatureÇÃê∂ê¨
	ComPtr<ID3D12RootSignature> CreateRootSignature(const D3D12_ROOT_SIGNATURE_DESC& rsDesc)
	{
		ComPtr<ID3DBlob> blob, errBlob;
		HRESULT hr = D3D12SerializeRootSignature
		(
			&rsDesc,
			D3D_ROOT_SIGNATURE_VERSION_1_0,
			&blob,
			&errBlob
		);
		if (FAILED(hr))
		{
			throw std::runtime_error("RootSignature failed.");
		}

		// RSÇÃê∂ê¨
		ComPtr<ID3D12RootSignature> rs;
		hr = m_device->GetDevice()->CreateRootSignature
		(
			0,
			blob->GetBufferPointer(),
			blob->GetBufferSize(),
			IID_PPV_ARGS(rs.ReleaseAndGetAddressOf())
		);
		if (FAILED(hr))
		{
			throw std::runtime_error("RS is failed.");
		}

		return rs;
	}

protected:
	UINT m_width;
	UINT m_height;
	std::wstring m_title;
};