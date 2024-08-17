#include "DXUtility.h"
#include "MathUtility.h"
#include "../../include/d3dx12.h"
#include "../Geometry/GeometryDefinitions.h"
#include "../Material/MaterialBase.h"

#include <stdexcept>
#include <DirectXTex.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

namespace DXUTILITY
{
	void DescriptorHeapManager::Allocate(Descriptor* desc)
	{
		// deallocateされていて登録されていたものは使いまわす
		auto it = m_freeMap.find(1);
		if (it != m_freeMap.end())
		{
			if (it->second.empty() == false)
			{
				// 設定を行いmapから削除
				*desc = it->second.front();
				it->second.pop_front();
				return;
			}
		}

		// 使いまわしするものがないときはheapから確保しにいく
		D3D12_CPU_DESCRIPTOR_HANDLE hCpu = m_heap->GetCPUDescriptorHandleForHeapStart();
		D3D12_GPU_DESCRIPTOR_HANDLE hGpu{};
		if (m_heap->GetDesc().Type != D3D12_DESCRIPTOR_HEAP_TYPE_RTV &&
			m_heap->GetDesc().Type != D3D12_DESCRIPTOR_HEAP_TYPE_DSV)
		{
			hGpu = m_heap->GetGPUDescriptorHandleForHeapStart();
		}
		
		*desc = {};

		auto heapDesc = m_heap->GetDesc();
		if (m_allocateIndex < heapDesc.NumDescriptors)
		{
			// まだ作れる場合はindexをずらしてdescに登録
			auto offset = m_incrementSize * m_allocateIndex;
			hCpu.ptr += offset;
			hGpu.ptr += offset;
			(*desc).m_heapBaseOffset = offset;
			(*desc).m_cpuHandle = hCpu;
			(*desc).m_gpuHandle = hGpu;
			(*desc).m_heapType = heapDesc.Type;
			m_allocateIndex++;
		}
	}

	void DescriptorHeapManager::Deallocate(Descriptor* desc)
	{
		auto it = m_freeMap.find(1);
		if (it == m_freeMap.end())
		{
			// まだ存在しない場合は生成
			m_freeMap.insert(std::make_pair(1, DescriptorList()));
			it = m_freeMap.find(1);
		}

		// 使いまわしに登録
		it->second.push_back(*desc);
	}

	void DescriptorHeapManager::AllocateTable(UINT count, Descriptor* descs)
	{
		auto it = m_freeMap.find(count);
		if (it != m_freeMap.end())
		{
			if (!it->second.empty())
			{
				// freeなallocatorでcount数が一致するものが存在
				// 使いまわす
				*descs = it->second.front();
				it->second.pop_front();
				return;
			}
		}

		auto hCpu = m_heap->GetCPUDescriptorHandleForHeapStart();
		auto hGpu = m_heap->GetGPUDescriptorHandleForHeapStart();
		auto heapDesc = m_heap->GetDesc();

		*descs = {};

		if (m_allocateIndex + count < heapDesc.NumDescriptors)
		{
			auto offset = m_incrementSize * m_allocateIndex;
			hCpu.ptr += offset;
			hGpu.ptr += offset;
			(*descs).m_heapBaseOffset = offset;
			(*descs).m_cpuHandle = hCpu;
			(*descs).m_gpuHandle = hGpu;
			(*descs).m_heapType = heapDesc.Type;
			
			// ex:count=5,次がズレる
			// ---| <-now start
			// ---|-----| <-next start

			// 次のallocate位置はcount分ズレるので問題なし
			m_allocateIndex += count;
		}
	}

	void DescriptorHeapManager::DeallocateTable(UINT count, Descriptor* descs)
	{
		auto it = m_freeMap.find(count);
		if (it == m_freeMap.end())
		{
			// まだ存在しない場合は生成
			m_freeMap.insert(std::make_pair(count, DescriptorList()));
			it = m_freeMap.find(count);
		}

		// 使いまわしに登録
		it->second.push_back(*descs);
	}

	Device::Device()
		: m_width(0)
		, m_height(0)
		, m_frameFenceArray()
		, m_fenceValueArray()
		, m_viewport()
		, m_scissorRect()
	{
		// CPUからのみアクセス可能なメモリにリソース確保
		m_defaultHeapProps = D3D12_HEAP_PROPERTIES
		{
			D3D12_HEAP_TYPE_DEFAULT,
			D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
			D3D12_MEMORY_POOL_UNKNOWN,
			1,1
		};

		// CPUからのアクセスOK + GPUの読み取りは一度
		m_uploadHeapProps = D3D12_HEAP_PROPERTIES
		{
			D3D12_HEAP_TYPE_UPLOAD,
			D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
			D3D12_MEMORY_POOL_UNKNOWN,
			1,1
		};
	}

	Device::~Device()
	{
	}

	bool Device::Initialize()
	{
		HRESULT hr;
		UINT dxgiFlags = 0;
#if _DEBUG
		// Debug Layer有効化
		ComPtr<ID3D12Debug> debug;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug))))
		{
			debug->EnableDebugLayer();
			dxgiFlags |= DXGI_CREATE_FACTORY_DEBUG;
		}
#endif

		// Device Removed Extended Data をON
#if defined(__ID3D12DeviceRemovedExtendedDataSettings1_INTERFACE_DEFINED__)
		ID3D12DeviceRemovedExtendedDataSettings1* pDredSettings1 = nullptr;
		hr = D3D12GetDebugInterface(__uuidof(ID3D12DeviceRemovedExtendedDataSettings1), (void**)&pDredSettings1);
		if (SUCCEEDED(hr))
		{
			pDredSettings1->SetAutoBreadcrumbsEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
			pDredSettings1->SetBreadcrumbContextEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
			pDredSettings1->SetPageFaultEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
			// LOG("[DRED 1.2] Device Removed Extended Data (Auto Breadcrumbs, Breadcrumb Context, Page Fault) are enabled");
		}
		else
		{
			// LOG("[DRED 1.2] Failed to enable DRED 1.2.");
		}
#endif

		// DLL読み込み
#ifdef _DEBUG
		SetDllDirectoryA("Library/assimp/build/lib/Debug");
		LoadLibraryExA("assimp-vc143-mtd.dll", NULL, NULL);
#else
		SetDllDirectoryA("Library/assimp/build/lib/Release");
		LoadLibraryExA("assimp-vc143-mt.dll", NULL, NULL);
#endif
		
		// DXGIFactory
		// 描画結果を画面に表示するまでのlayer
		ComPtr<IDXGIFactory3> factory;
		hr = CreateDXGIFactory2(dxgiFlags, IID_PPV_ARGS(&factory));
		if (FAILED(hr))
		{
			return false;
		}

		// hardware adapterの検索
		ComPtr<IDXGIAdapter1> useAdapter;
		{
			UINT adapterIndex = 0;
			ComPtr<IDXGIAdapter1> adapter;
			while (DXGI_ERROR_NOT_FOUND != factory->EnumAdapters1(adapterIndex, &adapter))
			{
				DXGI_ADAPTER_DESC1 desc1{};
				adapter->GetDesc1(&desc1);
				++adapterIndex;

				// softwareは除く
				if (desc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
				{
					continue;
				}

				// 利用できるhardwareか作って確認してみる
				hr = D3D12CreateDevice
				(
					adapter.Get(),
					D3D_FEATURE_LEVEL_12_0,
					__uuidof(ID3D12Device), nullptr
				);
				if (SUCCEEDED(hr))
				{
					break;
				}
			}

			// 利用するhardwareの確定
			adapter.As(&useAdapter);
		}

		// deviceの作成
		hr = D3D12CreateDevice(useAdapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_d3d12Device));
		if (FAILED(hr))
		{
			return false;
		}

		// adapter名の取得
		DXGI_ADAPTER_DESC1 adapterDesc{};
		useAdapter->GetDesc1(&adapterDesc);
		{
			std::vector<char> buf;
			auto ret = WideCharToMultiByte(CP_UTF8, 0, adapterDesc.Description, -1, nullptr, 0, nullptr, nullptr);
			if (ret > 0)
			{
				ret++;
				buf.resize(ret);
				ret = WideCharToMultiByte(CP_UTF8, 0, adapterDesc.Description, -1, buf.data(), int(buf.size()) - 1, nullptr, nullptr);
				m_adapterName = std::string(buf.data());
			}
		}

		// DXRのサポート確認
		D3D12_FEATURE_DATA_D3D12_OPTIONS5 options{};
		hr = m_d3d12Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &options, UINT(sizeof(options)));
		if (FAILED(hr) || options.RaytracingTier == D3D12_RAYTRACING_TIER_NOT_SUPPORTED)
		{
			throw std::runtime_error("DXR is not supported.");
		}

		// command queueの作成
		D3D12_COMMAND_QUEUE_DESC queueDesc
		{
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			0,
			D3D12_COMMAND_QUEUE_FLAG_NONE,
			0
		};
		hr = m_d3d12Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue));
		if (FAILED(hr))
		{
			return false;
		}

		// descriptorの準備
		if (PrepareDescriptor() == false)
		{
			return false;
		}

		// CommandAllocatorの準備
		if (PrepareCommandAllocators() == false)
		{
			return false;
		}

		// Globalデータを準備
		m_global = std::make_shared<GlobalSettings>();

		return true;
	}

	void Device::Destroy()
	{
		// GPUの処理待ち
		waitForIdleGpu();

		// eventのClose
		CloseHandle(m_fenceEvent); m_fenceEvent = 0;
		CloseHandle(m_waitEvent); m_waitEvent = 0;

		// descriptroheapの解放
		for (UINT i = 0; i < BACK_BUFFER_COUNT; ++i)
		{
			m_rtvHeap.Deallocate(&m_rtDescriptorArray[i]);
		}
		m_dsvHeap.Deallocate(&m_dsvDescriptor);
	}

	bool Device::CreateSwapchain(UINT width, UINT height, HWND hwnd)
	{
		HRESULT hr;
		m_width = width;
		m_height = height;

		if (m_swapchain == nullptr)
		{
			ComPtr<IDXGIFactory3> factory;
			hr = CreateDXGIFactory2(0, IID_PPV_ARGS(&factory));
			if (FAILED(hr))
			{
				return false;
			}

			// swapchainの生成
			DXGI_SWAP_CHAIN_DESC1 scDesc{};
			scDesc.BufferCount = BACK_BUFFER_COUNT;
			scDesc.Width = width;
			scDesc.Height = height;
			scDesc.Format = m_backbufferFormat;
			scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
			scDesc.SampleDesc.Count = 1;

			ComPtr<IDXGISwapChain1> swapchain;
			hr = factory->CreateSwapChainForHwnd
			(
				m_commandQueue.Get(),
				hwnd,
				&scDesc,
				nullptr,
				nullptr,
				&swapchain
			);
			if (FAILED(hr))
			{
				return false;
			}
			swapchain.As(&m_swapchain);

			// Fenceも用意
			if (PrepareFrameFences() == false)
			{
				return false;
			}
		}

		// RTVの作成
		if (PrepareRenderTargetView() == false)
		{
			return false;
		}

		// Depth Bufferの作成
		if (PrepareDepthBuffer(width, height) == false)
		{
			return false;
		}

		// DSVの作成
		if (PrepareDepthStencilView() == false)
		{
			return false;
		}

		// back bufferのindexを設定
		m_frameIndex = m_swapchain->GetCurrentBackBufferIndex();

		// Viewportの用意
		if (PrepareViewport(width, height) == false)
		{
			return false;
		}

		return true;
	}

	Device::ComPtr<ID3D12GraphicsCommandList4> Device::CreateCommandList()
	{
		ComPtr<ID3D12GraphicsCommandList4> commandList;
		auto allocator = GetCurrentCommandAllocator();

		m_d3d12Device->CreateCommandList
		(
			0,
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			allocator.Get(),
			nullptr,
			IID_PPV_ARGS(commandList.ReleaseAndGetAddressOf())
		);
		return commandList;
	}

	Device::ComPtr<ID3D12Device5> Device::GetDevice()
	{
		return m_d3d12Device;
	}
	
	Device::ComPtr<ID3D12CommandAllocator> Device::GetCurrentCommandAllocator()
	{
		return m_commandAllocatorArray[m_frameIndex];
	}

	Device::ComPtr<ID3D12CommandQueue> Device::GetDefaultQueue() const
	{
		return m_commandQueue;
	}

	UINT Device::GetCurrentFrameIndex() const
	{
		return m_frameIndex;
	}

	std::string Device::GetAdapterName() const
	{
		return m_adapterName;
	}

	const D3D12_VIEWPORT& Device::GetDefaultViewport() const
	{
		return m_viewport;
	}

	const D3D12_RECT& Device::GetDefaultScissorRect() const
	{
		return m_scissorRect;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE& Device::GetRTV()
	{
		return m_rtDescriptorArray[m_frameIndex].m_cpuHandle;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE& Device::GetDSV()
	{
		return m_dsvDescriptor.m_cpuHandle;
	}

	Device::ComPtr<ID3D12Resource> Device::GetRT()
	{
		return m_rtArray[m_frameIndex];
	}

	Device::ComPtr<ID3D12Resource> Device::GetDS()
	{
		return m_depthBuffer;
	}

	D3D12_HEAP_PROPERTIES Device::GetDefaultHeapProps() const
	{
		return m_defaultHeapProps;
	}

	D3D12_HEAP_PROPERTIES Device::GetUploadHeapProps() const
	{
		return m_uploadHeapProps;
	}

	Device::ComPtr<ID3D12Resource> Device::CreateBuffer(size_t size, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES initialState, D3D12_HEAP_TYPE heapType, const wchar_t* name)
	{
		D3D12_HEAP_PROPERTIES heapProps{};

		// heaptypeで分岐
		if (heapType == D3D12_HEAP_TYPE_DEFAULT)
		{
			heapProps = GetDefaultHeapProps();
		}
		if (heapType == D3D12_HEAP_TYPE_UPLOAD)
		{
			heapProps = GetUploadHeapProps();
		}

		HRESULT hr;
		ComPtr<ID3D12Resource> resource;

		// 簡単なbuffer設定
		D3D12_RESOURCE_DESC resourceDesc{};
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resourceDesc.Alignment = 0;
		resourceDesc.Width = size;
		resourceDesc.Height = 1;
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.MipLevels = 1;
		resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
		resourceDesc.SampleDesc = { 1,0 };
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		resourceDesc.Flags = flags;

		// 確保
		hr = m_d3d12Device->CreateCommittedResource
		(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			initialState,
			nullptr,
			IID_PPV_ARGS(&resource)
		);

		if (FAILED(hr))
		{
			OutputDebugStringA("CreateBuffer is failed\n");
		}

		if (resource != nullptr && name != nullptr)
		{
			resource->SetName(name);
		}

		return resource;
	}

	Device::ComPtr<ID3D12Resource> Device::CreateTexture2D
	(
		UINT width, UINT height, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES initialState, D3D12_HEAP_TYPE heapType,
		DXGI_SAMPLE_DESC sampleDesc, DirectX::XMVECTORF32 clearColor
	)
	{
		D3D12_HEAP_PROPERTIES heapProps{};

		// heaptypeで分岐
		if (heapType == D3D12_HEAP_TYPE_DEFAULT)
		{
			heapProps = GetDefaultHeapProps();
		}
		if (heapType == D3D12_HEAP_TYPE_UPLOAD)
		{
			heapProps = GetUploadHeapProps();
		}

		HRESULT hr;
		ComPtr<ID3D12Resource> resource;
		
		// 簡単なbuffer設定
		D3D12_RESOURCE_DESC resourceDesc{};
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		resourceDesc.Alignment = 0;
		resourceDesc.Width = width;
		resourceDesc.Height = height;
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.MipLevels = 0;
		resourceDesc.Format = format;
		resourceDesc.SampleDesc = sampleDesc;
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		resourceDesc.Flags = flags;
		
		// todo: rtvで見分け
		D3D12_CLEAR_VALUE clearValue{};
		if (flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
			)
		{
			clearValue.Format = format;
			memcpy(clearValue.Color, clearColor, sizeof(float) * 4);
		}
		else if (flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)
		{
			
			// TYPELESSは型決定のため分岐しておく
			clearValue.Format = format == DXGI_FORMAT_R32_TYPELESS ? DXGI_FORMAT_D32_FLOAT : format;
			clearValue.DepthStencil.Depth = 1.0f;
			clearValue.DepthStencil.Stencil = 0;
		}

		// 確保
		hr = m_d3d12Device->CreateCommittedResource
		(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			initialState,
			&clearValue,
			IID_PPV_ARGS(resource.ReleaseAndGetAddressOf())
		);

		if (FAILED(hr))
		{
			OutputDebugStringA("CreateBuffer is failed\n");
		}

		return resource;
	}

	Descriptor Device::CreateSRV(ComPtr<ID3D12Resource> resource, const D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc)
	{
		auto descriptor = AllocateDescriptor();
		m_d3d12Device->CreateShaderResourceView
		(
			resource.Get(),
			srvDesc,
			descriptor.m_cpuHandle
		);
		return descriptor;
	}

	Descriptor Device::CreateUAV(ComPtr<ID3D12Resource> resource, const D3D12_UNORDERED_ACCESS_VIEW_DESC* uavDesc)
	{
		auto descriptor = AllocateDescriptor();
		m_d3d12Device->CreateUnorderedAccessView
		(
			resource.Get(),
			nullptr,
			uavDesc,
			descriptor.m_cpuHandle
		);
		return descriptor;
	}
	
	Device::ComPtr<ID3D12Fence1> Device::CreateFence()
	{
		ComPtr<ID3D12Fence1> fence;
		m_d3d12Device->CreateFence
		(
			0,
			D3D12_FENCE_FLAG_NONE,
			IID_PPV_ARGS(fence.ReleaseAndGetAddressOf())
		);

		return fence;
	}

	void Device::ExecuteCommandList(ComPtr<ID3D12GraphicsCommandList4> command)
	{
		ID3D12CommandList* commandListArray[] =
		{
			command.Get(),
		};

		m_commandQueue->ExecuteCommandLists(1, commandListArray);
	}

	void Device::Present(UINT interval)
	{
		if (m_swapchain)
		{
			m_swapchain->Present(interval, 0);
			WaitAvailableFrame();
		}
	}

	void Device::waitForIdleGpu()
	{
		if (m_commandQueue)
		{
			auto commandList = CreateCommandList();
			auto waitFence = CreateFence();
			UINT64 fenceValue = 1;

			// CPU側でGPUの待機
			waitFence->SetEventOnCompletion(fenceValue, m_waitEvent);
			// gpu側で終了時にセットする値
			m_commandQueue->Signal(waitFence.Get(), fenceValue);

			WaitForSingleObject(m_waitEvent, INFINITE);
		}
	}

	void Device::WriteToHostVisibleMemory(ComPtr<ID3D12Resource> resource, const void* pData, size_t dataSize)
	{
		if (resource == nullptr)
		{
			return;
		}

		void* mapped = nullptr;
		D3D12_RANGE range{ 0, dataSize };
		HRESULT hr = resource->Map(0, &range, &mapped);

		if (SUCCEEDED(hr))
		{
			memcpy(mapped, pData, dataSize);
			resource->Unmap(0, &range);
		}
	}

	void Device::WriteToDefaultMemory(ComPtr<ID3D12Resource> resource, const void* pData, size_t dataSize)
	{
		if (resource == nullptr)
		{
			return;
		}

		ComPtr<ID3D12Resource> staging;
		HRESULT hr;
		auto heapProps = GetUploadHeapProps();

		// resourceを確保
		D3D12_RESOURCE_DESC resourceDesc{};
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resourceDesc.Width = dataSize;
		resourceDesc.Height = 1;
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.MipLevels = 1;
		resourceDesc.SampleDesc = { 1,0 };
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

		hr = m_d3d12Device->CreateCommittedResource
		(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(staging.ReleaseAndGetAddressOf())
		);

		if (FAILED(hr))
		{
			return;
		}

		void* mapped = nullptr;
		D3D12_RANGE range{ 0, dataSize };
		hr = staging->Map(0, &range, &mapped);
		if (SUCCEEDED(hr))
		{
			memcpy(mapped, pData, dataSize);
			staging->Unmap(0, &range);
		}

		// resource用のcommand, copyしていく
		auto command = CreateCommandList();
		command->CopyResource(resource.Get(), staging.Get());
		command->Close();

		// commandをqueueに投げ込んでGPUへ
		ExecuteCommandList(command);

		// 待機
		waitForIdleGpu();
	}

	Descriptor Device::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type)
	{
		Descriptor descriptor;

		if (type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
		{
			m_heap.Allocate(&descriptor);
		}

		if (type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER)
		{
			m_samplerHeap.Allocate(&descriptor);
		}

		if (type == D3D12_DESCRIPTOR_HEAP_TYPE_RTV)
		{
			m_rtvHeap.Allocate(&descriptor);
		}

		if (type == D3D12_DESCRIPTOR_HEAP_TYPE_DSV)
		{
			m_dsvHeap.Allocate(&descriptor);
		}

		return descriptor;
	}

	Descriptor Device::AllocateDescriptorTable(UINT size, D3D12_DESCRIPTOR_HEAP_TYPE type)
	{
		Descriptor descriptor;

		if (type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
		{
			m_heap.AllocateTable(size, &descriptor);
		}

		if (type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER)
		{
			m_samplerHeap.AllocateTable(size, &descriptor);
		}

		if (type == D3D12_DESCRIPTOR_HEAP_TYPE_RTV)
		{
			m_rtvHeap.AllocateTable(size, &descriptor);
		}

		if (type == D3D12_DESCRIPTOR_HEAP_TYPE_DSV)
		{
			m_dsvHeap.AllocateTable(size, &descriptor);
		}

		return descriptor;
	}

	void Device::DeallocateDescriptor(Descriptor& descriptor)
	{
		auto type = descriptor.m_heapType;

		if (type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
		{
			m_heap.Deallocate(&descriptor);
		}

		if (type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER)
		{
			m_samplerHeap.Deallocate(&descriptor);
		}

		if (type == D3D12_DESCRIPTOR_HEAP_TYPE_RTV)
		{
			m_rtvHeap.Deallocate(&descriptor);
		}

		if (type == D3D12_DESCRIPTOR_HEAP_TYPE_DSV)
		{
			m_dsvHeap.Deallocate(&descriptor);
		}
	}

	void Device::DeallocateDescriptorTable(UINT size, Descriptor& descriptor)
	{
		auto type = descriptor.m_heapType;

		if (type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
		{
			m_heap.DeallocateTable(size, &descriptor);
		}

		if (type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER)
		{
			m_samplerHeap.DeallocateTable(size, &descriptor);
		}

		if (type == D3D12_DESCRIPTOR_HEAP_TYPE_RTV)
		{
			m_rtvHeap.DeallocateTable(size, &descriptor);
		}

		if (type == D3D12_DESCRIPTOR_HEAP_TYPE_DSV)
		{
			m_dsvHeap.DeallocateTable(size, &descriptor);
		}
	}

	Device::ComPtr<ID3D12DescriptorHeap> Device::GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type)
	{
		if (type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
		{
			return m_heap.GetHeap();
		}

		if (type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER)
		{
			return m_samplerHeap.GetHeap();
		}

		if (type == D3D12_DESCRIPTOR_HEAP_TYPE_RTV)
		{
			return m_rtvHeap.GetHeap();
		}

		if (type == D3D12_DESCRIPTOR_HEAP_TYPE_DSV)
		{
			return m_dsvHeap.GetHeap();
		}

		return nullptr;
	}

	// Descriptorの準備
	bool Device::PrepareDescriptor()
	{
		HRESULT hr;

		// descriptor heap(RTV)
		ComPtr<ID3D12DescriptorHeap> heap;
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc
		{
			D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
			RENDER_TARGET_VIEW_MAX,
			D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
			0
		};
		hr = m_d3d12Device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(heap.ReleaseAndGetAddressOf()));
		if (FAILED(hr))
		{
			return false;
		}
		m_rtvHeap.Intialize(heap, m_d3d12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));

		// descriptor heap(DSV)
		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc
		{
			D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
			DEPTH_STENCIL_VIEW_MAX,
			D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
			0
		};
		hr = m_d3d12Device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(heap.ReleaseAndGetAddressOf()));
		if (FAILED(hr))
		{
			return false;
		}
		m_dsvHeap.Intialize(heap, m_d3d12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV));

		// descriptor heap(SRV/CBV/UAVなど)
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc
		{
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
			SHADE_RESOURCE_VIEW_MAX,
			D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
			0
		};
		hr = m_d3d12Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(heap.ReleaseAndGetAddressOf()));
		if (FAILED(hr))
		{
			return false;
		}
		m_heap.Intialize(heap, m_d3d12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));

		// descriptor heap(Sampler)
		D3D12_DESCRIPTOR_HEAP_DESC samplerDesc
		{
			D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
			SAMPLER_VIEW_MAX,
			D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
			0
		};
		hr = m_d3d12Device->CreateDescriptorHeap(&samplerDesc, IID_PPV_ARGS(heap.ReleaseAndGetAddressOf()));
		if (FAILED(hr))
		{
			return false;
		}
		m_samplerHeap.Intialize(heap, m_d3d12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER));

		return true;
	}
	
	// RTVの準備
	bool Device::PrepareRenderTargetView()
	{
		for (UINT i = 0; i < BACK_BUFFER_COUNT; ++i)
		{
			m_swapchain->GetBuffer(i, IID_PPV_ARGS(m_rtArray[i].ReleaseAndGetAddressOf()));
			m_rtArray[i]->SetName(L"SwapchainFrame");

			D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
			rtvDesc.Format = m_backbufferFormat;
			rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

			m_rtvHeap.Allocate(&m_rtDescriptorArray[i]);
			auto rtvHandle = m_rtDescriptorArray[i].m_cpuHandle;
			m_d3d12Device->CreateRenderTargetView(m_rtArray[i].Get(), &rtvDesc, rtvHandle);
		}

		return true;
	}

	// DepthBufferの準備
	bool Device::PrepareDepthBuffer(int width, int height)
	{
		HRESULT hr;

		D3D12_RESOURCE_DESC depthDesc{};
		depthDesc.Format = m_depthFormat;
		depthDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		depthDesc.Width = width;
		depthDesc.Height = height;
		depthDesc.DepthOrArraySize = 1;
		depthDesc.MipLevels = 1;
		depthDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		depthDesc.SampleDesc.Count = 1;

		D3D12_CLEAR_VALUE clearValue{};
		clearValue.Format = m_depthFormat;
		clearValue.DepthStencil.Depth = 1.0f;
		clearValue.DepthStencil.Stencil = 0;

		hr = m_d3d12Device->CreateCommittedResource
		(
			&m_defaultHeapProps,
			D3D12_HEAP_FLAG_NONE,
			&depthDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&clearValue,
			IID_PPV_ARGS(m_depthBuffer.ReleaseAndGetAddressOf())
		);
		if (FAILED(hr))
		{
			return false;
		}

		m_depthBuffer->SetName(L"DepthBuffer(Default)");

		return true;
	}

	bool Device::PrepareDepthStencilView()
	{
		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
		dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
		dsvDesc.Texture2D.MipSlice = 0;

		// dsv用のdescriptorを確保
		m_dsvDescriptor = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

		// dsvを作成
		m_d3d12Device->CreateDepthStencilView(m_depthBuffer.Get(), &dsvDesc, m_dsvDescriptor.m_cpuHandle);

		return true;
	}

	bool Device::PrepareCommandAllocators()
	{
		HRESULT hr;

		// BackBuffer分を生成
		for (UINT i = 0; i < BACK_BUFFER_COUNT; ++i)
		{
			hr = m_d3d12Device->CreateCommandAllocator
			(
				D3D12_COMMAND_LIST_TYPE_DIRECT,
				IID_PPV_ARGS(m_commandAllocatorArray[i].ReleaseAndGetAddressOf())
			);

			// 生成失敗
			if (FAILED(hr))
			{
				return false;
			}

			m_fenceEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
			m_waitEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
		}

		return true;
	}

	bool Device::PrepareFrameFences()
	{
		HRESULT hr;

		// BackBuffer分を生成
		for (UINT i = 0; i < BACK_BUFFER_COUNT; ++i)
		{
			m_fenceValueArray[i] = 0;
			hr = m_d3d12Device->CreateFence(
				m_fenceValueArray[i],
				D3D12_FENCE_FLAG_NONE,
				IID_PPV_ARGS(m_frameFenceArray[i].ReleaseAndGetAddressOf())
			);
			if (FAILED(hr))
			{
				return false;
			}
		}

		return true;
	}

	bool Device::PrepareViewport(int width, int height)
	{
		m_viewport.TopLeftX = 0.0f;
		m_viewport.TopLeftY = 0.0f;
		m_viewport.Width = float(width);
		m_viewport.Height = float(height);
		m_viewport.MinDepth = 0.0f;
		m_viewport.MaxDepth = 1.0f;

		m_scissorRect.left = 0;
		m_scissorRect.right = width;
		m_scissorRect.top = 0;
		m_scissorRect.bottom = height;

		return true;
	}

	void Device::WaitAvailableFrame()
	{
		// fenceの挿入
		auto fence = m_frameFenceArray[m_frameIndex];
		auto value = ++m_fenceValueArray[m_frameIndex];
		m_commandQueue->Signal(fence.Get(), value);

		m_frameIndex = m_swapchain->GetCurrentBackBufferIndex();
		fence = m_frameFenceArray[m_frameIndex];
		auto finishValue = m_fenceValueArray[m_frameIndex];

		if (fence->GetCompletedValue() < finishValue)
		{
			// 待ち
			fence->SetEventOnCompletion(finishValue, m_fenceEvent);
			WaitForSingleObject(m_fenceEvent, INFINITE);
		}
	}

//@todo usingで分ける
#pragma region Resource
	/**
	* Textureの生成
	*/
	TextureResource LoadTextureFromFile(std::weak_ptr<Device> inDevice, const std::wstring& fileName)
	{
		auto deviceData = inDevice.lock();
		if (deviceData == nullptr){ throw std::runtime_error("device is invalid."); }

		DirectX::TexMetadata metadata;
		DirectX::ScratchImage image;

		//ファイル読み込み
		HRESULT hr = E_FAIL;
		const std::wstring extDDS(L"dds");
		const std::wstring extPNG(L"png");
		const std::wstring extHDR(L"hdr");
		const std::wstring extTGA(L"tga");
		const std::wstring extJPG(L"jpg");
		if (fileName.length() < 3) {
			throw std::runtime_error("texture filename is invalid.");
		}

		if (std::equal(std::rbegin(extDDS), std::rend(extDDS), std::rbegin(fileName))) {
			hr = LoadFromDDSFile(fileName.c_str(), DirectX::DDS_FLAGS_NONE, &metadata, image);
		}
		if (std::equal(std::rbegin(extPNG), std::rend(extPNG), std::rbegin(fileName))) {
			hr = LoadFromWICFile(fileName.c_str(), DirectX::WIC_FLAGS_NONE, &metadata, image);
		}
		if (std::equal(std::rbegin(extHDR), std::rend(extHDR), std::rbegin(fileName)))
		{
			hr = LoadFromHDRFile(fileName.c_str(), &metadata, image);
		}
		if (std::equal(std::rbegin(extTGA), std::rend(extTGA), std::rbegin(fileName)))
		{
			hr = DirectX::LoadFromTGAFile(fileName.c_str(), &metadata, image);
		}
		if (std::equal(std::rbegin(extJPG), std::rend(extJPG), std::rbegin(fileName)))
		{
			hr = DirectX::LoadFromWICFile(fileName.c_str(), DirectX::WIC_FLAGS_NONE, &metadata, image);
		}

		if (FAILED(hr))
		{
			throw std::runtime_error("Failed Texture Load.");
		}

		//texture resourceの確保
		ComPtr<ID3D12Resource> texRes;
		ComPtr<ID3D12Device> device;
		deviceData->GetDevice().As(&device);
		CreateTexture(device.Get(), metadata, &texRes);
		texRes->SetName(fileName.c_str());

		//転送のためのstagingデータの用意
		ComPtr<ID3D12Resource> srcBuffer;
		std::vector<D3D12_SUBRESOURCE_DATA> subresources;
		PrepareUpload(device.Get(), image.GetImages(), image.GetImageCount(), metadata, subresources);
		const auto totalBytes = GetRequiredIntermediateSize(texRes.Get(), 0, UINT(subresources.size()));

		auto staging = deviceData->CreateBuffer(
			totalBytes, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_HEAP_TYPE_UPLOAD);
		staging->SetName(L"Tex-Staging");

		//commandを実行してtexture dataの転送を行う
		auto command = deviceData->CreateCommandList();
		UpdateSubresources(
			command.Get(), texRes.Get(), staging.Get(), 0, 0, UINT(subresources.size()), subresources.data());
		auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(texRes.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		command->ResourceBarrier(1, &barrier);
		command->Close();

		// 転送開始と処理完了を待つ.
		deviceData->ExecuteCommandList(command);
		deviceData->waitForIdleGpu();

		//shader resource viewの生成
		DXUTILITY::TextureResource ret;
		ret.res = texRes;
		ret.srv = deviceData->AllocateDescriptor();
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Format = metadata.format;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		if (metadata.IsCubemap()) {
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
			srvDesc.TextureCube.MipLevels = UINT(metadata.mipLevels);
			srvDesc.TextureCube.MostDetailedMip = 0;
			srvDesc.TextureCube.ResourceMinLODClamp = 0;
		}
		else {
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.ResourceMinLODClamp = 0;
		}

		deviceData->GetDevice()->CreateShaderResourceView(texRes.Get(), &srvDesc, ret.srv.m_cpuHandle);

		return ret;
	}

	/**
	* UAV Bufferの生成
	*/
	ComPtr<ID3D12Resource> CreateBuffer(std::weak_ptr<Device> device, size_t requestSize, const void* initData, D3D12_HEAP_TYPE heapType, D3D12_RESOURCE_FLAGS flags, const wchar_t* name)
	{
		auto deviceData = device.lock();
		if (deviceData == nullptr) { return nullptr; }
		
		//stateの決定
		auto initialState = D3D12_RESOURCE_STATE_COPY_DEST;
		if (heapType == D3D12_HEAP_TYPE_UPLOAD) {
			initialState = D3D12_RESOURCE_STATE_GENERIC_READ;
		}

		//resourceの生成+名前付け
		auto resource = deviceData->CreateBuffer(
			requestSize, flags, initialState, heapType, name);
		if (resource && name != nullptr) {
			resource->SetName(name);
		}

		//リソースの書き込み
		if (initData != nullptr) {
			if (heapType == D3D12_HEAP_TYPE_DEFAULT) {
				deviceData->WriteToDefaultMemory(resource, initData, requestSize);
			}
			if (heapType == D3D12_HEAP_TYPE_UPLOAD) {
				deviceData->WriteToHostVisibleMemory(resource, initData, requestSize);
			}
		}

		//リソースデータを返す
		return resource;
	}

	ComPtr<ID3D12Resource> CreateBufferUAV(std::weak_ptr<Device> device, size_t requestSize, D3D12_RESOURCE_STATES initialState, const wchar_t* name)
	{
		auto deviceData = device.lock();
		if (deviceData == nullptr) { return nullptr; }

		// flagを変える
		auto flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		return deviceData->CreateBuffer(
			requestSize, flags,
			initialState, D3D12_HEAP_TYPE_DEFAULT
		);
	}

	/**
	* Constant Bufferの生成
	*/
	ComPtr<ID3D12Resource> CreateConstantBuffer(std::weak_ptr<Device> device, size_t totalSize)
	{
		auto deviceData = device.lock();
		if (deviceData == nullptr) { return nullptr; }

		totalSize = Utility::RoundUp(totalSize, 256);
		return deviceData->CreateBuffer(
			totalSize,
			D3D12_RESOURCE_FLAG_NONE,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			D3D12_HEAP_TYPE_UPLOAD
		);
	}

	/**
	* UAV生成(フォーマット指定)
	*/
	Descriptor CreateStructuredUAV(std::weak_ptr<Device> device, ComPtr<ID3D12Resource> resource, UINT numElements, UINT firstElement, DXGI_FORMAT format)
	{
		auto deviceData = device.lock();
		if (deviceData == nullptr) { std::runtime_error("Error: device is removed."); }

		//UAVDescの生成
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		uavDesc.Format = format;
		uavDesc.Buffer.NumElements = numElements;
		uavDesc.Buffer.FirstElement = firstElement;

		//UAV生成
		return deviceData->CreateUAV(resource, &uavDesc);
	}

	/**
	* UAV生成(ストライド指定)
	*/
	Descriptor CreateStructuredUAV(std::weak_ptr<Device> device, ComPtr<ID3D12Resource> resource, UINT numElements, UINT firstElement, UINT stride)
	{
		auto deviceData = device.lock();
		if (deviceData == nullptr) { std::runtime_error("Error: device is removed."); }

		//uavDescの生成
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		uavDesc.Buffer.NumElements = numElements;
		uavDesc.Buffer.FirstElement = firstElement;
		uavDesc.Buffer.StructureByteStride = stride;

		// strideによる分岐
		if (stride == 0)
		{
			uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
			uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
		}
		else
		{
			uavDesc.Format = DXGI_FORMAT_UNKNOWN;
			uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
		}

		//UAVの生成
		return deviceData->CreateUAV(resource, &uavDesc);
	}

	/**
	* StructuredBuffer用のSRV生成(フォーマット指定)
	*/
	Descriptor CreateStructuredSRV(std::weak_ptr<Device> device, ComPtr<ID3D12Resource> resource, UINT numElements, UINT firstElement, DXGI_FORMAT format)
	{
		auto deviceData = device.lock();
		if (deviceData == nullptr) { std::runtime_error("Error: device is removed."); }

		//SRVDescの生成
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		srvDesc.Format = format;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Buffer.NumElements = numElements;
		srvDesc.Buffer.FirstElement = firstElement;

		//SRV生成
		return deviceData->CreateSRV(resource, &srvDesc);
	}

	/**
	* StructuredBuffer用のSRV生成(ストライド指定)
	*/
	Descriptor CreateStructuredSRV(std::weak_ptr<Device> device, ComPtr<ID3D12Resource> resource, UINT numElements, UINT firstElement, UINT stride)
	{
		auto deviceData = device.lock();
		if (deviceData == nullptr) { std::runtime_error("Error: device is removed."); }

		//SRVDescの生成
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Buffer.NumElements = numElements;
		srvDesc.Buffer.FirstElement = firstElement;
		srvDesc.Buffer.StructureByteStride = stride;

		//SRVの生成
		return deviceData->CreateSRV(resource, &srvDesc);
	}

	/**
	* Shader Tableの書き込み(Id)
	*/
	UINT WriteShaderIdentifier(void* dst, const void* shaderId)
	{
		memcpy(dst, shaderId, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
		return D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
	}

	/**
	* Shader Tableの書き込み(Descriptor)
	*/
	UINT WriteGPUDescriptor(void* dst, const Descriptor& descriptor)
	{
		auto handle = descriptor.m_gpuHandle;
		memcpy(dst, &handle, sizeof(handle));
		return UINT(sizeof(handle));
	}

	/**
	* Shader Tableの書き込み(Constant Buffer)
	*/
	UINT WriteGpuResourceAddr(void* dst, const ComPtr<ID3D12Resource>& res)
	{
		D3D12_GPU_VIRTUAL_ADDRESS addr = res->GetGPUVirtualAddress();
		memcpy(dst, &addr, sizeof(D3D12_GPU_VIRTUAL_ADDRESS));
		return UINT(sizeof(D3D12_GPU_VIRTUAL_ADDRESS));
	}

	/**
	* Shader Tableの書き込み(Constant Buffer)
	*/
	UINT WriteGpuResourceAddr(void* dst, const D3D12_GPU_VIRTUAL_ADDRESS& addr)
	{
		memcpy(dst, &addr, sizeof(addr));
		return UINT(sizeof(addr));
	}
#pragma endregion

#pragma region DynamicResource
	DynamicConstantBuffer::~DynamicConstantBuffer()
	{
	}

	/**
	* 初期化(Back Buffer分)
	*/
	bool DynamicConstantBuffer::Initialize(std::weak_ptr<Device> device, UINT requestSize, const wchar_t* name)
	{
		auto deviceData = device.lock();
		if (deviceData == nullptr) { return false; }

		requestSize = Utility::RoundUp(requestSize, 256);
		UINT count = deviceData->BACK_BUFFER_COUNT;
		m_resources.resize(count);

		for (UINT i = 0; i < count; ++i) {
			//BackBufferの数だけ用意
			m_resources[i] = deviceData->CreateBuffer(
				requestSize,
				D3D12_RESOURCE_FLAG_NONE,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				D3D12_HEAP_TYPE_UPLOAD);
			if (m_resources[i]) {
				m_resources[i]->SetName(name);
			}
			else {
				return false;
			}
		}

		m_isInitialize = true;
		return true;
	}

	/**
	* 書き込み
	*/
	void DynamicConstantBuffer::Write(UINT bufferIndex, const void* src, UINT size)
	{
		auto& buffer = m_resources[bufferIndex];
		void* dst = nullptr;
		//書き込み
		buffer->Map(0, nullptr, &dst);
		if (dst) {
			memcpy(dst, src, size);
			buffer->Unmap(0, nullptr);
		}
	}

	/**
	* デストラクタ(descriptorの解放)
	*/
	DynamicBuffer::~DynamicBuffer()
	{
	}
	
	/**
	 * 初期化(Back Buffer分)
	 */
	bool DynamicBuffer::Initialize(std::weak_ptr<Device> device, UINT requestSize, const wchar_t* name)
	{
		auto deviceData = device.lock();
		if (deviceData == nullptr) { return false; }

		m_bufferSize = Utility::RoundUp(requestSize, 256);
		UINT count = deviceData->BACK_BUFFER_COUNT;
		m_resources.resize(count);

		for (UINT i = 0; i < count; ++i) {
			m_resources[i] = deviceData->CreateBuffer(
				m_bufferSize,
				D3D12_RESOURCE_FLAG_NONE,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				D3D12_HEAP_TYPE_UPLOAD);

			// Resource確保チェック
			if (m_resources[i] == nullptr)
			{
				return false;
			}
			
			// 名前を設定
			m_resources[i]->SetName(name);
		}
		m_descriptors.resize(count);
		
		// descriptorの取得
		for (UINT i = 0; i < count; ++i) {
			m_descriptors[i] = deviceData->AllocateDescriptor();
		}

		m_isInitialize = true;
		return true;
	}

	bool DynamicBuffer::UnInitialize(std::weak_ptr<Device> device)
	{
		auto deviceData = device.lock();
		if (deviceData == nullptr) { return false; }

		for (auto& desc : m_descriptors)
		{
			deviceData->DeallocateDescriptor(desc);
		}

		return true;
	}

	/**
	 * BufferViewを用意する必要がある場合の処理
	 */
	void DynamicBuffer::SetupBufferView(std::weak_ptr<Device> device)
	{
		auto deviceData = device.lock();
		if (deviceData == nullptr) { return; }

		// size的に初期化されているかのチェック
		if (m_bufferSize == 0)
		{
			std::runtime_error("Buffer size isn't Calculated.");
		}

		UINT count = deviceData->BACK_BUFFER_COUNT;

		// Buffer Viewの生成
		for (UINT i = 0; i < count; ++i) {
			D3D12_CONSTANT_BUFFER_VIEW_DESC cbDesc{};
			cbDesc.BufferLocation = m_resources[i]->GetGPUVirtualAddress();
			cbDesc.SizeInBytes = m_bufferSize;
			deviceData->GetDevice()->CreateConstantBufferView(&cbDesc, m_descriptors[i].m_cpuHandle);
		}
	}

	/**
	 * Mapを行う(Index指定)
	 */
	void* DynamicBuffer::Map(UINT bufferIndex)
	{
		void* p = nullptr;
		m_resources[bufferIndex]->Map(0, nullptr, &p);
		return p;
	}

	/**
	 * UnMapを行う(Index指定)
	 */
	void DynamicBuffer::Unmap(UINT bufferIndex)
	{
		m_resources[bufferIndex]->Unmap(0, nullptr);
	}

	// SamplerのDescriptorを解除
	SamplerDescriptor::~SamplerDescriptor()
	{
	}

	// Samplerの用意
	bool SamplerDescriptor::Initialize(Device* device, const wchar_t* name)
	{
		// descriptorの取得
		m_descriptor = device->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

		// SamplerのDescを用意
		D3D12_SAMPLER_DESC samplerDesc{};
		
		samplerDesc.Filter = D3D12_ENCODE_BASIC_FILTER
		(
			D3D12_FILTER_TYPE_LINEAR, // min
			D3D12_FILTER_TYPE_LINEAR, // mag
			D3D12_FILTER_TYPE_LINEAR, // mip
			D3D12_FILTER_REDUCTION_TYPE_STANDARD // reduction
		);

		samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplerDesc.MaxLOD = FLT_MAX;
		samplerDesc.MinLOD = -FLT_MAX;
		samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;

		device->GetDevice()->CreateSampler(&samplerDesc, m_descriptor.m_cpuHandle);

		return true;
	}
	bool SamplerDescriptor::UnInitialize(Device* device)
	{
		if (device == nullptr) {
			return false;
		}

		device->DeallocateDescriptor(m_descriptor);
		return true;
	}
#pragma endregion

#pragma region Text
	// UTF-8に変換
	std::string ToUTF8(const std::wstring& value)
	{
		auto length = WideCharToMultiByte(CP_UTF8, 0U, value.data(), -1, nullptr, 0, nullptr, nullptr);
		auto buffer = new char [length];

		WideCharToMultiByte(CP_UTF8, 0U, value.data(), -1, buffer, length, nullptr, nullptr);

		std::string result(buffer);
		delete[] buffer; buffer = nullptr;
		return result;
	}

	// string -> wstring
	std::wstring ToWstring(const std::string& value)
	{
		// SJIS → wstring
		int iBufferSize = MultiByteToWideChar(CP_ACP, 0, value.c_str(), -1, (wchar_t*)NULL, 0);
		wchar_t* cpUCS2 = new wchar_t[iBufferSize];
		MultiByteToWideChar(CP_ACP, 0, value.c_str(), -1, cpUCS2, iBufferSize);
		std::wstring oRet(cpUCS2, cpUCS2 + iBufferSize - 1);
		delete[] cpUCS2;
		return oRet;
	}
#pragma endregion
}
