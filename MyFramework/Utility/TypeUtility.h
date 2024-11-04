#pragma once
#include <wrl.h>
#include <DirectXMath.h>
#include <d3d12.h>

// Math
using XMVECTOR = DirectX::XMVECTOR;
using XMFLOAT2 = DirectX::XMFLOAT2;
using XMFLOAT3 = DirectX::XMFLOAT3;
using XMFLOAT4 = DirectX::XMFLOAT4;
using XMFLOAT3X4 = DirectX::XMFLOAT3X4;
using XMFLOAT4X4 = DirectX::XMFLOAT4X4;
using XMMATRIX = DirectX::XMMATRIX;

// ComPtr
template<class T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

// Rootsignature
enum class RootType {
    CBV = D3D12_ROOT_PARAMETER_TYPE_CBV,
    SRV = D3D12_ROOT_PARAMETER_TYPE_SRV,
    UAV = D3D12_ROOT_PARAMETER_TYPE_UAV,
};

enum class RangeType {
    SRV = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
    UAV = D3D12_DESCRIPTOR_RANGE_TYPE_UAV,
    CBV = D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
    Sampler = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER
};

enum class AddressMode {
    Wrap = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
    Mirror = D3D12_TEXTURE_ADDRESS_MODE_MIRROR,
    Clamp = D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
    Border = D3D12_TEXTURE_ADDRESS_MODE_BORDER,
    MirrorOnce = D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE
};

