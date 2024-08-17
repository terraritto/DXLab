#pragma once
#include <d3d12.h>
#include <dxgi1_5.h>
#include <wrl.h>

#include <string>
#include <memory>
#include <vector>
#include <list>
#include <array>
#include <functional>
#include <unordered_map>

#include "TypeUtility.h"
#include "../GlobalSettings.h"

struct DefaultMesh;
struct aiMesh;
struct aiMaterial;
class MaterialBase;

namespace DXUTILITY
{
	// ディスクリプタの格納
	struct Descriptor
	{
		UINT m_heapBaseOffset;
		D3D12_CPU_DESCRIPTOR_HANDLE m_cpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE m_gpuHandle;
		D3D12_DESCRIPTOR_HEAP_TYPE m_heapType;

		// constructor
		Descriptor()
			: m_heapBaseOffset(0)
			, m_cpuHandle()
			, m_gpuHandle()
			, m_heapType(D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES)
		{
		}

		// heap typeが決まっているか？
		bool IsInvalid() const
		{
			return m_heapType == D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES;
		}
	};

	class DescriptorHeapManager
	{
	public:
		template<class T>
		using ComPtr = Microsoft::WRL::ComPtr<T>;

	public:
		// Constructor
		DescriptorHeapManager() = default;
		DescriptorHeapManager(const DescriptorHeapManager&) = delete;
		DescriptorHeapManager& operator=(const DescriptorHeapManager&) = delete;

	public:
		// 初期化処理
		void Intialize(ComPtr<ID3D12DescriptorHeap> heap, UINT incrementSize)
		{
			m_heap = heap;
			m_incrementSize = incrementSize;
		}

	public:
		// *--- Descriptor 単体確保処理 START---*
		// ディスクリプタを確保する.
		void Allocate(Descriptor* desc);

		// ディスクリプタの解放処理.
		void Deallocate(Descriptor* desc);
		// *--- Descriptor 単体確保処理 END---*

		// *--- Descriptor 連続確保処理 START---*
		// 指定した個数が連続保証ありで確保する.
		// 返却されるのは先頭ディスクリプタのみ.
		void AllocateTable(UINT count, Descriptor* descs);

		// 連続保証ありで確保したものの解放処理.
		void DeallocateTable(UINT count, Descriptor* descs);
		// *--- Descriptor 連続確保処理 END---*

		ComPtr<ID3D12DescriptorHeap> GetHeap() { return m_heap; }


	private:
		UINT m_allocateIndex = 0;
		UINT m_incrementSize = 0;
		ComPtr<ID3D12DescriptorHeap> m_heap;

		using DescriptorList = std::list<Descriptor>;
		std::unordered_map<UINT, DescriptorList> m_freeMap;
	};

	class Device
	{
	public:
		template<class T>
		using ComPtr = Microsoft::WRL::ComPtr<T>;

	public:
		// Constant Value
		static constexpr UINT BACK_BUFFER_COUNT = 3;
		static constexpr UINT RENDER_TARGET_VIEW_MAX = 64;
		static constexpr UINT DEPTH_STENCIL_VIEW_MAX = 64;
		static constexpr UINT SHADE_RESOURCE_VIEW_MAX = 1024;
		static constexpr UINT SAMPLER_VIEW_MAX = 64;

	public:
		// constructor関連(コピーは禁止)
		Device();
		Device(const Device&) = delete;
		Device& operator=(const Device&) = delete;
		~Device();

	public:
		bool Initialize();
		void Destroy();

	public:
		// deviceInit関係
		bool CreateSwapchain(UINT width, UINT height, HWND hwnd);

		ComPtr<ID3D12GraphicsCommandList4> CreateCommandList();
		ComPtr<ID3D12Device5> GetDevice();
		ComPtr<ID3D12CommandAllocator> GetCurrentCommandAllocator();
		ComPtr<ID3D12CommandQueue> GetDefaultQueue() const;
		UINT GetCurrentFrameIndex() const;
		std::string GetAdapterName() const;

		// viewportとScissor
		const D3D12_VIEWPORT& GetDefaultViewport() const;
		const D3D12_RECT& GetDefaultScissorRect() const;

		// memory関係
		D3D12_CPU_DESCRIPTOR_HANDLE& GetRTV();
		D3D12_CPU_DESCRIPTOR_HANDLE& GetDSV();
		ComPtr<ID3D12Resource> GetRT();
		ComPtr<ID3D12Resource> GetDS();

		// ScreenSize
		UINT GetScreenWidth() const { return m_width; }
		UINT GetScreenHeight() const { return m_height; }

		D3D12_HEAP_PROPERTIES GetDefaultHeapProps() const;
		D3D12_HEAP_PROPERTIES GetUploadHeapProps() const;

		ComPtr<ID3D12Resource> CreateBuffer(size_t size, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES initialState, D3D12_HEAP_TYPE heapType, const wchar_t* name = nullptr);
		ComPtr<ID3D12Resource> CreateTexture2D(UINT width, UINT height, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES initialState, D3D12_HEAP_TYPE heapType, DXGI_SAMPLE_DESC sampleDesc = DXGI_SAMPLE_DESC{ 1, 0 }, DirectX::XMVECTORF32 clearColor = DirectX::g_XMZero);

		Descriptor CreateSRV(ComPtr<ID3D12Resource> resource, const D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc);
		Descriptor CreateUAV(ComPtr<ID3D12Resource> resource, const D3D12_UNORDERED_ACCESS_VIEW_DESC* uavDesc);

		// GPUの処理関係
		ComPtr<ID3D12Fence1> CreateFence();
		void ExecuteCommandList(ComPtr<ID3D12GraphicsCommandList4> command);
		void Present(UINT interval);
		void waitForIdleGpu();

		// write
		void WriteToHostVisibleMemory(ComPtr<ID3D12Resource> resource, const void* pData, size_t dataSize);
		void WriteToDefaultMemory(ComPtr<ID3D12Resource> resource, const void* pData, size_t dataSize);

		// descriptor
		Descriptor AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		Descriptor AllocateDescriptorTable(UINT size, D3D12_DESCRIPTOR_HEAP_TYPE type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		void DeallocateDescriptor(Descriptor& descriptor);
		void DeallocateDescriptorTable(UINT size, Descriptor& descriptor);
		
		ComPtr<ID3D12DescriptorHeap> GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type);

		std::shared_ptr<GlobalSettings> m_global;

	protected:
		// 準備系
		bool PrepareDescriptor();
		bool PrepareRenderTargetView();
		bool PrepareDepthBuffer(int width, int height);
		bool PrepareDepthStencilView();
		bool PrepareCommandAllocators();
		bool PrepareFrameFences();
		bool PrepareViewport(int width, int height);

	private:
		void WaitAvailableFrame();

	private:
		// Device関係
		ComPtr<ID3D12Device5> m_d3d12Device;
		std::string m_adapterName;
		ComPtr<ID3D12CommandQueue> m_commandQueue;
		std::array<ComPtr<ID3D12CommandAllocator>, BACK_BUFFER_COUNT> m_commandAllocatorArray;
		ComPtr<IDXGISwapChain3> m_swapchain;
	
		// event
		HANDLE m_fenceEvent = 0;
		HANDLE m_waitEvent = 0;
	
	private:
		// Format
		DXGI_FORMAT m_backbufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		DXGI_FORMAT m_depthFormat = DXGI_FORMAT_D32_FLOAT;

	private:
		// メモリ関係
		// Descriptor
		DescriptorHeapManager m_rtvHeap;
		DescriptorHeapManager m_dsvHeap;
		DescriptorHeapManager m_heap; // CBV/SRV/UAVなど
		DescriptorHeapManager m_samplerHeap; // sampler用

		// descriptor
		std::array<ComPtr<ID3D12Resource>, BACK_BUFFER_COUNT> m_rtArray;
		std::array<Descriptor, BACK_BUFFER_COUNT> m_rtDescriptorArray;
		ComPtr<ID3D12Resource> m_depthBuffer;
		Descriptor m_dsvDescriptor;

		// fence
		std::array<ComPtr<ID3D12Fence1>, BACK_BUFFER_COUNT> m_frameFenceArray;
		std::array<UINT64, BACK_BUFFER_COUNT> m_fenceValueArray;
	
		// heapの設定
		D3D12_HEAP_PROPERTIES m_defaultHeapProps;
		D3D12_HEAP_PROPERTIES m_uploadHeapProps;

		// frameの位置
		UINT m_frameIndex = 0;

	private:
		// 画面サイズ
		UINT m_width;
		UINT m_height;

		// 画面周り
		D3D12_VIEWPORT m_viewport;
		D3D12_RECT m_scissorRect;
	};

#pragma region Resource
	//--- Texture関係
	// Texの構造体
	struct TextureResource
	{
		ComPtr<ID3D12Resource> res;
		DXUTILITY::Descriptor srv;
	};

	// Textureの読み込み(ファイル名指定)
	TextureResource LoadTextureFromFile(std::weak_ptr<Device> device, const std::wstring& fileName);

	//--- Resource関係
	// 動的なConstantbufferの生成
	class DynamicConstantBuffer {
	public:
		using ResourceType = ComPtr<ID3D12Resource>;

		~DynamicConstantBuffer();

		bool Initialize(std::weak_ptr<Device> device, UINT requestSize, const wchar_t* name = L"");

		//書き込み
		void Write(UINT bufferIndex, const void* src, UINT size);

		bool IsInitialize() const { return m_isInitialize; }

		//Getter(Resource)
		ResourceType Get(UINT bufferIndex) const { return m_resources[bufferIndex]; }
	private:
		std::vector<ResourceType> m_resources;
		bool m_isInitialize = false;
	};

	// 動的なbufferの生成
	class DynamicBuffer {
	public:
		//型の決定など
		using ResourceType = ComPtr<ID3D12Resource>;
		using Device = DXUTILITY::Device;
		~DynamicBuffer();

		bool Initialize(std::weak_ptr<Device> device, UINT requestSize, const wchar_t* name = L"");
		bool UnInitialize(std::weak_ptr<Device> device);

		bool IsInitialize() const { return m_isInitialize; }

		// BufferViewを用意する必要がある場合の処理
		void SetupBufferView(std::weak_ptr<Device> device);

		//メモリ確保系
		void* Map(UINT bufferIndex);
		void Unmap(UINT bufferIndex);

		//Getter
		//Resource
		ResourceType Get(UINT bufferIndex) const { return m_resources[bufferIndex]; }
		//Descriptor
		DXUTILITY::Descriptor GetDescriptor(UINT bufferIndex) const { return m_descriptors[bufferIndex]; }
	private:
		std::vector<ResourceType> m_resources;
		std::vector<DXUTILITY::Descriptor> m_descriptors;

		UINT m_bufferSize = 0;
		bool m_isInitialize = false;
	};

	// Sampler用Descriptor
	class SamplerDescriptor {
	public:
		//型の決定など
		using ResourceType = ComPtr<ID3D12Resource>;
		using Device = DXUTILITY::Device;
		~SamplerDescriptor();

		bool Initialize(Device* device, const wchar_t* name = L"");
		bool UnInitialize(Device* device);

		//Descriptor
		DXUTILITY::Descriptor GetDescriptor() const { return m_descriptor; }
	
	private:
		DXUTILITY::Descriptor m_descriptor;
		UINT m_bufferSize = 0;
	};

	// Bufferの生成
	ComPtr<ID3D12Resource> CreateBuffer(
		std::weak_ptr<Device> device,
		size_t requestSize,
		const void* initData,
		D3D12_HEAP_TYPE heapType,
		D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE,
		const wchar_t* name = nullptr
	);

	// UAV Bufferの生成
	ComPtr<ID3D12Resource> CreateBufferUAV(
		std::weak_ptr<Device> device,
		size_t requestSize,
		D3D12_RESOURCE_STATES initialState,
		const wchar_t* name);

	// Constant Bufferの生成
	ComPtr<ID3D12Resource> CreateConstantBuffer(
		std::weak_ptr<Device> device,
		size_t totalSize
	);

	// UAVの生成(フォーマット指定)
	Descriptor CreateStructuredUAV(
		std::weak_ptr<Device> device,
		ComPtr<ID3D12Resource> resource,
		UINT numElements,
		UINT firstElement,
		DXGI_FORMAT format);

	// StructuredBuffer用のSRV生成(ストライド指定)
	Descriptor CreateStructuredUAV(
		std::weak_ptr<Device> device,
		ComPtr<ID3D12Resource> resource,
		UINT numElements,
		UINT firstElement,
		UINT stride);

	// StructuredBuffer用のSRV生成(フォーマット指定)
	Descriptor CreateStructuredSRV(
		std::weak_ptr<Device> device,
		ComPtr<ID3D12Resource> resource,
		UINT numElements,
		UINT firstElement,
		DXGI_FORMAT format);

	// StructuredBuffer用のSRV生成(ストライド指定)
	Descriptor CreateStructuredSRV(
		std::weak_ptr<Device> device,
		ComPtr<ID3D12Resource> resource,
		UINT numElements,
		UINT firstElement,
		UINT stride);

	//--- メモリ書き込みUtil
	// shader id
	UINT WriteShaderIdentifier(void* dst, const void* shaderId);
	
	// Descriptor for GPU
	UINT WriteGPUDescriptor(void* dst, const Descriptor& descriptor);

	// Constant Buffer
	UINT WriteGpuResourceAddr(void* dst, const ComPtr<ID3D12Resource>& addr);
	UINT WriteGpuResourceAddr(void* dst, const D3D12_GPU_VIRTUAL_ADDRESS& addr);
#pragma endregion

#pragma region Text
	// UTF-8に変換
	std::string ToUTF8(const std::wstring& value);
	std::wstring ToWstring(const std::string& value);
#pragma endregion
}
