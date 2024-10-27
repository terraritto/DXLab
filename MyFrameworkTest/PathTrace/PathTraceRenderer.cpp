#include "PathTraceRenderer.h"
#include <random>
#include <cmath>
#include "../../include/imgui/imgui.h"
#include "../../include/imgui/imgui_impl_dx12.h"
#include "../../include/imgui/imgui_impl_win32.h"
#include "../../MyFramework/Utility/Win32App.h"
#include "../../MyFramework/AS/BLAS/DefaultMesh/DefaultMeshBLAS.h"
#include "../../MyFramework/AS/BLAS/ProcedualMesh/ProcedualMeshBLAS.h"
#include "../../MyFramework/AS/BLAS/Processor/Test3DGeometryProcessor.h"
#include "../../MyFramework/Material/Analytic/AnalyticSphere.h"
#include "../../MyFramework/Material/DefaultMeshMaterial.h"
#include "../../MyFramework/Pool/MeshPool.h"
#include "../../MyFramework/Pool/AreaLightPool.h"
#include "../../MyFramework/Sampler/Hammersley.h"
#include "../../MyFramework/Sampler/PureRandom.h"
#include "../../MyFramework/PSO/Post/ForDXR/AveragePSO.h"

static const wchar_t* PathTraceMesh = L"hgIntersectMesh";
static const wchar_t* PathTraceSphere = L"hgIntersectSphere";

PathTraceRenderer::PathTraceRenderer(UINT width, UINT height)
	: RenderInterface(width, height, L"PathTrace")
	, m_samplerTimer(0.0)
{
}

void PathTraceRenderer::Initialize()
{
	// DX12の初期化
	if (InitializeGraphicsDevice(Win32Application::GetHWND()) == false)
	{
		throw std::runtime_error("Failed Initialize Device.");
	}

	// Postの初期化をしておく
	InitializePostProcess();

	MaterialManager& materialManager = MaterialManager::GetInstance();
	MultiTLASPool& multiTLASPool = MultiTLASPool::GetInstance();

	// Max/Minの初期化
	DirectX::XMFLOAT3 minValueVec, maxValueVec;
	auto UpdateMinValue = [](float& target, const float& value)
		{
			if (value <= target)
			{
				target = value;
			}
		};

	auto UpdateMaxValue = [](float& target, const float& value)
		{
			if (target <= value)
			{
				target = value;
			}
		};

	{
		constexpr float maxValue = std::numeric_limits<float>::infinity();
		constexpr float minValue = std::numeric_limits<float>::min();

		maxValueVec.x = maxValueVec.y = maxValueVec.z = minValue;
		minValueVec.x = minValueVec.y = minValueVec.z = maxValue;
	}

	// BLAS
	m_blasProcessor = std::make_unique<BLASManager>();
	m_blasProcessor->SetDevice(m_device);
	m_blasProcessor->SetCommandProcessor(std::make_unique<Test3DGeometryProcessor>());

	// AABB
	{
		std::unique_ptr<ProcedualMeshBLAS> aabb = std::make_unique<ProcedualMeshBLAS>();
		aabb->SetScale(50.0f);
		m_blasProcessor->AddBLAS("AABB", std::move(aabb));
	}
	{
		m_meshLoader = std::make_unique<DefaultMeshLoader>();
		m_sponzaFileName = L"Mesh/SimpleSponza/sponza.obj";
		if (m_meshLoader->LoadMesh(m_device, m_sponzaFileName) == false)
		{
			throw std::runtime_error("Failed Initialize Mesh.");
		}

		m_moriFileName = L"Mesh/mitsuba/mitsuba-sphere.obj";
		if (m_meshLoader->LoadMesh(m_device, m_moriFileName) == false)
		{
			throw std::runtime_error("Failed Initialize Mesh.");
		}

		auto meshList = MeshPool::GetInstance().GetMesh(m_sponzaFileName);
		for (auto& mesh : *meshList)
		{
			std::unique_ptr<DefaultMeshBLAS> defaultMeshBLAS = std::make_unique<DefaultMeshBLAS>();
			std::string name = mesh->m_name;
			defaultMeshBLAS->SetupMesh(mesh);
			m_blasProcessor->AddBLAS(name, std::move(defaultMeshBLAS));

			// 最大最小を抽出
			UpdateMaxValue(maxValueVec.x, mesh->m_MaxPos.x);
			UpdateMaxValue(maxValueVec.y, mesh->m_MaxPos.y);
			UpdateMaxValue(maxValueVec.z, mesh->m_MaxPos.z);
			UpdateMinValue(minValueVec.x, mesh->m_MinPos.x);
			UpdateMinValue(minValueVec.y, mesh->m_MinPos.y);
			UpdateMinValue(minValueVec.z, mesh->m_MinPos.z);
		}

		meshList = MeshPool::GetInstance().GetMesh(m_moriFileName);
		for (auto& mesh : *meshList)
		{
			std::unique_ptr<DefaultMeshBLAS> defaultMeshBLAS = std::make_unique<DefaultMeshBLAS>();
			std::string name = mesh->m_name;
			defaultMeshBLAS->SetupMesh(mesh);
			m_blasProcessor->AddBLAS(name, std::move(defaultMeshBLAS));
		}
	}

	m_blasProcessor->Initialize();

	{
		// Directional
		std::unique_ptr<Light> directionalLight = std::make_unique<Light>();
		directionalLight->m_color = DirectX::g_XMOne;
		directionalLight->m_intensity = 1.0f;
		directionalLight->m_type = 1;
		directionalLight->m_direction = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		LightPool::GetInstance().RegisterLight(std::move(directionalLight));
	}

	int offset = 0;

	std::mt19937 mt(555);
	std::uniform_real_distribution<> randDistribution(0.0, 1.0);

	// Sphere
	/*
	for(int i= 0; i < 100; i++)
	{
		XMMATRIX mtx = DirectX::XMMatrixTranslation(
			minValueVec.x + randDistribution(mt) * (maxValueVec.x - minValueVec.x),
			minValueVec.y + randDistribution(mt) * (maxValueVec.y - minValueVec.y),
			minValueVec.z + randDistribution(mt) * (maxValueVec.z - minValueVec.z));

		auto tlas = std::make_unique<ProcedualMeshTLAS>();
		tlas->InitializeBLAS(static_cast<ProcedualMeshBLAS*>(m_blasProcessor->GetBlas("AABB")));
		tlas->SetInstanceId(0);
		tlas->SetInstanceHGIndex(offset);
		tlas->SetTransform(mtx);
		tlas->SetShaderIdentifier(PathTraceSphere);

		// NEE用のライトも用意
		auto areaLight = std::make_unique<SphereAreaLightData>();
		areaLight->m_position = DirectX::XMVector4Transform(DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), mtx);
		areaLight->m_emission = DirectX::XMVectorSet(10, 10, 10, 1.0f);
		areaLight->m_radius = 50.0f;
		AreaLightPool::GetInstance().RegisterAreaSphereLight(std::move(areaLight));

		auto analyticSphereMaterial = std::make_unique<AnalyticSphere>();
		analyticSphereMaterial->SetRadius(50.0f);
		analyticSphereMaterial->SetAABBCenter(DirectX::XMVectorSet(0.1f, 0.1f, 0.1f, 1.0f));

		analyticSphereMaterial->SetAlbedo(DirectX::XMVectorSet(10, 10, 10, 1.0f));
		analyticSphereMaterial->SetBRDFType(BRDFType::SimpleLight);

		tlas->SetMaterialIndex(materialManager.AddMaterial(std::move(analyticSphereMaterial)));

		multiTLASPool.AddTLAS(tlas);
		offset++;
	}
	*/

	for (int i = -1; i <= 1; i++)
	{
		XMMATRIX mtx = DirectX::XMMatrixTranslation(
			i * 60.0f, 30.0f, 30.0f);

		auto tlas = std::make_unique<ProcedualMeshTLAS>();
		tlas->InitializeBLAS(static_cast<ProcedualMeshBLAS*>(m_blasProcessor->GetBlas("AABB")));
		tlas->SetInstanceId(0);
		tlas->SetInstanceHGIndex(offset);
		tlas->SetTransform(mtx);
		tlas->SetShaderIdentifier(PathTraceSphere);

		// NEE用のライトも用意
		auto areaLight = std::make_unique<SphereAreaLightData>();
		areaLight->m_position = DirectX::XMVector4Transform(DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), mtx);
		areaLight->m_emission = DirectX::XMVectorSet(6, 6, 6, 1.0f);
		areaLight->m_radius = 20.0f;
		AreaLightPool::GetInstance().RegisterAreaSphereLight(std::move(areaLight));

		auto analyticSphereMaterial = std::make_unique<AnalyticSphere>();
		analyticSphereMaterial->SetRadius(20.0f);
		analyticSphereMaterial->SetAABBCenter(DirectX::XMVectorSet(0.1f, 0.1f, 0.1f, 1.0f));

		analyticSphereMaterial->SetAlbedo(DirectX::XMVectorSet(6, 6, 6, 1.0f));
		analyticSphereMaterial->SetBRDFType(BRDFType::SimpleLight);

		tlas->SetMaterialIndex(materialManager.AddMaterial(std::move(analyticSphereMaterial)));

		multiTLASPool.AddTLAS(tlas);
		offset++;
	}

	// mesh
	{
		auto meshList = MeshPool::GetInstance().GetMesh(m_sponzaFileName);
		for (auto mesh : *meshList)
		{
			auto name = mesh->m_name;
			auto blas = static_cast<DefaultMeshBLAS*>(m_blasProcessor->GetBlas(name));
			auto mesh = blas->GetMesh().lock();
			XMMATRIX mtx = DirectX::XMMatrixIdentity();

			auto tlas = std::make_unique<DefaultMeshTLAS>();
			tlas->InitializeBLAS(blas);
			tlas->SetInstanceId(0);
			tlas->SetInstanceHGIndex(offset);
			tlas->SetTransform(mtx);
			tlas->SetShaderIdentifier(PathTraceMesh);
			tlas->SetMaterialIndex(mesh->m_materialId);

			multiTLASPool.AddTLAS(tlas);
			offset++;
		}

		meshList = MeshPool::GetInstance().GetMesh(m_moriFileName);
		int moriIndex = 0;
		for (auto mesh : *meshList)
		{
			auto name = mesh->m_name;
			auto blas = static_cast<DefaultMeshBLAS*>(m_blasProcessor->GetBlas(name));
			auto mesh = blas->GetMesh().lock();
			XMMATRIX mtx = DirectX::XMMatrixTranslation(
				0.0f, -2.0f, 30.0f);

			// lambertを用意
			auto lambertMat = std::make_unique<SelfDefMaterial>();
			lambertMat->SetAlbedo(DirectX::XMVectorSet(0.718,0.483,0.000, 1.0f));
			lambertMat->SetSpecular(DirectX::XMVectorSet(0.5f, 0.5f, 0.5f, 0.1f));
			lambertMat->SetReflectColor(DirectX::XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f));
			lambertMat->SetEtaIn(0.5f);
			lambertMat->SetEtaOut(1.0f);
			lambertMat->SetMaterialName("Mori_BlinnPhong" + std::to_string(moriIndex));
			lambertMat->SetBRDFType(BRDFType::PT_Test);
			int lambertIndex = materialManager.AddMaterial(std::move(lambertMat));

			// Phongを用意
			auto phongMat = std::make_unique<SelfDefMaterial>();
			phongMat->SetAlbedo(DirectX::XMVectorSet(0.718, 0.483, 0.000, 1.0f));
			phongMat->SetSpecular(DirectX::XMVectorSet(0.5f, 0.5f, 0.5f, 0.4f));
			phongMat->SetReflectColor(DirectX::XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f));
			phongMat->SetEtaIn(0.5f);
			phongMat->SetEtaOut(1.0f);
			phongMat->SetMaterialName("Mori_Blinn" + std::to_string(moriIndex));
			phongMat->SetBRDFType(BRDFType::PT_Test);
			int phongIndex = materialManager.AddMaterial(std::move(phongMat));

			auto lambertTlas = std::make_unique<DefaultMeshTLAS>();
			lambertTlas->InitializeBLAS(blas);
			lambertTlas->SetInstanceId(0);
			lambertTlas->SetInstanceHGIndex(offset);
			lambertTlas->SetTransform(mtx);
			lambertTlas->SetShaderIdentifier(PathTraceMesh);
			lambertTlas->SetMaterialIndex(lambertIndex);

			multiTLASPool.AddTLAS(lambertTlas);
			offset++;
			
			mtx = DirectX::XMMatrixTranslation(
				 5.0f, -2.0f, 30.0f);

			auto phongTlas = std::make_unique<DefaultMeshTLAS>();
			phongTlas->InitializeBLAS(blas);
			phongTlas->SetInstanceId(0);
			phongTlas->SetInstanceHGIndex(offset);
			phongTlas->SetTransform(mtx);
			phongTlas->SetShaderIdentifier(PathTraceMesh);
			phongTlas->SetMaterialIndex(phongIndex);

			multiTLASPool.AddTLAS(phongTlas);
			offset++;

			moriIndex++;
		}
	}

	multiTLASPool.ConstructTLAS(m_device);
	materialManager.WriteAllMaterialData();

	// Global RootSignature
	m_grsProcessor = std::make_unique<CommonGRS>();
	m_grsProcessor->Initialize(m_device.get());

	// local RootSignature
	m_rsRGS = std::make_unique<PathTraceRGLRS>();
	m_rsRGS->Initialize(m_device.get());
	m_rsSphere = std::make_unique<AabbLRS>();
	m_rsSphere->Initialize(m_device.get());
	m_rsMesh = std::make_unique<DefaultMeshLRS>();
	m_rsMesh->Initialize(m_device.get());

	// State Object
	m_soProcessor = std::make_unique<PathTraceSO>();
	std::vector<ID3D12RootSignature*> LRSList;
	LRSList.push_back(m_rsRGS->GetLocalRS());
	LRSList.push_back(m_rsSphere->GetLocalRS());
	LRSList.push_back(m_rsMesh->GetLocalRS());
	m_soProcessor->Initialize(m_device.get(), m_grsProcessor->GetGlobalRS(), LRSList);

	// カメラ関係
	XMFLOAT3 eyePos(6.0f, 4.0f, 20.0f);
	XMFLOAT3 target(0.0f, 0.0f, 0.0f);
	m_camera.SetLookAt(eyePos, target);
	m_camera.SetPerspective(
		DirectX::XM_PIDIV4, GetAspect(), 0.1f, 100.0f
	);
	m_input->AddInputProcessor(&m_camera);

	// Scene
	m_sceneParam.loopCount = 0;
	m_sceneCB.Initialize(m_device, sizeof(SceneParam), L"SceneCB");

	// Sampler
	m_sampler = std::make_unique<PureRandom>();
	m_sampler->Setup(m_device);

	// Result
	m_resultBuffer = std::make_unique<ResultBuffer>();
	m_resultBuffer->Initialize(m_device.get(), GetWidth(), GetHeight());

	// Textureの用意
	m_hdrBackground = DXUTILITY::LoadTextureFromFile(m_device, L"lilienstein_4k.hdr");

	// Lightの用意
	LightPool& lightPool = LightPool::GetInstance();
	lightPool.Initialize(m_device);

	// AreaLightの用意
	AreaLightPool& areaLightPool = AreaLightPool::GetInstance();
	areaLightPool.Initialize(m_device);

	// Scene
	m_sceneParam.loopCount = 0;
	m_sceneParam.lightCount = AreaLightPool::GetInstance().GetLightNum();
	m_sceneCB.Initialize(m_device, sizeof(SceneParam), L"SceneCB");

	// shaderTable
	m_shaderTable = std::make_unique<PathTraceShaderTable>();
	m_shaderTable->InitializeDispatchRayDesc(GetWidth(), GetHeight(), 1);
	m_shaderTable->InitializeResultBuffer(m_resultBuffer->GetOutputDescriptor());
	m_shaderTable->Initialize(m_device, m_soProcessor->GetRtState());

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

void PathTraceRenderer::Destroy()
{
	m_device->waitForIdleGpu();

	m_postPSOList.clear();

	// 確保しているものを消していく
	m_blasProcessor.release();
	m_grsProcessor.release();
	m_rsRGS.release();
	m_rsSphere.release();
	m_rsMesh.release();
	m_soProcessor.release();
	m_resultBuffer.release();
	m_shaderTable.release();
	m_sampler.release();

	ImGui_ImplDX12_Shutdown();
	TerminateGraphicsDevice();
}

void PathTraceRenderer::Update(double DeltaTime)
{
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	auto framerate = ImGui::GetIO().Framerate;
	ImGui::Begin("Information");
	ImGui::BeginGroup();
	ImGui::Text("GPU: %s", m_device->GetAdapterName().c_str());
	ImGui::Text("Framerate: %.3f ms", 1000.0f / framerate);
	ImGui::Text("Spp: %d", m_sceneParam.loopCount);

	XMFLOAT3 camPos;
	XMStoreFloat3(&camPos, m_camera.GetPosition());
	ImGui::Text("CameraPos (%.2f, %.2f, %.2f)", camPos.x, camPos.y, camPos.z);
	ImGui::EndGroup();
	auto& lightList = LightPool::GetInstance().GetLights();
	ImGui::BeginChild("Light");
	for (auto& light : lightList)
	{
		ImGui::SliderFloat(light->m_name.c_str(), &light->m_intensity, 0.001f, 3.0f);

		if (light->m_type == 0)
		{
			ImGui::SliderFloat3("Position", light->m_position.m128_f32, -1000.0f, 1000.0f);
		}

		if (light->m_type == 1)
		{
			ImGui::SliderFloat3("Direction", light->m_direction.m128_f32, -1.0f, 1.0f);
		}
	}
	ImGui::EndChild();

	ImGui::Separator();
	ImGui::End();

	ImGui::Begin("Material");

	// mori begin
	auto meshList = MeshPool::GetInstance().GetMesh(m_moriFileName);
	auto index = MaterialManager::GetInstance().GetMaterialTail();
	for (int i = 0; i < index; i++)
	{
		auto* material = static_cast<DefaultMeshMaterial*>(MaterialManager::GetInstance().GetMaterial(i).get());
		if (material == nullptr)
		{
			continue;
		}

		auto name = material->GetMaterialName();
		if (name.find("Mori") == std::string::npos)
		{
			continue;
		}

		if (ImGui::TreeNode(name.c_str()))
		{
			XMVECTOR vec3Data = material->GetAlbedo();
			ImGui::ColorPicker3("albedo", vec3Data.m128_f32);
			material->SetAlbedo(vec3Data);

			vec3Data = material->GetSpecular();
			ImGui::ColorPicker3("specular", vec3Data.m128_f32);
			material->SetSpecular(vec3Data);

			float floatData = material->GetSpecularPow();
			ImGui::SliderFloat("shininess", &floatData, 0.0f, 1.0f);
			material->SetSpecularPow(floatData);

			ImGui::TreePop();
		}
	}
	// mori end

	/*
	auto index = MaterialManager::GetInstance().GetMaterialTail();
	for (int i = 0; i < index; i++)
	{
		auto* material = static_cast<DefaultMeshMaterial*>(MaterialManager::GetInstance().GetMaterial(i).get());
		if (material == nullptr)
		{
			continue;
		}

		std::string treeName = "material_" + std::to_string(i + 1);
		if (ImGui::TreeNode(treeName.c_str()))
		{
			XMVECTOR vec3Data = material->GetAlbedo();
			ImGui::ColorPicker3("albedo", vec3Data.m128_f32);
			material->SetAlbedo(vec3Data);

			vec3Data = material->GetSpecular();
			ImGui::SliderFloat3("specular", vec3Data.m128_f32, -30.0f, 30.0f);
			material->SetSpecular(vec3Data);

			float floatData = material->GetSpecularPow();
			ImGui::SliderFloat("shininess", &floatData, 0.0f, 1000.0f);
			material->SetSpecularPow(floatData);

			ImGui::TreePop();
		}
	}
	*/
	ImGui::End();

	m_sceneParam.mtxView = m_camera.GetViewMatrix();
	m_sceneParam.mtxProj = m_camera.GetProjectionMatrix();
	m_sceneParam.mtxViewInv = XMMatrixInverse(nullptr, m_sceneParam.mtxView);
	m_sceneParam.mtxProjInv = XMMatrixInverse(nullptr, m_sceneParam.mtxProj);

	m_sceneParam.ambientColor = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

	m_sceneParam.loopCount =
		(m_input->IsMouseMoved() == false && m_input->IsKeyboardInput() == false)
		? m_sceneParam.loopCount + 1 : 0;

	m_sceneParam.loopCount = std::clamp(m_sceneParam.loopCount, 0u, MAX_LOOP_COUNT);

	// とりあえず現状はマウス入力のみ
	m_isClear = m_input->IsMouseMoved() || m_input->IsKeyboardInput();

	// Samplerの更新
	m_samplerTimer += m_time->GetDeltaTime();
	if (m_samplerTimer > 10.0)
	{
		m_samplerTimer = 0.0;
		m_sampler->Update();
	}

	for (auto& post : m_postPSOList)
	{
		post->Update(DeltaTime);
		auto average = std::dynamic_pointer_cast<AveragePSO>(post);
		if (average)
		{
			average->Updatespp(m_sceneParam.loopCount);
		}
	}
}

void PathTraceRenderer::OnRender()
{
	auto device = m_device->GetDevice();
	auto renderTarget = m_device->GetRT();
	auto allocator = m_device->GetCurrentCommandAllocator();
	allocator->Reset();
	m_commandList->Reset(allocator.Get(), nullptr);
	auto frameIndex = m_device->GetCurrentFrameIndex();

	//scene関係のconstant buffer
	m_sceneCB.Write(frameIndex, &m_sceneParam, sizeof(m_sceneParam));
	auto sceneConstantBuffer = m_sceneCB.Get(frameIndex);

	// Lightの書き込み
	LightPool::GetInstance().UpdateData(frameIndex);
	AreaLightPool::GetInstance().UpdateData(frameIndex);

	// Materialの書き込み
	MaterialManager& materialManager = MaterialManager::GetInstance();
	MultiTLASPool& multiTLASPool = MultiTLASPool::GetInstance();
	materialManager.UpdateMaterialData(frameIndex);

	ID3D12DescriptorHeap* descriptorHeaps[] =
	{
		m_device->GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV).Get(),
	};
	m_commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	if (m_isClear)
	{
		float color[4] = { 0.0f };
		// COPYからRenderTargetへ
		auto barrierToRTForClear = CD3DX12_RESOURCE_BARRIER::Transition
		(
			m_resultBuffer->GetOutputBuffer().Get(),
			D3D12_RESOURCE_STATE_COPY_SOURCE,
			D3D12_RESOURCE_STATE_RENDER_TARGET
		);

		// 結果をリセット
		m_commandList->ResourceBarrier(1, &barrierToRTForClear);
		m_commandList->ClearRenderTargetView(m_resultBuffer->GetOutputRTVDescriptor()->m_cpuHandle, color, 0, nullptr);

		// UAVに遷移させる
		auto barrierToUAVForClear = CD3DX12_RESOURCE_BARRIER::Transition
		(
			m_resultBuffer->GetOutputBuffer().Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS
		);
		m_commandList->ResourceBarrier(1, &barrierToUAVForClear);
	}
	else
	{
		//raytracing結果のBufferをUAV状態へ
		auto barrierToUAV = CD3DX12_RESOURCE_BARRIER::Transition(
			m_resultBuffer->GetOutputBuffer().Get(),
			D3D12_RESOURCE_STATE_COPY_SOURCE,
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS
		);
		m_commandList->ResourceBarrier(1, &barrierToUAV);
	}

	// MAXを超えるまでRayを飛ばす
	if (m_sceneParam.loopCount < MAX_LOOP_COUNT)
	{
		m_commandList->SetComputeRootSignature(m_grsProcessor->GetGlobalRS().Get());
		m_commandList->SetComputeRootDescriptorTable(0, multiTLASPool.GetDescriptor().m_gpuHandle);
		m_commandList->SetComputeRootConstantBufferView(1, sceneConstantBuffer->GetGPUVirtualAddress());
		m_commandList->SetComputeRootDescriptorTable(2, LightPool::GetInstance().GetDescriptor(frameIndex).m_gpuHandle);
		m_commandList->SetComputeRootDescriptorTable(3, AreaLightPool::GetInstance().GetDescriptor(frameIndex).m_gpuHandle);
		m_commandList->SetComputeRootDescriptorTable(4, m_hdrBackground.srv.m_gpuHandle);
		m_commandList->SetComputeRootDescriptorTable(5, m_sampler->GetNoiseDescriptor().m_gpuHandle);
		m_commandList->SetComputeRootDescriptorTable(6, m_sampler->GetSeedDescriptor().m_gpuHandle);

		m_commandList->SetPipelineState1(m_soProcessor->GetRtState().Get());
		m_commandList->DispatchRays(&m_shaderTable->GetDispatchRayDesc(frameIndex));
	}

	// Post対応
	RenderToTexture();

	//UI描画
	auto rtv = m_device->GetRTV();
	const auto& viewport = m_device->GetDefaultViewport();
	m_commandList->OMSetRenderTargets(1, &rtv, FALSE, nullptr);
	m_commandList->RSSetViewports(1, &viewport);

	ImGui::Render();
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_commandList.Get());

	//present可能な用にbarrierをset
	auto barrierToPresent = CD3DX12_RESOURCE_BARRIER::Transition
	(
		renderTarget.Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT
	);

	m_commandList->ResourceBarrier(1, &barrierToPresent);

	m_commandList->Close();

	m_device->ExecuteCommandList(m_commandList);
	m_device->Present(1);
}

void PathTraceRenderer::InitializePostProcess()
{
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

	// Post関係のPSOを政策
	std::unique_ptr<PostPSO> postPSO;
	{
		// 平均化用
		postPSO = std::make_unique<AveragePSO>(m_device);

		postPSO->SetVSByte(L"SimpleVS.dxlib");
		postPSO->SetPSByte(L"AveragePS.dxlib");
		postPSO->SetRootSignature(m_postRootSignature);
		postPSO->CreatePSO(m_device->GetDevice());
		m_postPSOList.emplace_back(std::move(postPSO));
	}
}

void PathTraceRenderer::RenderToTexture()
{
	auto renderTarget = m_device->GetRT();

	//raytracingの結果をbackbufferへcopyする
	D3D12_RESOURCE_BARRIER barriers[] = {
		// DXR用からPostのPSで使えるようにする
		CD3DX12_RESOURCE_BARRIER::Transition(
			m_resultBuffer->GetOutputBuffer().Get(),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
		),
		// renderTargetはRTとして利用するように切り替えておく
		CD3DX12_RESOURCE_BARRIER::Transition(
			renderTarget.Get(),
			D3D12_RESOURCE_STATE_PRESENT,
			D3D12_RESOURCE_STATE_RENDER_TARGET
		),
	};

	m_commandList->ResourceBarrier(_countof(barriers), barriers);

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
			postPSO->Draw(m_commandList, m_resultBuffer->GetOutputDescriptor()->m_gpuHandle);
		}
	}

	// 元の路線に戻しておく
	auto barrierToUAV = CD3DX12_RESOURCE_BARRIER::Transition(
		m_resultBuffer->GetOutputBuffer().Get(),
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_COPY_SOURCE
	);

	m_commandList->ResourceBarrier(1, &barrierToUAV);
}
