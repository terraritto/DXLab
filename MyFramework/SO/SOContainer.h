#pragma once
#include <vector>
#include <string>
#include <map>
#include <d3d12.h>
#include "../Utility/TypeUtility.h"
#include "../../include/d3dx12.h"

struct ShaderFileInfo {
	// shader data
	std::vector<char> binary;
	D3D12_SHADER_BYTECODE code;

	// shader���̊֐�
	std::vector<LPCWSTR> functionName;
	
	// hitgroup
	std::wstring hitgroupName; // �o�^����Hitgroup��
	bool IsHitgroupShader = false; // hitgroup�p��shader���H
};

class SOContainer
{
public:
	// �����g����
	void SetState(D3D12_STATE_OBJECT_TYPE ObjType) { m_subObject.SetStateObjectType(ObjType); }

	// shader�p
	void LoadShader(std::vector<std::wstring> fileNameArray, std::vector<std::vector<LPCWSTR>> funcName, std::vector<std::wstring> hitgroupName);
	void SetupRootSignature(const std::wstring HitgroupName, ID3D12RootSignature* rsResource);

	// �쐬�p
	void AddDxil();
	void AddHitGroup();
	void AddShaderConfig(UINT maxPayloadSize, UINT maxAttributeSize);
	void AddRS(ComPtr<ID3D12RootSignature> rsResource);
	void AddLRS();
	void AddLRS(ID3D12RootSignature* rsResource, LPCWSTR ExportName);
	void AddPipeline(UINT maxRecursion);

	// stateobject��Get
	CD3DX12_STATE_OBJECT_DESC& GetSubObject() { return m_subObject; }

protected:
	std::map<std::wstring, ShaderFileInfo> m_shaderInfo;

	std::map<std::wstring, ID3D12RootSignature*> m_rootSignatureMap;
	
	CD3DX12_STATE_OBJECT_DESC m_subObject;
};
