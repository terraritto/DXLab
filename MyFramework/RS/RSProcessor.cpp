#include "RSProcessor.h"
#include "../../include/d3dx12.h"

void RSProcessor::Add(RootType type, UINT shaderRegister, UINT registerSpace)
{
    //RootParameterのAdd
    D3D12_ROOT_PARAMETER rootParam{};
    rootParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParam.ParameterType = static_cast<D3D12_ROOT_PARAMETER_TYPE>(type);
    rootParam.Descriptor.ShaderRegister = shaderRegister;
    rootParam.Descriptor.RegisterSpace = registerSpace;
    m_params.push_back(rootParam);
}

void RSProcessor::Add(RangeType type, UINT shaderRegister, UINT registerSpace, UINT descriptorCount)
{
    //Descriptor RangeのAdd
    D3D12_DESCRIPTOR_RANGE range{};
    range.RangeType = static_cast<D3D12_DESCRIPTOR_RANGE_TYPE>(type);
    range.NumDescriptors = descriptorCount;
    range.BaseShaderRegister = shaderRegister;
    range.RegisterSpace = registerSpace;
    range.OffsetInDescriptorsFromTableStart = 0;
    m_ranges.push_back(range);

    // あとで Range 解決するのでインデックスを保存
    // RangeとParamの対応付けが行われる
    UINT rangeIndex = UINT(m_params.size());
    m_rangeLocation.push_back(rangeIndex); //rangeはこのparam indexと対応付け

    //root paramの生成
    D3D12_ROOT_PARAMETER rootParam{};
    rootParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParam.DescriptorTable.NumDescriptorRanges = 1;
    rootParam.DescriptorTable.pDescriptorRanges = nullptr; // ここは生成時に解決する.
    m_params.push_back(rootParam);
}

void RSProcessor::AddStaticSampler(UINT shaderRegister, UINT registerSpace, D3D12_FILTER filter, AddressMode addressU, AddressMode addressV, AddressMode addressW)
{
    CD3DX12_STATIC_SAMPLER_DESC desc;
    desc.Init(shaderRegister,
        filter,
        static_cast<D3D12_TEXTURE_ADDRESS_MODE>(addressU),
        static_cast<D3D12_TEXTURE_ADDRESS_MODE>(addressV),
        static_cast<D3D12_TEXTURE_ADDRESS_MODE>(addressW)
    );
    desc.RegisterSpace = registerSpace;
    m_samplers.push_back(desc);
}

void RSProcessor::Clear() {
    m_params.clear();
    m_ranges.clear();
    m_rangeLocation.clear();
    m_samplers.clear();
}

ComPtr<ID3D12RootSignature> RSProcessor::Create(DXUTILITY::Device* device, bool isLocalRoot, const wchar_t* name)
{
    //rangeとRootParamsの対応付け
    for (UINT i = 0; i < UINT(m_ranges.size()); ++i) {
        auto index = m_rangeLocation[i];
        m_params[index].DescriptorTable.pDescriptorRanges = &m_ranges[i];
    }

    //Descの生成
    D3D12_ROOT_SIGNATURE_DESC desc{};
    //Root Parameterの設定
    if (!m_params.empty()) {
        desc.pParameters = m_params.data();
        desc.NumParameters = UINT(m_params.size());
    }

    if (!m_samplers.empty()) {
        desc.pStaticSamplers = m_samplers.data();
        desc.NumStaticSamplers = UINT(m_samplers.size());
    }

    if (isLocalRoot) {
        desc.Flags |= D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;
    }

    //以下 Root Signatureの生成
    ComPtr<ID3DBlob> blob, errBlob;
    HRESULT hr = D3D12SerializeRootSignature(
        &desc, D3D_ROOT_SIGNATURE_VERSION_1_0, &blob, &errBlob);
    if (FAILED(hr)) {
        return nullptr;
    }

    ComPtr<ID3D12RootSignature> rootSignature;
    hr = device->GetDevice()->CreateRootSignature(
        0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
    if (FAILED(hr)) {
        return nullptr;
    }

    return rootSignature;
}
