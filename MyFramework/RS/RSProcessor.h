#pragma once
#include <vector>
#include <memory>
#include <d3d12.h>
#include "../Utility/DXUtility.h"
#include "../Utility/TypeUtility.h"

class RSProcessor {
public:
    // Root‚Å’Ç‰Á
    void Add(RootType type, UINT shaderRegister, UINT registerSpace = 0);

    // Range‚Å’Ç‰Á
    void Add(RangeType type, UINT shaderRegister, UINT registerSpace = 0, UINT descriptorCount = 1);

    // shader‚Ì’Ç‰Á
    void AddStaticSampler(
        UINT shaderRegister,
        UINT registerSpace = 0,
        D3D12_FILTER filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR,
        AddressMode addressU = AddressMode::Wrap,
        AddressMode addressV = AddressMode::Wrap,
        AddressMode addressW = AddressMode::Wrap
    );

    void Clear();

    ComPtr<ID3D12RootSignature> Create(
        DXUTILITY::Device* device,
        bool isLocalRoot, const wchar_t* name);

private:
    std::vector<D3D12_ROOT_PARAMETER> m_params;
    std::vector<D3D12_DESCRIPTOR_RANGE> m_ranges;
    std::vector<UINT> m_rangeLocation;
    std::vector<D3D12_STATIC_SAMPLER_DESC> m_samplers;
};
