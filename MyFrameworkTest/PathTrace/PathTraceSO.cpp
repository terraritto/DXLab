#include "PathTraceSO.h"
#include <stdexcept>

void PathTraceSO::Initialize(DXUTILITY::Device* device, ComPtr<ID3D12RootSignature> grsResource, std::vector<ID3D12RootSignature*> lrsResourceList)
{
	m_container.SetState(D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE);

	// shader fileÇÃÉçÅ[Éh
	std::vector<std::wstring> shaderNameArray;
	std::vector<std::wstring> hitgroupNameArray;
	std::vector<std::vector<LPCWSTR>> shaderFuncNameArray;

	auto SetupShaderName = [&](std::wstring inShaderName, std::vector<LPCWSTR> inShaderFuncNameArray, std::wstring inHitgroupName)
		{
			shaderNameArray.push_back(inShaderName);
			shaderFuncNameArray.push_back(inShaderFuncNameArray);
			hitgroupNameArray.push_back(inHitgroupName);
		};

	SetupShaderName(L"raygen.dxlib", { L"mainRayGen" }, L"");
	SetupShaderName(L"miss.dxlib", { L"mainMiss", L"shadowMiss" }, L"");
	SetupShaderName(L"chsSphere.dxlib", { L"ClosestHitSphere", L"IntersectSphere" }, L"hgIntersectSphere");
	//SetupShaderName(L"chsPlane.dxlib", { L"ClosestHitPlane", L"IntersectPlane" }, L"hgIntersectPlane");
	//SetupShaderName(L"chsTriangle.dxlib", { L"ClosestHitTriangle", L"IntersectTriangle" }, L"hgIntersectTriangle");
	SetupShaderName(L"chsMesh.dxlib", { L"ClosestHitMesh" }, L"hgIntersectMesh");
	m_container.LoadShader(shaderNameArray, shaderFuncNameArray, hitgroupNameArray);

	// hitgroupÇÃópà”
	//m_container.SetupRootSignature(L"hgIntersectPlane", lrsResourceList[1]);
	m_container.SetupRootSignature(L"hgIntersectSphere", lrsResourceList[1]);
	//m_container.SetupRootSignature(L"hgIntersectTriangle", lrsResourceList[3]);
	m_container.SetupRootSignature(L"hgIntersectMesh", lrsResourceList[2]);

	m_container.AddDxil();
	m_container.AddHitGroup();

	// Global Rot Signature
	m_container.AddRS(grsResource);
	// Local RootSignature
	m_container.AddLRS(lrsResourceList[0], L"mainRayGen");
	m_container.AddLRS();

	m_container.AddShaderConfig(sizeof(XMFLOAT3)+ sizeof(XMFLOAT3) + sizeof(UINT), sizeof(XMFLOAT3));
	m_container.AddPipeline(16);

	HRESULT hr = device->GetDevice()->CreateStateObject
	(
		m_container.GetSubObject(),
		IID_PPV_ARGS(m_rtState.ReleaseAndGetAddressOf())
	);

	if (FAILED(hr))
	{
		throw std::runtime_error("State Object Creation is failed.");
	}
}
