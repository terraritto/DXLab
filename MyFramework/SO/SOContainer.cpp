#include "SOContainer.h"
#include "ShaderUtility.h"

void SOContainer::LoadShader(std::vector<std::wstring> fileNameArray, std::vector<std::vector<LPCWSTR>> funcName, std::vector<std::wstring> hitgroupName)
{
    int count = 0;
    for (auto& filename : fileNameArray) {
        std::vector<char> data;
        if (!LoadFile(data, filename)) {
            throw std::runtime_error("shader file not found");
        }

        // データの格納
        m_shaderInfo.emplace(filename, ShaderFileInfo());

        m_shaderInfo[filename].binary = std::move(data);
        m_shaderInfo[filename].code = CD3DX12_SHADER_BYTECODE{ m_shaderInfo[filename].binary.data(), m_shaderInfo[filename].binary.size()};
        m_shaderInfo[filename].functionName = funcName[count];
        m_shaderInfo[filename].hitgroupName = hitgroupName[count];

        // 先頭がchsか判定
        // todo: libにこの処理自体をまとめとく
        const std::wstring prefix = L"chs";
        std::wstring::size_type position = filename.rfind(prefix);
        if (position != std::wstring::npos)
        {
            const auto target = filename.substr(position, 3);
            m_shaderInfo[filename].IsHitgroupShader = (target == prefix);
        }
        count++;
    }
}

void SOContainer::SetupRootSignature(const std::wstring HitgroupName, ID3D12RootSignature* rsResource)
{
    // MapにLocal RootSignatureを追加
    m_rootSignatureMap[HitgroupName] = rsResource;
}

void SOContainer::AddDxil()
{
    for(auto& shaderInfo : m_shaderInfo)
    {
        auto dxilSubObject = m_subObject.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
        dxilSubObject->SetDXILLibrary(&shaderInfo.second.code); 
        for (auto funcName : shaderInfo.second.functionName)
        {
            dxilSubObject->DefineExport(funcName);
        }
    }
}

void SOContainer::AddHitGroup()
{
    for (auto& shaderInfo : m_shaderInfo)
    {
        if (shaderInfo.second.IsHitgroupShader == false) { continue; }
        auto hitgroup = m_subObject.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
        hitgroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE_TRIANGLES);
        hitgroup->SetClosestHitShaderImport(shaderInfo.second.functionName[0]); // 0番目に絶対来るように
        if (shaderInfo.second.functionName.size() > 1)
        {
            hitgroup->SetIntersectionShaderImport(shaderInfo.second.functionName[1]); // 1番目に絶対来るように
            // IntersectionなのでHitGroupを変える
            hitgroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE_PROCEDURAL_PRIMITIVE);
        }
        hitgroup->SetHitGroupExport(shaderInfo.second.hitgroupName.c_str());
    }
}

void SOContainer::AddShaderConfig(UINT maxPayloadSize, UINT maxAttributeSize)
{
    auto shaderConfig = m_subObject.CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
    shaderConfig->Config(maxPayloadSize, maxAttributeSize);
}

void SOContainer::AddRS(ComPtr<ID3D12RootSignature> rsResource)
{
    auto rootsig = m_subObject.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();
    rootsig->SetRootSignature(rsResource.Get());
}

// Local SignatureをHitgroupと結び付ける
// todo: 0はRGに強制的にとかしてもいいかも
void SOContainer::AddLRS()
{
    for (auto& shaderInfo : m_shaderInfo)
    {
        if (shaderInfo.second.IsHitgroupShader == false) { continue; }
        if (m_rootSignatureMap[shaderInfo.second.hitgroupName] == nullptr) { continue; }
        
        auto lrs = m_subObject.CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
        lrs->SetRootSignature(m_rootSignatureMap[shaderInfo.second.hitgroupName]);

        auto associate = m_subObject.CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
        associate->AddExport(shaderInfo.second.hitgroupName.c_str());
        associate->SetSubobjectToAssociate(*lrs);
    }
}

// 単体用
void SOContainer::AddLRS(ID3D12RootSignature* rsResource, LPCWSTR ExportName)
{
    auto lrs = m_subObject.CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
    lrs->SetRootSignature(rsResource);

    auto lrsAssocModel = m_subObject.CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
    lrsAssocModel->AddExport(ExportName);
    lrsAssocModel->SetSubobjectToAssociate(*lrs);
}

void SOContainer::AddPipeline(UINT maxRecursion)
{
    auto pipelineConfig = m_subObject.CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();
    pipelineConfig->Config(maxRecursion);
}


