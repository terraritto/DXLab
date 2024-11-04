#include "SimpleModelApp.h"
#include "../../MyFramework/PSO/Post/FXAAPSO.h"
#include "../../../include/imgui/imgui.h"
#include "../../../include/imgui/imgui_impl_dx12.h"
#include "../../../include/imgui/imgui_impl_win32.h"
#include "../../../MyFramework/Geometry/GeometryDefinitions.h"
#include "../../../MyFramework/Material/DefaultMeshMaterial.h"
#include "../../../MyFramework/Result/MSAABuffer.h"
#include "../../../MyFramework/SO/ShaderUtility.h"
#include "../../../MyFramework/Utility/Win32App.h"
#include "../../../include/d3dx12.h"
#include "../../../MyFramework/Pool/MeshPool.h"
#include "../../../MyFramework/Utility/ShaderParamUtil.h"

SimpleModelApp::SimpleModelApp(UINT width, UINT height)
	: RenderInterface(width, height, L"SimpleTriangle")
{
}

void SimpleModelApp::Initialize()
{
	// DX12の初期化
	if (InitializeGraphicsDevice(Win32Application::GetHWND()) == false)
	{
		throw std::runtime_error("Failed Initialize Device.");
	}

	// Meshの初期化
	SetupMesh();

	// Bufferの初期化
	SetupBuffer();

	// ShadowMapの初期化
	SetupShadowMap();

	// Post用のRenderTargetを用意
	m_result = std::make_unique<ResultBuffer>();

	// SSAA分岐
	if(m_device->m_global->isUseSSAA)
	{ 
		auto scale = m_device->m_global->ssaaScale;
		m_result->Initialize(m_device.get(), GetWidth() * scale, GetHeight() * scale, DXGI_FORMAT_R8G8B8A8_UNORM);
	}
	else
	{
		m_result->Initialize(m_device.get(), GetWidth(), GetHeight(), DXGI_FORMAT_R8G8B8A8_UNORM);
	}
	
	// msaa限定
	if (m_device->m_global->isUseMSAA)
	{
		m_msaa = std::make_unique<MSAABuffer>();
		m_msaa->Initialize(m_device.get(), GetWidth(), GetHeight(), DXGI_FORMAT_R8G8B8A8_UNORM);
	}

	// DescriptorRangeとRootParamを生成
	CD3DX12_DESCRIPTOR_RANGE postTex, postCbv;
	postTex.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
	postCbv.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);

	CD3DX12_ROOT_PARAMETER postrootParams[2];
	postrootParams[0].InitAsDescriptorTable(1, &postTex, D3D12_SHADER_VISIBILITY_PIXEL);
	postrootParams[1].InitAsDescriptorTable(1, &postCbv, D3D12_SHADER_VISIBILITY_ALL);

	CD3DX12_STATIC_SAMPLER_DESC descSampler(0,
		D3D12_FILTER_MIN_MAG_MIP_LINEAR,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);

	// RootSignatureの生成
	CD3DX12_ROOT_SIGNATURE_DESC postRootSigDesc{};
	postRootSigDesc.Init(
		_countof(postrootParams), postrootParams, // pParameters
		1, &descSampler, // pStaticSamplers
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	m_postRootSignature = CreateRootSignature(postRootSigDesc);

	// DescriptorRangeとRootParamを生成
	CD3DX12_DESCRIPTOR_RANGE cbv, shadow, tex1srv, tex2srv, tex3srv, tex4srv, sampler, mat, light;
	cbv.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	tex1srv.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
	tex2srv.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
	tex3srv.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2);
	tex4srv.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3);
	sampler.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0);
	shadow.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);
	mat.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 2);
	light.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 3);

	CD3DX12_ROOT_PARAMETER rootParams[9];
	rootParams[0].InitAsDescriptorTable(1, &cbv, D3D12_SHADER_VISIBILITY_ALL);
	rootParams[1].InitAsDescriptorTable(1, &tex1srv, D3D12_SHADER_VISIBILITY_PIXEL);
	rootParams[2].InitAsDescriptorTable(1, &tex2srv, D3D12_SHADER_VISIBILITY_PIXEL);
	rootParams[3].InitAsDescriptorTable(1, &tex3srv, D3D12_SHADER_VISIBILITY_PIXEL);
	rootParams[4].InitAsDescriptorTable(1, &tex4srv, D3D12_SHADER_VISIBILITY_PIXEL);
	rootParams[5].InitAsDescriptorTable(1, &sampler, D3D12_SHADER_VISIBILITY_PIXEL);
	rootParams[6].InitAsDescriptorTable(1, &shadow, D3D12_SHADER_VISIBILITY_PIXEL);
	rootParams[7].InitAsDescriptorTable(1, &mat, D3D12_SHADER_VISIBILITY_PIXEL);
	rootParams[8].InitAsDescriptorTable(1, &light, D3D12_SHADER_VISIBILITY_PIXEL);

	// RootSignatureの生成
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc{};
	rootSigDesc.Init(
		_countof(rootParams), rootParams, // pParameters
		0, nullptr, // pStaticSamplers
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	m_rootSignature = CreateRootSignature(rootSigDesc);

	// InputLayout
	std::vector<D3D12_INPUT_ELEMENT_DESC> inputElementDesc =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexMesh, Position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(VertexMesh, Normal), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(VertexMesh, TexCoord), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA},
		{"TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(VertexMesh, Tangent), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA},
	};

	if (m_device->m_global->isUseMSAA)
	{
		m_psoContainer.SetSampleDesc({ m_device->m_global->msaaSample, 0 });
	}

	// PSOの生成
	m_psoContainer.SetVSByte(L"SimpleModelVS.dxlib");
	m_psoContainer.SetPSByte(L"SimpleModelPS.dxlib");
	m_psoContainer.SetCullMode(D3D12_CULL_MODE_FRONT);
	m_psoContainer.IsDepthClip(true);
	m_psoContainer.SetRootSignature(m_rootSignature);
	m_psoContainer.SetInputElement(inputElementDesc);
	m_psoContainer.CreatePSO(m_device->GetDevice());

	BlendData data
	{
		true,
		D3D12_BLEND_SRC_ALPHA, D3D12_BLEND_INV_SRC_ALPHA,
		D3D12_BLEND_SRC_ALPHA, D3D12_BLEND_INV_SRC_ALPHA
	};

	m_AlphaPsoContainer.SetVSByte(L"SimpleModelVS.dxlib");
	m_AlphaPsoContainer.SetPSByte(L"SimpleModelAlphaPS.dxlib");
	m_AlphaPsoContainer.SetRootSignature(m_rootSignature);
	m_AlphaPsoContainer.SetCullMode(D3D12_CULL_MODE_FRONT);
	m_AlphaPsoContainer.IsDepthClip(true);
	m_AlphaPsoContainer.SetInputElement(inputElementDesc);
	m_AlphaPsoContainer.SetBlendData(data);
	m_AlphaPsoContainer.CreatePSO(m_device->GetDevice());

	// Rim用
	m_rimContainer.SetVSByte(L"SimpleModelVS.dxlib");
	m_rimContainer.SetPSByte(L"RimPS.dxlib");
	m_rimContainer.SetCullMode(D3D12_CULL_MODE_FRONT);
	m_rimContainer.IsDepthClip(true);
	m_rimContainer.SetRootSignature(m_rootSignature);
	m_rimContainer.SetInputElement(inputElementDesc);
	m_rimContainer.CreatePSO(m_device->GetDevice());

	// Post関係のPSOを作成
	std::unique_ptr<PostPSO> postPSO;
	{
		// Test
		postPSO = std::make_unique<FXAAPSO>(m_device);

		postPSO->SetVSByte(L"SimpleVS.dxlib");
		postPSO->SetPSByte(L"FXAAPS.dxlib");
		postPSO->SetRootSignature(m_postRootSignature);
		postPSO->CreatePSO(m_device->GetDevice());
		m_postPSOList.emplace_back(std::move(postPSO));
	}

	// CommandList
	m_commandList = m_device->CreateCommandList();
	m_commandList->Close();

	// ImGui
	auto heap = m_device->GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	auto descriptorForImGui = m_device->AllocateDescriptor();
	ImGui_ImplDX12_Init
	(
		m_device->GetDevice().Get(),
		m_device->BACK_BUFFER_COUNT,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		heap.Get(),
		descriptorForImGui.m_cpuHandle,
		descriptorForImGui.m_gpuHandle
	);

	m_device->waitForIdleGpu();
}

void SimpleModelApp::Destroy()
{
	m_postPSOList.clear();
	m_sampler->UnInitialize(m_device.get());
	m_sceneCB.UnInitialize(m_device);
	m_sceneShadowCB.UnInitialize(m_device);
	m_shadowCB.UnInitialize(m_device);
	m_hemisphereSceneCB.UnInitialize(m_device);
	
	ImGui_ImplDX12_Shutdown();
}

void SimpleModelApp::Update(double DeltaTime)
{
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	auto framerate = ImGui::GetIO().Framerate;
	ImGui::Begin("Information");
	ImGui::Text("GPU: %s", m_device->GetAdapterName().c_str());
	ImGui::Text("Framerate: %.3f ms", 1000.0f / framerate);

	XMFLOAT3 camPos;
	XMStoreFloat3(&camPos, m_camera.GetPosition());
	ImGui::Text("CameraPos (%.2f, %.2f, %.2f)", camPos.x, camPos.y, camPos.z);

	ImGui::End();


	auto& lightList = LightPool::GetInstance().GetLights();
	ImGui::Begin("Light");

	for (auto& light : lightList)
	{
		std::string treeName = ConvertLightTypeString(*light.get()) + "_" + light->m_name;
		if (ImGui::TreeNode(treeName.c_str()))
		{
			ImGui::SliderFloat3("color", light->m_color.m128_f32, 0.0f, 1.0f);
			
			switch (light->m_type)
			{
			case 0:
				ImGui::SliderFloat3("position", light->m_position.m128_f32, -1000.0f, 1000.0f);
				ImGui::SliderFloat("intensity", &light->m_intensity, 0.001f, 1000.0f);
				break;

			case 1:
				ImGui::SliderFloat3("direction", light->m_direction.m128_f32, -1.0f, 1.0f);
				ImGui::SliderFloat("intensity", &light->m_intensity, 0.001f, 2.0f);
				break;

			case 2:
				ImGui::SliderFloat3("direction", light->m_direction.m128_f32, -1.0f, 1.0f);
				ImGui::SliderFloat("radius", &light->m_radius, 0.001f, 10.0f);
				ImGui::SliderFloat("inner Angle", &light->m_innerAngle, 0.001f, 10.0f);
				ImGui::SliderFloat("outer Angle", &light->m_outerAngle, 0.001f, 10.0f);
				ImGui::SliderFloat3("position", light->m_position.m128_f32, -1000.0f, 1000.0f);
				ImGui::SliderFloat("intensity", &light->m_intensity, 0.001f, 1000.0f);
				break;

			default:
				break;
			}

			ImGui::TreePop();
		}
	}

	ImGui::End();


	ImGui::Begin("Material");
	auto index = MaterialManager::GetInstance().GetMaterialTail();
	for (int i = 0; i < index; i++)
	{
		auto* material = static_cast<DefaultMeshMaterial*>(MaterialManager::GetInstance().GetMaterial(i).get());
		if (material == nullptr)
		{
			continue;
		}

		std::string treeName = "material_" + std::to_string(i+1);
		if (ImGui::TreeNode(treeName.c_str()))
		{
			XMVECTOR vec3Data = material->GetAlbedo();
			ImGui::SliderFloat3("albedo", vec3Data.m128_f32, -30.0f, 30.0f);
			material->SetAlbedo(vec3Data);

			vec3Data = material->GetSpecular();
			ImGui::SliderFloat3("specular", vec3Data.m128_f32, -30.0f, 30.0f);
			material->SetSpecular(vec3Data);

			float floatData = material->GetSpecularPow();
			ImGui::SliderFloat("shininess", &floatData, 0.0f, 1000.0f);
			material->SetSpecularPow(floatData);

			std::string path = DXUTILITY::ToUTF8(material->GetColorTextureName());
			ImGui::Text("ColorTexture: %s", path.c_str());
			ImGui::Image((ImTextureID)(material->GetColorTexture().lock().get()->srv.m_gpuHandle.ptr), ImVec2(300, 300));

			path = DXUTILITY::ToUTF8(material->GetDiffuseTextureName());
			ImGui::Text("Diffuse Texture: %s", path.c_str());
			ImGui::Image((ImTextureID)(material->GetDiffuseTexture().lock().get()->srv.m_gpuHandle.ptr), ImVec2(300, 300));

			path = DXUTILITY::ToUTF8(material->GetNormalTextureName());
			ImGui::Text("Diffuse Texture: %s", path.c_str());
			ImGui::Image((ImTextureID)(material->GetNormalTexture().lock().get()->srv.m_gpuHandle.ptr), ImVec2(300, 300));

			ImGui::TreePop();
		}
	}
	ImGui::End();

	for (auto& post : m_postPSOList)
	{
		post->Update(DeltaTime);
	}

	// Shadow
	ImGui::Begin("Shadow");
	{
		ImGui::SliderFloat3("pos", m_shadowCamera.GetPosition().m128_f32, 0.0f, 100.0f);
		ImGui::SliderFloat("distance", &m_device->m_global->lightDistance, 1000.0f, 10000.0);

		ImGui::Text("ShadowMap:");
		ImGui::Image((ImTextureID)(m_shadowMap->GetOutputDescriptor()->m_gpuHandle.ptr), ImVec2(300, 300));

		// 更新
		auto pos = m_shadowCamera.GetPosition();
		auto scalar = m_device->m_global->lightDistance;
		XMFLOAT3 eyePos(pos.m128_f32[0], pos.m128_f32[1], pos.m128_f32[2]);
		XMFLOAT3 dir = GetDirection();
		XMFLOAT3 target(
			eyePos.x - dir.x * scalar,
			eyePos.y - dir.y * scalar,
			eyePos.z - dir.z * scalar);

		m_shadowCamera.SetLookAt(eyePos, target);
		m_shadowCamera.SetPerspective(
			DirectX::XM_PIDIV4, GetAspect(), 0.1f, scalar
		);
	}
	ImGui::End();

	ImGui::EndFrame();

	auto frameIndex = m_device->GetCurrentFrameIndex();
	MaterialManager::GetInstance().UpdateMaterialData(frameIndex);
}

void SimpleModelApp::OnRender()
{
	auto device = m_device->GetDevice();
	auto frameIndex = m_device->GetCurrentFrameIndex();

	// Allocatorのリセット
	auto allocator = m_device->GetCurrentCommandAllocator();
	allocator->Reset();

	// commandlistのリセット
	m_commandList->Reset(allocator.Get(), nullptr);

	auto renderTarget = m_device->GetRT();

	// swapchain表示可能からRTV描画可能へ
	auto barrerToRT = CD3DX12_RESOURCE_BARRIER::Transition
	(
		renderTarget.Get(),
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);
	m_commandList->ResourceBarrier(1, &barrerToRT);
	
	// ShadowMap
	RenderToShadow();

	// Mainの描画
	if (m_device->m_global->isUseMSAA)
	{
		RenderToMSAA();
	}
	else
	{
		RenderToTexture();
	}

	// Post用描画
	RenderToMain();

	// ImGuiの描画
	ImGui::Render();
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_commandList.Get());

	// RTV描画可能->swapchain表示可能
	CD3DX12_RESOURCE_BARRIER barriers[] =
	{ CD3DX12_RESOURCE_BARRIER::Transition
	(
		renderTarget.Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT
	) , 
		// post用にRenderに戻す
		CD3DX12_RESOURCE_BARRIER::Transition(
		m_result->GetOutputBuffer().Get(),
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET
	) ,
		// ShadowをRenderTargetに戻しておく
		CD3DX12_RESOURCE_BARRIER::Transition(
		m_shadowMap->GetOutputBuffer().Get(),
		D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET
	) ,	
		// ShadowのDepthも戻す
		CD3DX12_RESOURCE_BARRIER::Transition(
		m_shadowMap->GetDepthBuffer().Get(),
		D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE
	),
	};

	m_commandList->ResourceBarrier(_countof(barriers), barriers);

	m_commandList->Close();

	m_device->ExecuteCommandList(m_commandList);

	m_device->Present(1);
}

void SimpleModelApp::SetupMesh()
{
	// meshを用意
	m_meshLoader = std::make_unique<DefaultMeshLoader>();
	m_fileName = L"Mesh/SimpleSponza/sponza.obj";
	m_modelName = L"Mesh/mitsuba/mitsuba-sphere.obj";
	;
	if (m_meshLoader->LoadMesh(m_device, m_fileName) == false)
	{
		throw std::runtime_error("Failed Initialize Mesh.");
	}

	if (m_meshLoader->LoadMesh(m_device, m_modelName) == false)
	{
		throw std::runtime_error("Failed Initialize Mesh.");
	}
}

void SimpleModelApp::SetupBuffer()
{
	// 定数バッファ関係の用意
	m_sceneCB.Initialize(m_device, sizeof(ShaderParameters));
	m_sceneCB.SetupBufferView(m_device);

	m_sceneShadowCB.Initialize(m_device, sizeof(SceneShadowParameters));
	m_sceneShadowCB.SetupBufferView(m_device);

	m_shadowCB.Initialize(m_device, sizeof(ShadowParameters));
	m_shadowCB.SetupBufferView(m_device);

	m_hemisphereSceneCB.Initialize(m_device, sizeof(ShaderHemisphereParameters));
	m_hemisphereSceneCB.SetupBufferView(m_device);

	// Samplerの用意
	m_sampler = std::make_unique<DXUTILITY::SamplerDescriptor>();
	m_sampler->Initialize(m_device.get());

	// Materialの更新
	MaterialManager::GetInstance().WriteDescriptorData();

	// カメラの用意
	XMFLOAT3 eyePos(6.0f, 4.0f, 20.0f);
	XMFLOAT3 target(0.0f, 0.0f, 0.0f);
	m_camera.SetLookAt(eyePos, target);
	m_camera.SetPerspective(
		DirectX::XM_PIDIV4, GetAspect(), 0.1f, 10000.0f
	);
	m_input->AddInputProcessor(&m_camera);

	// 固定のDirectional Lightを用意
	{
		std::unique_ptr<Light> directionalLight = std::make_unique<Light>();
		directionalLight->m_color = DirectX::g_XMOne;
		directionalLight->m_intensity = 1.0f;
		directionalLight->m_type = 1;
		directionalLight->m_direction = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		LightPool::GetInstance().RegisterLight(std::move(directionalLight));
	}

	// ライトを初期化
	LightPool::GetInstance().Initialize(m_device);
}

void SimpleModelApp::SetupShadowMap()
{
	auto format = DXGI_FORMAT_R32G32B32A32_FLOAT;

	// ShadowMap用のRenderTargetを用意
	m_shadowMap = std::make_unique<ResultBuffer>();

	DirectX::XMVECTORF32 clearColor = { 1.0f,1.0f,1.0f,1.0f };
	m_shadowMap->Initialize(m_device.get(), GetWidth(), GetHeight(), format, clearColor);

	// DescriptorRangeとRootParamを生成
	CD3DX12_DESCRIPTOR_RANGE sceneCB;
	sceneCB.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);

	CD3DX12_ROOT_PARAMETER rootParams[1];
	rootParams[0].InitAsDescriptorTable(1, &sceneCB, D3D12_SHADER_VISIBILITY_ALL);

	// RootSignatureの生成
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc{};
	rootSigDesc.Init(
		_countof(rootParams), rootParams, // pParameters
		0, nullptr, // pStaticSamplers
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	m_shadowMapRS = CreateRootSignature(rootSigDesc);

	// InputLayout
	std::vector<D3D12_INPUT_ELEMENT_DESC> inputElementDesc =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexMesh, Position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(VertexMesh, Normal), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(VertexMesh, TexCoord), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA},
		{"TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(VertexMesh, Tangent), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA},
	};

	// PSOの生成
	m_shadowMapPSO.SetVSByte(L"SimpleShadowVS.dxlib");
	m_shadowMapPSO.SetPSByte(L"SimpleShadowPS.dxlib");
	m_shadowMapPSO.SetRTVFormat(0, format);
	m_shadowMapPSO.SetCullMode(D3D12_CULL_MODE_NONE);
	m_shadowMapPSO.IsDepthClip(true);
	m_shadowMapPSO.SetRootSignature(m_shadowMapRS);
	m_shadowMapPSO.SetInputElement(inputElementDesc);
	m_shadowMapPSO.CreatePSO(m_device->GetDevice());

	// Shadowmap用のカメラの準備
	auto scalar = m_device->m_global->lightDistance;
	XMFLOAT3 eyePos(6.0f, 450.0f, 20.0f);
	XMFLOAT3 dir = GetDirection();
	XMFLOAT3 target(
		eyePos.x - dir.x * scalar, 
		eyePos.y - dir.y * scalar,
		eyePos.z - dir.z * scalar);
	m_shadowCamera.SetLookAt(eyePos, target);
	m_shadowCamera.SetPerspective(
		DirectX::XM_PIDIV4, GetAspect(), 0.1f, 10000.0f
	);
}

XMFLOAT3 SimpleModelApp::GetDirection()
{
	XMFLOAT3 lightDir(1.0, 0.0, 0.0);
	auto& lightList = LightPool::GetInstance().GetLights();
	for (auto& light : lightList)
	{
		if (light->m_type == 1)
		{
			lightDir.x = light->m_direction.m128_f32[0];
			lightDir.y = light->m_direction.m128_f32[1];
			lightDir.z = light->m_direction.m128_f32[2];
			break;
		}
	}
	return lightDir;
}

void SimpleModelApp::RenderToShadow()
{
	auto frameIndex = m_device->GetCurrentFrameIndex();

	// カラーバッファのクリア
	const float clearColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	m_commandList->ClearRenderTargetView(
		m_shadowMap->GetOutputRTVDescriptor()->m_cpuHandle, clearColor, 0, nullptr);

	// デプスバッファのクリア
	m_commandList->ClearDepthStencilView(
		m_shadowMap->GetOutputDSVDescriptor()->m_cpuHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	// 描画先をセット
	m_commandList->OMSetRenderTargets(
		1, &m_shadowMap->GetOutputRTVDescriptor()->m_cpuHandle, FALSE, &m_shadowMap->GetOutputDSVDescriptor()->m_cpuHandle);

	// 各行列をセットしていく
	ShadowParameters shaderParams;
	auto Rotate = DirectX::XMMatrixRotationAxis(DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), DirectX::XMConvertToRadians(0.0f));
	DirectX::XMStoreFloat4x4(&shaderParams.mtxWorld, Rotate);
	DirectX::XMStoreFloat4x4(&shaderParams.mtxView, m_shadowCamera.GetViewMatrix());
	DirectX::XMStoreFloat4x4(&shaderParams.mtxProj, m_shadowCamera.GetProjectionMatrix());
	
	// 定数バッファの更新
	{
		void* p = m_shadowCB.Map(frameIndex);
		memcpy(p, &shaderParams, sizeof(ShadowParameters));
		m_shadowCB.Unmap(frameIndex);
	}

	// RootSignatureのセット
	m_commandList->SetGraphicsRootSignature(m_shadowMapRS.Get());

	// viewport/scissorのセット
	m_commandList->RSSetViewports(1, &m_device->GetDefaultViewport());
	m_commandList->RSSetScissorRects(1, &m_device->GetDefaultScissorRect());

	// DescriptorHeapのセット
	ID3D12DescriptorHeap* heaps[] =
	{
		m_device->GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV).Get(),
		m_device->GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER).Get()
	};
	m_commandList->SetDescriptorHeaps(_countof(heaps), heaps);

	// 一気にmeshを描いていく
	{
		auto meshList = MeshPool::GetInstance().GetMesh(m_modelName);
		if (meshList != nullptr)
		{
			// PSOのセット
			m_commandList->SetPipelineState(m_shadowMapPSO.GetPSO().Get());

			for (auto mesh : *meshList)
			{
				auto& material = MaterialManager::GetInstance().GetMaterial(mesh->m_materialId);
				DefaultMeshMaterial* meshMat = static_cast<DefaultMeshMaterial*>(material.get());

				// Primitiveのセット
				m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
				m_commandList->IASetVertexBuffers(0, 1, &mesh->m_vertexView);
				m_commandList->IASetIndexBuffer(&mesh->m_indexView);

				// RootDescriptorのセット
				m_commandList->SetGraphicsRootDescriptorTable(0, m_shadowCB.GetDescriptor(frameIndex).m_gpuHandle);

				// コマンドの発行
				m_commandList->DrawIndexedInstanced(mesh->m_indexCount, 1, 0, 0, 0);
			}
		}
	}

	// 描画後、テクスチャとして使うためのバリアを設定.
	D3D12_RESOURCE_BARRIER barriers[]
		= 
	{
		 CD3DX12_RESOURCE_BARRIER::Transition(
		m_shadowMap->GetOutputBuffer().Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE),
		CD3DX12_RESOURCE_BARRIER::Transition(
		m_shadowMap->GetDepthBuffer().Get(),
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		D3D12_RESOURCE_STATE_GENERIC_READ),
	};

	m_commandList->ResourceBarrier(_countof(barriers), barriers);

}

void SimpleModelApp::RenderToTexture()
{
	auto frameIndex = m_device->GetCurrentFrameIndex();

	// カラーバッファのクリア
	const float clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	m_commandList->ClearRenderTargetView(
		m_result->GetOutputRTVDescriptor()->m_cpuHandle, clearColor, 0, nullptr);

	// デプスバッファのクリア
	m_commandList->ClearDepthStencilView(
		m_result->GetOutputDSVDescriptor()->m_cpuHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	// 描画先をセット
	m_commandList->OMSetRenderTargets(1, &m_result->GetOutputRTVDescriptor()->m_cpuHandle, 
		FALSE, &m_result->GetOutputDSVDescriptor()->m_cpuHandle);

	// 各行列をセットしていく
	ShaderParameters shaderParams;
	auto Rotate = DirectX::XMMatrixRotationAxis(DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), DirectX::XMConvertToRadians(0.0f));
	DirectX::XMStoreFloat4x4(&shaderParams.mtxWorld, Rotate);
	DirectX::XMStoreFloat4x4(&shaderParams.mtxView, m_camera.GetViewMatrix());
	DirectX::XMStoreFloat4x4(&shaderParams.mtxProj, m_camera.GetProjectionMatrix());
	shaderParams.cameraParam = m_camera.GetPosition();
	shaderParams.lightIndex = LightPool::GetInstance().GetLights().size();

	// 定数バッファの更新
	{
		void* p = m_sceneCB.Map(frameIndex);
		memcpy(p, &shaderParams, sizeof(ShaderParameters));
		m_sceneCB.Unmap(frameIndex);

		LightPool::GetInstance().UpdateData(frameIndex);
	}

	{
		SceneShadowParameters sceneShadowParams;

		auto lightVP = m_shadowCamera.GetViewMatrix() * m_shadowCamera.GetProjectionMatrix();
		DirectX::XMStoreFloat4x4(&sceneShadowParams.LVP, lightVP);
		sceneShadowParams.offset = 1.0f/1024.0f; // @todo: 設定パラメータは持たせる

		void* p = m_sceneShadowCB.Map(frameIndex);
		memcpy(p, &sceneShadowParams, sizeof(SceneShadowParameters));
		m_sceneShadowCB.Unmap(frameIndex);
	}

	{
		ShaderHemisphereParameters hemisphereParams;
		DirectX::XMStoreFloat4x4(&hemisphereParams.mtxWorld, Rotate);
		DirectX::XMStoreFloat4x4(&hemisphereParams.mtxView, m_camera.GetViewMatrix());
		DirectX::XMStoreFloat4x4(&hemisphereParams.mtxProj, m_camera.GetProjectionMatrix());
		hemisphereParams.cameraParam = m_camera.GetPosition();
		hemisphereParams.lightIndex = LightPool::GetInstance().GetLights().size();
		hemisphereParams.skyColor = DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 1.0f);
		hemisphereParams.groundColor = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);
		hemisphereParams.groundNormal = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);

		void* p = m_hemisphereSceneCB.Map(frameIndex);
		memcpy(p, &hemisphereParams, sizeof(ShaderHemisphereParameters));
		m_hemisphereSceneCB.Unmap(frameIndex);
	}

	// RootSignatureのセット
	m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());

	// viewport/scissorのセット
	if(m_device->m_global->isUseSSAA)
	{
		auto viewport = m_device->GetDefaultViewport();
		viewport.Width = GetWidth() * m_device->m_global->ssaaScale;
		viewport.Height = GetHeight() * m_device->m_global->ssaaScale;

		auto scissor = m_device->GetDefaultScissorRect();
		scissor.right = GetWidth() * m_device->m_global->ssaaScale;
		scissor.bottom = GetHeight() * m_device->m_global->ssaaScale;

		m_commandList->RSSetViewports(1, &viewport);
		m_commandList->RSSetScissorRects(1, &scissor);
	}
	else
	{
		m_commandList->RSSetViewports(1, &m_device->GetDefaultViewport());
		m_commandList->RSSetScissorRects(1, &m_device->GetDefaultScissorRect());
	}

	// DescriptorHeapのセット
	ID3D12DescriptorHeap* heaps[] =
	{
		m_device->GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV).Get(),
		m_device->GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER).Get()
	};
	m_commandList->SetDescriptorHeaps(_countof(heaps), heaps);

	// 一気にmeshを描いていく
	{
		auto meshList = MeshPool::GetInstance().GetMesh(m_fileName);
		auto modelList = MeshPool::GetInstance().GetMesh(m_modelName);

		if (meshList != nullptr)
		{
			for (auto mesh : *meshList)
			{
				auto& material = MaterialManager::GetInstance().GetMaterial(mesh->m_materialId);
				DefaultMeshMaterial* meshMat = static_cast<DefaultMeshMaterial*>(material.get());

				// PSOのセット
				m_commandList->SetPipelineState(meshMat->GetIsAlpha() ? m_AlphaPsoContainer.GetPSO().Get() : m_psoContainer.GetPSO().Get());

				// Primitiveのセット
				m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
				m_commandList->IASetVertexBuffers(0, 1, &mesh->m_vertexView);
				m_commandList->IASetIndexBuffer(&mesh->m_indexView);

				// RootDescriptorのセット
				m_commandList->SetGraphicsRootDescriptorTable(0, m_sceneCB.GetDescriptor(frameIndex).m_gpuHandle);
				m_commandList->SetGraphicsRootDescriptorTable(1, meshMat->GetColorTexture().lock().get()->srv.m_gpuHandle);
				m_commandList->SetGraphicsRootDescriptorTable(2, meshMat->GetDiffuseTexture().lock().get()->srv.m_gpuHandle);
				m_commandList->SetGraphicsRootDescriptorTable(3, meshMat->GetNormalTexture().lock().get()->srv.m_gpuHandle);
				m_commandList->SetGraphicsRootDescriptorTable(4, m_shadowMap->GetOutputDescriptor()->m_gpuHandle);
				m_commandList->SetGraphicsRootDescriptorTable(5, m_sampler->GetDescriptor().m_gpuHandle);
				m_commandList->SetGraphicsRootDescriptorTable(6, m_sceneShadowCB.GetDescriptor(frameIndex).m_gpuHandle);
				m_commandList->SetGraphicsRootDescriptorTable(7, material->GetDescriptor(frameIndex));
				m_commandList->SetGraphicsRootDescriptorTable(8, LightPool::GetInstance().GetDescriptor(frameIndex).m_gpuHandle);

				// コマンドの発行
				m_commandList->DrawIndexedInstanced(mesh->m_indexCount, 1, 0, 0, 0);
			}
		}

		// どっかでまとめたいね
		if (modelList != nullptr)
		{
			for (auto mesh : *modelList)
			{
				auto& material = MaterialManager::GetInstance().GetMaterial(mesh->m_materialId);
				DefaultMeshMaterial* meshMat = static_cast<DefaultMeshMaterial*>(material.get());

				// PSOのセット
				m_commandList->SetPipelineState(meshMat->GetIsAlpha() ? m_AlphaPsoContainer.GetPSO().Get() : m_rimContainer.GetPSO().Get());

				// Primitiveのセット
				m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
				m_commandList->IASetVertexBuffers(0, 1, &mesh->m_vertexView);
				m_commandList->IASetIndexBuffer(&mesh->m_indexView);

				// RootDescriptorのセット
				m_commandList->SetGraphicsRootDescriptorTable(0, m_hemisphereSceneCB.GetDescriptor(frameIndex).m_gpuHandle);
				m_commandList->SetGraphicsRootDescriptorTable(1, meshMat->GetColorTexture().lock().get()->srv.m_gpuHandle);
				m_commandList->SetGraphicsRootDescriptorTable(2, meshMat->GetDiffuseTexture().lock().get()->srv.m_gpuHandle);
				m_commandList->SetGraphicsRootDescriptorTable(3, meshMat->GetNormalTexture().lock().get()->srv.m_gpuHandle);
				m_commandList->SetGraphicsRootDescriptorTable(4, m_shadowMap->GetOutputDepthDescriptor()->m_gpuHandle);
				m_commandList->SetGraphicsRootDescriptorTable(5, m_sampler->GetDescriptor().m_gpuHandle);
				m_commandList->SetGraphicsRootDescriptorTable(6, m_sceneShadowCB.GetDescriptor(frameIndex).m_gpuHandle);
				m_commandList->SetGraphicsRootDescriptorTable(7, material->GetDescriptor(frameIndex));
				m_commandList->SetGraphicsRootDescriptorTable(8, LightPool::GetInstance().GetDescriptor(frameIndex).m_gpuHandle);

				// コマンドの発行
				m_commandList->DrawIndexedInstanced(mesh->m_indexCount, 1, 0, 0, 0);
			}
		}
	}

	// 描画後、テクスチャとして使うためのバリアを設定.
	auto barrierToSRV = CD3DX12_RESOURCE_BARRIER::Transition(
		m_result->GetOutputBuffer().Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, 
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
	);
	m_commandList->ResourceBarrier(1, &barrierToSRV);
}

void SimpleModelApp::RenderToMSAA()
{
	auto frameIndex = m_device->GetCurrentFrameIndex();

	// Resolve->RT
	auto barrerToRT = CD3DX12_RESOURCE_BARRIER::Transition
	(
		m_msaa->GetOutputBuffer().Get(),
		D3D12_RESOURCE_STATE_RESOLVE_SOURCE,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);
	m_commandList->ResourceBarrier(1, &barrerToRT);

	// カラーバッファのクリア
	const float clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	m_commandList->ClearRenderTargetView(
		m_msaa->GetOutputRTVDescriptor()->m_cpuHandle, clearColor, 0, nullptr);

	// デプスバッファのクリア
	m_commandList->ClearDepthStencilView(
		m_msaa->GetOutputDSVDescriptor()->m_cpuHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	// 描画先をセット
	m_commandList->OMSetRenderTargets(1, &m_msaa->GetOutputRTVDescriptor()->m_cpuHandle, 
		FALSE, &m_msaa->GetOutputDSVDescriptor()->m_cpuHandle);

	// 各行列をセットしていく
	ShaderParameters shaderParams;
	auto Rotate = DirectX::XMMatrixRotationAxis(DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), DirectX::XMConvertToRadians(0.0f));
	DirectX::XMStoreFloat4x4(&shaderParams.mtxWorld, Rotate);
	DirectX::XMStoreFloat4x4(&shaderParams.mtxView, m_camera.GetViewMatrix());
	DirectX::XMStoreFloat4x4(&shaderParams.mtxProj, m_camera.GetProjectionMatrix());
	shaderParams.cameraParam = m_camera.GetPosition();
	shaderParams.lightIndex = LightPool::GetInstance().GetLights().size();

	// 定数バッファの更新
	{
		void* p = m_sceneCB.Map(frameIndex);
		memcpy(p, &shaderParams, sizeof(ShaderParameters));
		m_sceneCB.Unmap(frameIndex);

		LightPool::GetInstance().UpdateData(frameIndex);
	}

	{
		SceneShadowParameters sceneShadowParams;
		//DirectX::XMStoreFloat4x4(&sceneShadowParams.LVP, DirectX::XMMatrixTranspose(m_camera.GetViewMatrix()));

		void* p = m_sceneShadowCB.Map(frameIndex);
		memcpy(p, &sceneShadowParams, sizeof(SceneShadowParameters));
		m_sceneShadowCB.Unmap(frameIndex);
	}

	// RootSignatureのセット
	m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());

	// viewport/scissorのセット
	m_commandList->RSSetViewports(1, &m_device->GetDefaultViewport());
	m_commandList->RSSetScissorRects(1, &m_device->GetDefaultScissorRect());

	// DescriptorHeapのセット
	ID3D12DescriptorHeap* heaps[] =
	{
		m_device->GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV).Get(),
		m_device->GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER).Get()
	};
	m_commandList->SetDescriptorHeaps(_countof(heaps), heaps);

	// 一気にmeshを描いていく
	{
		auto meshList = MeshPool::GetInstance().GetMesh(m_fileName);
		auto modelList = MeshPool::GetInstance().GetMesh(m_modelName);
		if (meshList != nullptr)
		{
			for (auto mesh : *meshList)
			{
				auto& material = MaterialManager::GetInstance().GetMaterial(mesh->m_materialId);
				DefaultMeshMaterial* meshMat = static_cast<DefaultMeshMaterial*>(material.get());

				// PSOのセット
				m_commandList->SetPipelineState(meshMat->GetIsAlpha() ? m_AlphaPsoContainer.GetPSO().Get() : m_psoContainer.GetPSO().Get());

				// Primitiveのセット
				m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
				m_commandList->IASetVertexBuffers(0, 1, &mesh->m_vertexView);
				m_commandList->IASetIndexBuffer(&mesh->m_indexView);

				// RootDescriptorのセット
				m_commandList->SetGraphicsRootDescriptorTable(0, m_sceneCB.GetDescriptor(frameIndex).m_gpuHandle);
				m_commandList->SetGraphicsRootDescriptorTable(1, meshMat->GetColorTexture().lock().get()->srv.m_gpuHandle);
				m_commandList->SetGraphicsRootDescriptorTable(2, meshMat->GetDiffuseTexture().lock().get()->srv.m_gpuHandle);
				m_commandList->SetGraphicsRootDescriptorTable(3, meshMat->GetNormalTexture().lock().get()->srv.m_gpuHandle);
				m_commandList->SetGraphicsRootDescriptorTable(4, m_shadowMap->GetOutputDepthDescriptor()->m_gpuHandle);
				m_commandList->SetGraphicsRootDescriptorTable(5, m_sampler->GetDescriptor().m_gpuHandle);
				m_commandList->SetGraphicsRootDescriptorTable(6, m_sceneShadowCB.GetDescriptor(frameIndex).m_gpuHandle);
				m_commandList->SetGraphicsRootDescriptorTable(7, material->GetDescriptor(frameIndex));
				m_commandList->SetGraphicsRootDescriptorTable(8, LightPool::GetInstance().GetDescriptor(frameIndex).m_gpuHandle);

				// コマンドの発行
				m_commandList->DrawIndexedInstanced(mesh->m_indexCount, 1, 0, 0, 0);
			}
		}

		if (modelList != nullptr)
		{
			for (auto mesh : *modelList)
			{
				auto& material = MaterialManager::GetInstance().GetMaterial(mesh->m_materialId);
				DefaultMeshMaterial* meshMat = static_cast<DefaultMeshMaterial*>(material.get());

				// PSOのセット
				m_commandList->SetPipelineState(meshMat->GetIsAlpha() ? m_AlphaPsoContainer.GetPSO().Get() : m_rimContainer.GetPSO().Get());

				// Primitiveのセット
				m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
				m_commandList->IASetVertexBuffers(0, 1, &mesh->m_vertexView);
				m_commandList->IASetIndexBuffer(&mesh->m_indexView);

				// RootDescriptorのセット
				m_commandList->SetGraphicsRootDescriptorTable(0, m_hemisphereSceneCB.GetDescriptor(frameIndex).m_gpuHandle);
				m_commandList->SetGraphicsRootDescriptorTable(1, meshMat->GetColorTexture().lock().get()->srv.m_gpuHandle);
				m_commandList->SetGraphicsRootDescriptorTable(2, meshMat->GetDiffuseTexture().lock().get()->srv.m_gpuHandle);
				m_commandList->SetGraphicsRootDescriptorTable(3, meshMat->GetNormalTexture().lock().get()->srv.m_gpuHandle);
				m_commandList->SetGraphicsRootDescriptorTable(4, m_shadowMap->GetOutputDepthDescriptor()->m_gpuHandle);
				m_commandList->SetGraphicsRootDescriptorTable(5, m_sampler->GetDescriptor().m_gpuHandle);
				m_commandList->SetGraphicsRootDescriptorTable(6, m_sceneShadowCB.GetDescriptor(frameIndex).m_gpuHandle);
				m_commandList->SetGraphicsRootDescriptorTable(7, material->GetDescriptor(frameIndex));
				m_commandList->SetGraphicsRootDescriptorTable(8, LightPool::GetInstance().GetDescriptor(frameIndex).m_gpuHandle);

				// コマンドの発行
				m_commandList->DrawIndexedInstanced(mesh->m_indexCount, 1, 0, 0, 0);
			}
		}
	}

	// 描画後,Resolve Sourceとして設定
	auto ToResolveSrc = CD3DX12_RESOURCE_BARRIER::Transition(
		m_msaa->GetOutputBuffer().Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_RESOLVE_SOURCE
	);

	// MSAAバッファを Resolve のためにステート遷移させる.
	auto toResolveDst = CD3DX12_RESOURCE_BARRIER::Transition(
		m_result->GetOutputBuffer().Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_RESOLVE_DEST
	);

	D3D12_RESOURCE_BARRIER resolveBarriers[] 
		= { ToResolveSrc, toResolveDst };
	m_commandList->ResourceBarrier(_countof(resolveBarriers), resolveBarriers);
	m_commandList->ResolveSubresource(
		m_result->GetOutputBuffer().Get(), 0,
		m_msaa->GetOutputBuffer().Get(), 0,
		DXGI_FORMAT_R8G8B8A8_UNORM
	);

	auto barrierToSRV = 
	CD3DX12_RESOURCE_BARRIER::Transition(
		m_result->GetOutputBuffer().Get(),
		D3D12_RESOURCE_STATE_RESOLVE_DEST,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
	);
	m_commandList->ResourceBarrier(1, &barrierToSRV);
}

void SimpleModelApp::RenderToMain()
{
	// カラーバッファのクリア
	const float clearColor[] = { 0.1f, 0.25f, 0.5f, 0.0f };
	m_commandList->ClearRenderTargetView(
		m_device->GetRTV(), clearColor, 0, nullptr);

	// デプスバッファのクリア
	m_commandList->ClearDepthStencilView(
		m_device->GetDSV(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	// 描画先をセット
	m_commandList->OMSetRenderTargets(1, &m_device->GetRTV(), FALSE, &m_device->GetDSV());

	// RootSignatureのセット
	m_commandList->SetGraphicsRootSignature(m_postRootSignature.Get());

	// viewport/scissorのセット
	m_commandList->RSSetViewports(1, &m_device->GetDefaultViewport());
	m_commandList->RSSetScissorRects(1, &m_device->GetDefaultScissorRect());

	// DescriptorHeapのセット
	ID3D12DescriptorHeap* heaps[] =
	{
		m_device->GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV).Get(),
		m_device->GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER).Get()
	};
	m_commandList->SetDescriptorHeaps(_countof(heaps), heaps);

	// Post処理
	{
		for (auto& postPSO : m_postPSOList)
		{
			postPSO->Draw(m_commandList, m_result->GetOutputDescriptor()->m_gpuHandle);
		}
	}
}
