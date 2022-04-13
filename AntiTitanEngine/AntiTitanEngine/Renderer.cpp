#include "stdafx.h"
#include "Renderer.h"

Renderer::Renderer()
{
}

Renderer::~Renderer()
{
}

std::shared_ptr<RenderPrimitiveManager> Renderer::GetRenderPrimitiveManager()
{
	return mRenderPrimitiveManager;
}

bool Renderer::Init()
{
	Engine::Get()->GetAssetManager()->LoadExternalMapActor(MapActorLoadPath);
	mRenderPrimitiveManager = std::make_shared<RenderPrimitiveManager>();
	mCamera = std::make_shared<Camera>();
	mCamera->SetLens(0.25f * MathHelper::Pi, static_cast<float>(mClientWidth) / mClientHeight, 1.0f, 100000.0f);
	
	mRHI = std::make_shared<DXRHI>();
	mRHI->OpenDebugLayer();
	mRHI->InitPrimitiveManagerMember();
	CreateHeap();
	CreateRenderTarget();
	mRHI->ResetCommandList();
	mRHI->ExecuteCommandList();
	mRHI->WaitCommandComplete();
	mRHI->ResetCommandList();
	mRHI->BuildDescriptorHeaps();
	mRHI->BuildRootSignature();
	CreateShader();
	CreatePipeline();
	LoadMeshAndSetBuffer();
	mRHI->ExecuteCommandList();
	mRHI->WaitCommandComplete();
	return true;
}

void Renderer::CreateHeap()
{//Heap创建

	//base===============================================================================

	//基础Rtv
	mRenderPrimitiveManager->InsertHeapToLib(
		"mRtvHeap",mRHI->CreateDescriptorHeap("mRtvHeap", 2, RTV, NONE));

	//基础Dsv
	mRenderPrimitiveManager->InsertHeapToLib(
		"mDsvHeap",mRHI->CreateDescriptorHeap("mDsvHeap", 100, DSV, NONE));

	mRenderPrimitiveManager->InsertHeapToLib(
		"mCbvHeap",mRHI->CreateDescriptorHeap("mCbvHeap", 10000, CBVSRVUAV, SHADER_VISIBLE));
	//shadow===============================================================================

	//单独给shadow用的SrvHeap
	mRenderPrimitiveManager->InsertHeapToLib(
		"mShadowSrvDescriptorHeap",mRHI->CreateDescriptorHeap("mShadowSrvDescriptorHeap", 10000, CBVSRVUAV, SHADER_VISIBLE));

	//单独给shadow用的SrvHeap
	mRenderPrimitiveManager->InsertHeapToLib(
		"mShadowDsvDescriptorHeap",mRHI->CreateDescriptorHeap("mShadowDsvDescriptorHeap", 10000, DSV, NONE));

	//bloom===============================================================================
	//BloomRtvHeap
	mRenderPrimitiveManager->InsertHeapToLib(
		"BloomRtvHeap", mRHI->CreateDescriptorHeap("BloomRtvHeap", 2, RTV, NONE));
	//BloomDsvHeap
	mRenderPrimitiveManager->InsertHeapToLib(
		"BloomDsvHeap", mRHI->CreateDescriptorHeap("BloomDsvHeap", 100, DSV, NONE));
	//BloomSrvHeap
	mRenderPrimitiveManager->InsertHeapToLib(
		"BloomSrvHeap", mRHI->CreateDescriptorHeap("BloomSrvHeap", 10000, CBVSRVUAV, SHADER_VISIBLE));
}

void Renderer::CreateShader()
{
	mRenderPrimitiveManager->InsertShaderToLib("color", 
		mRHI->CreateShader("color", ShaderPath));

	mRenderPrimitiveManager->InsertShaderToLib("shadow", 
		mRHI->CreateShader("shadow", L"Shaders\\Shadows.hlsl"));

	mRenderPrimitiveManager->InsertShaderToLib("bloom",
		mRHI->CreateShader("bloom", L"Shaders\\bloom.hlsl"));
}

void Renderer::CreatePipeline()
{
	//基础管线====================================================================
	//PipelineState_DESC basePipDesc;
	//basePipDesc.rasterizeDesc.FrontCounterClockwise = true;

	auto baseShader = mRenderPrimitiveManager->GetShaderByName("color");
	mRenderPrimitiveManager->InsertPipelineToLib("basePipeline",
		mRHI->CreatePipeline("basePipeline", baseShader, 1, RenderTargetFormat_R8G8B8A8_UNORM, false));

	//阴影管线=====================================================================
	//PipelineState_DESC shadowPipDesc;
	//shadowPipDesc.rasterizeDesc.FrontCounterClockwise = true;
	//shadowPipDesc.rasterizeDesc.DepthBias = 40000;
	//shadowPipDesc.rasterizeDesc.DepthBiasClamp = 0.0f;
	//shadowPipDesc.rasterizeDesc.SlopeScaledDepthBias = 1.0f;
	auto shadowShader = mRenderPrimitiveManager->GetShaderByName("shadow");
	mRenderPrimitiveManager->InsertPipelineToLib("shadowPipeline",
		mRHI->CreatePipeline("shadowPipeline", shadowShader, 0, RenderTargetFormat_UNKNOWN, true));

	//Bloom管线===================================================================
	auto bloomShader = mRenderPrimitiveManager->GetShaderByName("bloom");
	mRenderPrimitiveManager->InsertPipelineToLib("bloomPipeline",
		mRHI->CreatePipeline("bloomPipeline", bloomShader, 1, RenderTargetFormat_R32G32B32A32_FLOAT, false));
}

void Renderer::CreateRenderTarget()
{
	//基础RenderTarget=======================================================
	auto baseRenderTarget = mRHI->CreateRenderTarget(
		"baseRenderTarget", TEXTURE2D, STATE_DEPTH_WRITE, D24_UNORM_S8_UINT,
		mRenderPrimitiveManager->GetHeapByName("mRtvHeap"),
		nullptr,
		mRenderPrimitiveManager->GetHeapByName("mDsvHeap"),
		2,
		mClientWidth,
		mClientHeight);
	mRenderPrimitiveManager->InsertRenderTargetToLib("baseRenderTarget", baseRenderTarget);
	
	//阴影RenderTarget=====================================================
	auto shadowRenderTarget = mRHI->CreateRenderTarget(
		"shadowRenderTarget", TEXTURE2D, STATE_DEPTH_WRITE , D24_UNORM_S8_UINT,
		nullptr,
		mRenderPrimitiveManager->GetHeapByName("mShadowSrvDescriptorHeap"),
		mRenderPrimitiveManager->GetHeapByName("mShadowDsvDescriptorHeap"),
		0,
		2048,
		2048);
	mRenderPrimitiveManager->InsertRenderTargetToLib("shadowRenderTarget", shadowRenderTarget);

	//BloomRenderTarget=====================================================
	//auto bloomRenderTarget = mRHI->CreateRenderTarget(
	//	"bloomRenderTarget", TEXTURE2D, STATE_DEPTH_WRITE, D24_UNORM_S8_UINT,
	//	mRenderPrimitiveManager->GetHeapByName("BloomRtvHeap"),
	//	mRenderPrimitiveManager->GetHeapByName("BloomSrvHeap"),
	//	mRenderPrimitiveManager->GetHeapByName("BloomDsvHeap"),
	//	1,
	//	mClientWidth,
	//	mClientHeight);
	//mRenderPrimitiveManager->InsertRenderTargetToLib("bloomRenderTarget", shadowRenderTarget);
	//不确定这个RenderTarget能不能用，刚创建完这个RenderTarget
	//接下来渲染成一个Srv传到BaseShader里
}

void Renderer::LoadMeshAndSetBuffer()
{
	std::string StaticMeshPath;
	std::set<std::string> StaticMeshs;//用于去重
	Engine::Get()->GetAssetManager()->GetGeometryLibrary()->resize(Engine::Get()->GetAssetManager()->GetMapActorInfo()->Size());

	//循环加入MeshGeometry
	for (int i = 0; i < Engine::Get()->GetAssetManager()->Geos.size(); i++)
	{
		std::shared_ptr<StaticMesh> mesh = std::make_shared<StaticMesh>();

		//相同的StaticMesh不用重复建立
		auto check = StaticMeshs.find(Engine::Get()->GetAssetManager()->GetMapActorInfo()->MeshNameArray[i]);
		if (check == StaticMeshs.end()) {
			StaticMeshs.insert(Engine::Get()->GetAssetManager()->GetMapActorInfo()->MeshNameArray[i]);
			Engine::Get()->GetAssetManager()->GetMapofGeosMesh()->insert(std::pair<int, std::string>(i, Engine::Get()->GetAssetManager()->GetMapActorInfo()->MeshNameArray[i]));
		}
		else { continue; }

		//读取mesh信息
		StaticMeshPath = "SplitMesh/" + Engine::Get()->GetAssetManager()->GetMapActorInfo()->MeshNameArray[i];
		StaticMeshPath.erase(StaticMeshPath.length() - 1);
		StaticMeshPath += ".bat";
		mesh->LoadStaticMeshFromBat(StaticMeshPath);
		
		mesh->SetMeshBuffer(mRHI->CreateMeshBuffer(mesh));

		//Mesh添加Texture
		mesh->material = std::make_shared<FMaterial>();
		auto texture = mRHI->CreateTexture("TestTexture", TextureLoadPath,2000);
		mesh->material->InsertTextureToTexLib("TestTexture", texture);

		auto Normaltexture = mRHI->CreateTexture("NormalTexture", NormalLoadPath, 2001);
		mesh->material->InsertTextureToTexLib("NormalTexture", Normaltexture);

		Engine::Get()->GetAssetManager()->InsertStaticMeshToLib(mesh->GetMeshName(), mesh);
	}
}

void Renderer::Update()
{
	//mRHI->Update();
	for (int index = 0; index < Engine::Get()->GetAssetManager()->GetMapActorInfo()->Size(); index++)
	{
		ObjectConstants objConstants;
		UpdateShadow(index, objConstants);
		UpdateMesh(index, objConstants);
		mRHI->CommitResourceToGPU(index,objConstants);
	}
	mRHI->CalculateFrameStats();
}

void Renderer::Draw()
{
	mRHI->DrawReset();

	DrawShadowPass();
	DrawScenePass();

	mRHI->DrawFinal();
}

void Renderer::UpdateMesh(int i, ObjectConstants& objConstants)
{
	//UPDate Actor--------------------------------------------------------------------------------
	//auto world = MathHelper::Identity4x4();
	auto location = XMMatrixTranslation(
		Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsTransformArray[i].translation.x,
		Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsTransformArray[i].translation.y,
		Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsTransformArray[i].translation.z
	);
	auto Scale = XMMatrixScaling(
		Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsTransformArray[i].scale3D.x,
		Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsTransformArray[i].scale3D.y,
		Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsTransformArray[i].scale3D.z
	);
	auto Rotator = XMMatrixScaling(
		Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsTransformArray[i].rotation.Pitch,
		Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsTransformArray[i].rotation.Yaw,
		Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsTransformArray[i].rotation.Roll
	);

	DirectX::XMVECTORF32 g_XMIdentityR3 = { { {
			Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsQuatArray[i].X,
			Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsQuatArray[i].Y,
			Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsQuatArray[i].Z,
			Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsQuatArray[i].W
		} } };

	auto mrotation = DirectX::XMMatrixRotationQuaternion(g_XMIdentityR3);

	glm::mat4 translateMat4 = glm::translate(glm::identity<glm::mat4>(), glm::vec3(
		Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsTransformArray[i].translation.x,
		Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsTransformArray[i].translation.y,
		Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsTransformArray[i].translation.z
	));

	glm::mat4 scaleMat4 = glm::scale(glm::identity<glm::mat4>(), glm::vec3(
		Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsTransformArray[i].scale3D.x,
		Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsTransformArray[i].scale3D.y,
		Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsTransformArray[i].scale3D.z
	));

	glm::quat rotationQuat(
		Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsQuatArray[i].X,
		Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsQuatArray[i].Y,
		Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsQuatArray[i].Z,
		Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsQuatArray[i].W
	);
	glm::mat4 rotationMat4 = glm::toMat4(rotationQuat);

	auto world = Scale * mrotation * location;
	glm::mat4 worldMat4 = scaleMat4 * rotationMat4 * translateMat4;
	//!!!旋转矩阵好像有问题，用glm的rotator传给Shader，mul(Normal,rotator)的值不对，表现出来的颜色不是正确的。
	//!!!要看正确的颜色可以乘XMMATRIX的rotation，XMMATRIX还要转化为FLOAT4X4

	auto mCamera = Engine::Get()->GetRenderer()->GetCamera();
	XMMATRIX worldViewProj = world * XMLoadFloat4x4(&mCamera->GetView4x4f()) * XMLoadFloat4x4(&mCamera->GetProj4x4f());
	glm::mat4 worldViewProjMat4 = mCamera->GetProjMat4() * mCamera->GetViewMat4() * worldMat4;

	//XMMATRIX worldViewProj = world * XMLoadFloat4x4(&mCamera.GetView4x4f()) * XMLoadFloat4x4((&mCamera.GetProj4x4f()));
	//glm::mat4 worldViewProjMat4 = mCamera.GetProjMat4() * mCamera.GetViewMat4() * worldMat4;

	XMStoreFloat4x4(&objConstants.WorldViewProj, XMMatrixTranspose(worldViewProj));
	objConstants.WorldViewProjMat4 = glm::transpose(worldViewProjMat4);
	//objConstants.mTime = Engine::Get()->gt.TotalTime();
	XMStoreFloat4x4(&objConstants.rotation, XMMatrixTranspose(mrotation));

	objConstants.LightDirection.x = Engine::Get()->GetAssetManager()->GetLight()->mLook.x;
	objConstants.LightDirection.y = Engine::Get()->GetAssetManager()->GetLight()->mLook.y;
	objConstants.LightDirection.z = Engine::Get()->GetAssetManager()->GetLight()->mLook.z;
	objConstants.LightDirection.w = 0.0f;

	auto meshName = Engine::Get()->GetAssetManager()->GetMapActorInfo()->MeshNameArray[i];
	auto mesh = Engine::Get()->GetAssetManager()->GetStaticMeshByName(meshName);

	objConstants.LightStrength  = Engine::Get()->GetAssetManager()->GetLight()->Strength;
	objConstants.gDiffuseAlbedo = mesh->material->DiffuseAlbedo;
	objConstants.gFresnelR0= mesh->material->FresnelR0;
	objConstants.gRoughness= mesh->material->Roughness;
	objConstants.LightLocation = Engine::Get()->GetAssetManager()->GetLight()->mLightInfo.mTransform.translation;
	objConstants.LightLocationW = 1;
	objConstants.CameraLocation = GetCamera()->GetPosition3f();
	objConstants.CameraLocationW = 1;
}

void Renderer::UpdateShadow(int i, ObjectConstants& objConstants)
{
	auto location = XMMatrixTranslation(
		Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsTransformArray[i].translation.x,
		Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsTransformArray[i].translation.y,
		Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsTransformArray[i].translation.z
	);
	auto Scale = XMMatrixScaling(
		Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsTransformArray[i].scale3D.x,
		Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsTransformArray[i].scale3D.y,
		Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsTransformArray[i].scale3D.z
	);
	auto Rotator = XMMatrixScaling(
		Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsTransformArray[i].rotation.Pitch,
		Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsTransformArray[i].rotation.Yaw,
		Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsTransformArray[i].rotation.Roll
	);

	DirectX::XMVECTORF32 g_XMIdentityR3 = { { {
			Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsQuatArray[i].X,
			Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsQuatArray[i].Y,
			Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsQuatArray[i].Z,
			Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsQuatArray[i].W
		} } };

	auto mrotation = DirectX::XMMatrixRotationQuaternion(g_XMIdentityR3);

	auto world = Scale * mrotation * location;

	auto TestVV = Engine::Get()->GetAssetManager()->mLight->GetView();//重新创建的V矩阵
	auto TestPP = Engine::Get()->GetAssetManager()->mLight->GetProj();//重新创建的P矩阵

	XMMATRIX T(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	////重新造的VP
	XMMATRIX VP = TestVV * TestPP;
	XMMATRIX S = TestVV * TestPP * T;
	XMMATRIX LightworldViewProj = world * TestVV * TestPP;
	XMMATRIX LightworldViewProjT = world * TestVV * TestPP * T;

	XMStoreFloat4x4(&objConstants.gWorld, XMMatrixTranspose(world));
	XMStoreFloat4x4(&objConstants.gLightVP, XMMatrixTranspose(VP));
	XMStoreFloat4x4(&objConstants.gShadowTransform, XMMatrixTranspose(S));
	XMStoreFloat4x4(&objConstants.gLightMVP, XMMatrixTranspose(LightworldViewProj));
	XMStoreFloat4x4(&objConstants.gLightMVPT, XMMatrixTranspose(LightworldViewProjT));
}

void Renderer::DrawShadowPass()
{
	auto Rendertarget = mRenderPrimitiveManager->GetRenderTargetByName("shadowRenderTarget");
	mRHI->SetScreenSetViewPort(
		Rendertarget->width,
		Rendertarget->height);
	mRHI->SetScissorRect(
		long(Rendertarget->width),
		long(Rendertarget->height));
	mRHI->ClearDepthStencilView(Rendertarget);
	mRHI->OMSetRenderTargets(Rendertarget);

	for (int ActorIndex = 0; ActorIndex < Engine::Get()->GetAssetManager()->GetMapActorInfo()->Size(); ActorIndex++)
	{
		mRHI->SetDescriptorHeap(mRenderPrimitiveManager->GetHeapByName("mCbvHeap"));
		mRHI->SetPipelineState(mRenderPrimitiveManager->GetPipelineByName("shadowPipeline"));
		mRHI->DrawMesh(ActorIndex, 0);
		mRHI->CommitShaderParameters();//这个提交shader函数没有做完
	}
}

void Renderer::DrawScenePass()
{
	auto Rendertarget = mRenderPrimitiveManager->GetRenderTargetByName("baseRenderTarget");

	mRHI->SetScreenSetViewPort(
		Rendertarget->width,
		Rendertarget->height);
	mRHI->SetScissorRect(
		long(Rendertarget->width),
		long(Rendertarget->height));
	mRHI->ResourceBarrier();
	mRHI->ClearRenderTargetView(Rendertarget, mClearColor, 0);
	mRHI->ClearDepthStencilView(Rendertarget);
	mRHI->OMSetRenderTargets(Rendertarget);
	mRHI->SetPipelineState(mRenderPrimitiveManager->GetPipelineByName("basePipeline"));
	mRHI->SetDescriptorHeap(mRenderPrimitiveManager->GetHeapByName("mCbvHeap"));
	
	for (int ActorIndex = 0; ActorIndex < Engine::Get()->GetAssetManager()->GetMapActorInfo()->Size(); ActorIndex++)
	{
		mRHI->DrawMesh(ActorIndex, 0);
	}
}

void Renderer::Destroy()
{
	mRHI = nullptr;
}

std::shared_ptr<RHI> Renderer::GetRHI()
{
	return mRHI;
}

std::shared_ptr<Camera> Renderer::GetCamera()
{
	return mCamera;
}

