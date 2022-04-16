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
	
	std::shared_ptr<StaticMesh> triangle = std::make_shared<StaticMesh>();
	triangle->meshBuffer = mRHI->CreateTriangleMeshBuffer();
	Engine::Get()->GetAssetManager()->InsertStaticMeshToLib("triangle", triangle);

	mRHI->ExecuteCommandList();
	mRHI->WaitCommandComplete();
	return true;
}

void Renderer::CreateHeap()
{//Heap创建

	//base===============================================================================

	//基础Rtv
	mRenderPrimitiveManager->InsertHeapToLib(
		"mRtvHeap",mRHI->CreateDescriptorHeap("mRtvHeap", 100, RTV, NONE));

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
		"BloomRtvHeap", mRHI->CreateDescriptorHeap("BloomRtvHeap", 1000, RTV, NONE));
	//BloomDsvHeap
	mRenderPrimitiveManager->InsertHeapToLib(
		"BloomDsvHeap", mRHI->CreateDescriptorHeap("BloomDsvHeap", 1000, DSV, NONE));
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

	mRenderPrimitiveManager->InsertShaderToLib("HDR",
		mRHI->CreateShader("HDR", L"Shaders\\HDR.hlsl"));

	mRenderPrimitiveManager->InsertShaderToLib("bloomSetup",
		mRHI->CreateShader("bloomSetup", L"Shaders\\bloomSetup.hlsl"));
	
	mRenderPrimitiveManager->InsertShaderToLib("bloomDown",
		mRHI->CreateShader("bloomDown", L"Shaders\\bloomDown1.hlsl"));

	mRenderPrimitiveManager->InsertShaderToLib("bloomUp",
		mRHI->CreateShader("bloomUp", L"Shaders\\bloomUp.hlsl"));

	mRenderPrimitiveManager->InsertShaderToLib("bloomMerge",
		mRHI->CreateShader("bloomMerge", L"Shaders\\bloomMerge.hlsl"));

	mRenderPrimitiveManager->InsertShaderToLib("ToneMap",
		mRHI->CreateShader("ToneMap", L"Shaders\\ToneMap.hlsl"));
}

void Renderer::CreatePipeline()
{
	//基础管线====================================================================
	auto baseShader = mRenderPrimitiveManager->GetShaderByName("color");
	mRenderPrimitiveManager->InsertPipelineToLib("basePipeline",
		mRHI->CreatePipeline("basePipeline", baseShader, 1, RenderTargetFormat_R8G8B8A8_UNORM, false));

	//阴影管线=====================================================================
	auto shadowShader = mRenderPrimitiveManager->GetShaderByName("shadow");
	mRenderPrimitiveManager->InsertPipelineToLib("shadowPipeline",
		mRHI->CreatePipeline("shadowPipeline", shadowShader, 0, RenderTargetFormat_UNKNOWN, true));

	//Bloom管线===================================================================
	auto HDRShader = mRenderPrimitiveManager->GetShaderByName("HDR");
	mRenderPrimitiveManager->InsertPipelineToLib("HDRPipeline",
		mRHI->CreatePipeline("HDRPipeline", HDRShader, 1, RenderTargetFormat_R16G16B16A16_FLOAT, false));

	auto bloomSetUpShader = mRenderPrimitiveManager->GetShaderByName("bloomSetup");
	mRenderPrimitiveManager->InsertPipelineToLib("bloomSetupPipeline",
		mRHI->CreatePipeline("bloomSetupPipeline", bloomSetUpShader, 1, RenderTargetFormat_R16G16B16A16_FLOAT, false));

	auto bloomDownShader = mRenderPrimitiveManager->GetShaderByName("bloomDown");
	mRenderPrimitiveManager->InsertPipelineToLib("bloomDownPipeline",
		mRHI->CreatePipeline("bloomDownPipeline", bloomDownShader, 1, RenderTargetFormat_R16G16B16A16_FLOAT, false));
	
	auto bloomUpShader = mRenderPrimitiveManager->GetShaderByName("bloomUp");
	mRenderPrimitiveManager->InsertPipelineToLib("bloomUpPipeline",
		mRHI->CreatePipeline("bloomUpPipeline", bloomUpShader, 1, RenderTargetFormat_R16G16B16A16_FLOAT, false));

	auto bloomMergeShader = mRenderPrimitiveManager->GetShaderByName("bloomMerge");
	mRenderPrimitiveManager->InsertPipelineToLib("bloomMergePipeline",
		mRHI->CreatePipeline("bloomMergePipeline", bloomMergeShader, 1, RenderTargetFormat_R16G16B16A16_FLOAT, false));
	
	auto ToneMapShader = mRenderPrimitiveManager->GetShaderByName("ToneMap");
	mRenderPrimitiveManager->InsertPipelineToLib("ToneMapPipeline",
		mRHI->CreatePipeline("ToneMapPipeline", ToneMapShader, 1, RenderTargetFormat_R8G8B8A8_UNORM, false));
}

void Renderer::CreateRenderTarget()
{
	//基础RenderTarget=======================================================
	{
	auto baseRenderTarget = mRHI->CreateRenderTarget(
		"baseRenderTarget", TEXTURE2D, STATE_DEPTH_WRITE, R8G8B8A8_UNORM,
		mRenderPrimitiveManager->GetHeapByName("mRtvHeap"),0,
		nullptr,0,
		mRenderPrimitiveManager->GetHeapByName("mDsvHeap"),0,
		true, 2, mClientWidth,mClientHeight);
	mRenderPrimitiveManager->InsertRenderTargetToLib("baseRenderTarget", baseRenderTarget);
	}
	//阴影RenderTarget=====================================================
	{
	auto shadowRenderTarget = mRHI->CreateRenderTarget(
		"shadowRenderTarget", TEXTURE2D, STATE_DEPTH_WRITE , D24_UNORM_S8_UINT,
		nullptr, 0,
		mRenderPrimitiveManager->GetHeapByName("mShadowSrvDescriptorHeap"), 0,
		mRenderPrimitiveManager->GetHeapByName("mShadowDsvDescriptorHeap"), 0,
		false, 0, 2048,2048);
	mRenderPrimitiveManager->InsertRenderTargetToLib("shadowRenderTarget", shadowRenderTarget);
	}
	//BloomRenderTarget=====================================================
	{ 
		{
			auto HDRRenderTarget = mRHI->CreateRenderTarget(
				"HDRRenderTarget", TEXTURE2D, STATE_DEPTH_WRITE, R16G16B16A16_FLOAT,
				mRenderPrimitiveManager->GetHeapByName("BloomRtvHeap"), 0,
				mRenderPrimitiveManager->GetHeapByName("BloomSrvHeap"), 0,
				mRenderPrimitiveManager->GetHeapByName("BloomDsvHeap"), 0,
				false, 1, mClientWidth, mClientHeight);
			mRenderPrimitiveManager->InsertRenderTargetToLib("HDRRenderTarget", HDRRenderTarget);
		}
		//------------------------------------
		{
			auto bloomSetupRenderTarget = mRHI->CreateRenderTarget(
				"bloomSetupRenderTarget", TEXTURE2D, STATE_DEPTH_WRITE, R16G16B16A16_FLOAT,
				mRenderPrimitiveManager->GetHeapByName("BloomRtvHeap"), 1,
				mRenderPrimitiveManager->GetHeapByName("BloomSrvHeap"), 1,
				mRenderPrimitiveManager->GetHeapByName("BloomDsvHeap"), 1,
				false, 1, mClientWidth / 4, mClientHeight / 4);
			mRenderPrimitiveManager->InsertRenderTargetToLib("bloomSetupRenderTarget", bloomSetupRenderTarget);
		}
		//------------------------------------
		{
			auto bloomDown2RenderTarget = mRHI->CreateRenderTarget(
				"bloomDown2RenderTarget", TEXTURE2D, STATE_DEPTH_WRITE, R16G16B16A16_FLOAT,
				mRenderPrimitiveManager->GetHeapByName("BloomRtvHeap"), 2,
				mRenderPrimitiveManager->GetHeapByName("BloomSrvHeap"), 2,
				mRenderPrimitiveManager->GetHeapByName("BloomDsvHeap"), 2,
				false, 1, (mClientWidth / 4) / 2, (mClientHeight / 4) / 2);
			mRenderPrimitiveManager->InsertRenderTargetToLib("bloomDown2RenderTarget", bloomDown2RenderTarget);
		}
		//------------------------------------
		{
			auto bloomDown3RenderTarget = mRHI->CreateRenderTarget(
				"bloomDown3RenderTarget", TEXTURE2D, STATE_DEPTH_WRITE, R16G16B16A16_FLOAT,
				mRenderPrimitiveManager->GetHeapByName("BloomRtvHeap"), 3,
				mRenderPrimitiveManager->GetHeapByName("BloomSrvHeap"), 3,
				mRenderPrimitiveManager->GetHeapByName("BloomDsvHeap"), 3,
				false, 1, ((mClientWidth / 4) / 2)/2, ((mClientHeight / 4) / 2)/2);
			mRenderPrimitiveManager->InsertRenderTargetToLib("bloomDown3RenderTarget", bloomDown3RenderTarget);
		}
		//------------------------------------
		{
			auto bloomDown4RenderTarget = mRHI->CreateRenderTarget(
				"bloomDown4RenderTarget", TEXTURE2D, STATE_DEPTH_WRITE, R16G16B16A16_FLOAT,
				mRenderPrimitiveManager->GetHeapByName("BloomRtvHeap"), 4,
				mRenderPrimitiveManager->GetHeapByName("BloomSrvHeap"), 4,
				mRenderPrimitiveManager->GetHeapByName("BloomDsvHeap"), 4,
				false, 1, (((mClientWidth / 4) / 2) / 2)/2, (((mClientHeight / 4) / 2) / 2)/2);
			mRenderPrimitiveManager->InsertRenderTargetToLib("bloomDown4RenderTarget", bloomDown4RenderTarget);
		}
		//------------------------------------
		{
			auto bloomUpRenderTarget = mRHI->CreateRenderTarget(
				"bloomUpRenderTarget", TEXTURE2D, STATE_DEPTH_WRITE, R16G16B16A16_FLOAT,
				mRenderPrimitiveManager->GetHeapByName("BloomRtvHeap"), 5,
				mRenderPrimitiveManager->GetHeapByName("BloomSrvHeap"), 5,
				mRenderPrimitiveManager->GetHeapByName("BloomDsvHeap"), 5,
				false, 1, (((mClientWidth / 4) / 2) / 2), (((mClientHeight / 4) / 2) / 2));
			mRenderPrimitiveManager->InsertRenderTargetToLib("bloomUpRenderTarget", bloomUpRenderTarget);
		}
		//------------------------------------
		{
			auto bloomUp2RenderTarget = mRHI->CreateRenderTarget(
				"bloomUp2RenderTarget", TEXTURE2D, STATE_DEPTH_WRITE, R16G16B16A16_FLOAT,
				mRenderPrimitiveManager->GetHeapByName("BloomRtvHeap"), 6,
				mRenderPrimitiveManager->GetHeapByName("BloomSrvHeap"), 6,
				mRenderPrimitiveManager->GetHeapByName("BloomDsvHeap"), 6,
				false, 1, (mClientWidth / 4)/2, (mClientHeight / 4)/2 );
			mRenderPrimitiveManager->InsertRenderTargetToLib("bloomUp2RenderTarget", bloomUp2RenderTarget);
		}
		//------------------------------------
		{
			auto bloomMergeRenderTarget = mRHI->CreateRenderTarget(
				"bloomMergeRenderTarget", TEXTURE2D, STATE_DEPTH_WRITE, R16G16B16A16_FLOAT,
				mRenderPrimitiveManager->GetHeapByName("BloomRtvHeap"), 7,
				mRenderPrimitiveManager->GetHeapByName("BloomSrvHeap"), 7,
				mRenderPrimitiveManager->GetHeapByName("BloomDsvHeap"), 7,
				false, 1, (mClientWidth / 4) , (mClientHeight / 4) );
			mRenderPrimitiveManager->InsertRenderTargetToLib("bloomMergeRenderTarget", bloomMergeRenderTarget);
		}
		{
			auto ToneMapRenderTarget = mRHI->CreateRenderTarget(
				"ToneMapRenderTarget", TEXTURE2D, STATE_DEPTH_WRITE, R8G8B8A8_UNORM,
				mRenderPrimitiveManager->GetHeapByName("BloomRtvHeap"), 8,
				mRenderPrimitiveManager->GetHeapByName("BloomSrvHeap"), 8,
				mRenderPrimitiveManager->GetHeapByName("BloomDsvHeap"), 8,
				false, 1, mClientWidth, mClientHeight);
			mRenderPrimitiveManager->InsertRenderTargetToLib("ToneMapRenderTarget", ToneMapRenderTarget);
		}
	}
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
	auto a = Engine::Get()->GetAssetManager()->GetMapActorInfo()->Size();
	mRHI->CalculateFrameStats();
}

void Renderer::Draw()
{
	mRHI->DrawReset();

	DrawShadowPass();
	DrawBloomPass();
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
	mRHI->RenderDocBeginEvent("Shadow");
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
	mRHI->RenderDocEndEvent();
}

void Renderer::DrawBloomPass()
{
	mRHI->RenderDocBeginEvent("Bloom");
	{	
		mRHI->RenderDocBeginEvent("DrawHDRScene");
		// Draw HDR Scene ================================================
		auto HDRRendertarget = mRenderPrimitiveManager->GetRenderTargetByName("HDRRenderTarget");

		mRHI->SetScreenSetViewPort(
			HDRRendertarget->width,
			HDRRendertarget->height);
		mRHI->SetScissorRect(
			long(HDRRendertarget->width),
			long(HDRRendertarget->height));
		mRHI->ClearRenderTargetView(HDRRendertarget, mClearColor, 0);
		mRHI->ClearDepthStencilView(HDRRendertarget);
		mRHI->OMSetRenderTargets(HDRRendertarget);
		mRHI->SetPipelineState(mRenderPrimitiveManager->GetPipelineByName("HDRPipeline"));
		mRHI->SetDescriptorHeap(mRenderPrimitiveManager->GetHeapByName("mCbvHeap"));
		for (int ActorIndex = 0; ActorIndex < Engine::Get()->GetAssetManager()->GetMapActorInfo()->Size(); ActorIndex++)
		{
			mRHI->DrawMesh(ActorIndex, 0);
		}
		mRHI->RenderDocEndEvent();
	}
	//Draw Setup Scene================================================
	{
		mRHI->RenderDocBeginEvent("BloomSetup");
		auto HDRRendertarget = mRenderPrimitiveManager->GetRenderTargetByName("HDRRenderTarget");
		auto SetupRendertarget = mRenderPrimitiveManager->GetRenderTargetByName("bloomSetupRenderTarget");
		mRHI->SetScreenSetViewPort(
			SetupRendertarget->width,
			SetupRendertarget->height);
		mRHI->SetScissorRect(
			long(SetupRendertarget->width),
			long(SetupRendertarget->height));
		//mRHI->ClearRenderTarget(Rendertarget, "BloomRtvHeap");
		mRHI->ClearRenderTargetView(SetupRendertarget, mClearColor, 0);
		mRHI->ClearDepthStencilView(SetupRendertarget);
		mRHI->OMSetRenderTargets(SetupRendertarget);
		mRHI->SetPipelineState(mRenderPrimitiveManager->GetPipelineByName("bloomSetupPipeline"));
		mRHI->SetDescriptorHeap(mRenderPrimitiveManager->GetHeapByName("mCbvHeap"));
		auto cbvheap = mRenderPrimitiveManager->GetHeapByName("mCbvHeap");
		int4 screenSize;
		screenSize.x = int(mClientWidth);
		screenSize.y = int(mClientHeight);
		screenSize.z = -1;
		screenSize.w = -1;
		//ObjectConstants objConstant;
		//objConstant.RenderTargetSize = screenSize;
		//mRHI->CommitResourceToGPU(0, objConstant);
		//mRHI->CommitShaderParameter_ConstantBuffer(0, cbvheap);

 		mRHI->SetDescriptorHeap(mRenderPrimitiveManager->GetHeapByName("BloomSrvHeap"));
 		mRHI->CommitShaderParameter_Texture(1, HDRRendertarget);
		mRHI->CommitShaderParameter_Constant(5, 4, screenSize);

		mRHI->BuildTriangleAndDraw(Engine::Get()->GetAssetManager()->GetStaticMeshByName("triangle")->meshBuffer);
		mRHI->RenderDocEndEvent();
	}
	//Draw Down2 Scene================================================
	{
		mRHI->RenderDocBeginEvent("BloomDown");

		auto SetupRendertarget = mRenderPrimitiveManager->GetRenderTargetByName("bloomSetupRenderTarget");
		auto Down2Rendertarget = mRenderPrimitiveManager->GetRenderTargetByName("bloomDown2RenderTarget");
		mRHI->SetScreenSetViewPort(
			Down2Rendertarget->width,
			Down2Rendertarget->height);
		mRHI->SetScissorRect(
			long(Down2Rendertarget->width),
			long(Down2Rendertarget->height));
		//mRHI->ClearRenderTarget(Rendertarget, "BloomRtvHeap");
		mRHI->ClearRenderTargetView(Down2Rendertarget, mClearColor, 0);
		mRHI->ClearDepthStencilView(Down2Rendertarget);
		mRHI->OMSetRenderTargets(Down2Rendertarget);
		mRHI->SetPipelineState(mRenderPrimitiveManager->GetPipelineByName("bloomDownPipeline"));
		mRHI->SetDescriptorHeap(mRenderPrimitiveManager->GetHeapByName("mCbvHeap"));
		auto cbvheap = mRenderPrimitiveManager->GetHeapByName("mCbvHeap");
		int4 screenSize;
		screenSize.x = int(mClientWidth /4);
		screenSize.y = int(mClientHeight /4);
		screenSize.z = -1;
		screenSize.w = -1;
		//ObjectConstants objConstant;
		//objConstant.RenderTargetSize = screenSize;
		//mRHI->CommitResourceToGPU(1, objConstant);
		//mRHI->CommitShaderParameter_ConstantBuffer(1, cbvheap);

		mRHI->SetDescriptorHeap(mRenderPrimitiveManager->GetHeapByName("BloomSrvHeap"));
		mRHI->CommitShaderParameter_Texture(1, SetupRendertarget);
		mRHI->CommitShaderParameter_Constant(5, 4, screenSize);
		mRHI->BuildTriangleAndDraw(Engine::Get()->GetAssetManager()->GetStaticMeshByName("triangle")->meshBuffer);
		
		mRHI->RenderDocEndEvent();
	}
	//Draw Down3 Scene================================================
	{
		mRHI->RenderDocBeginEvent("BloomDown");

		auto Down2Rendertarget = mRenderPrimitiveManager->GetRenderTargetByName("bloomDown2RenderTarget");
		auto Down3Rendertarget = mRenderPrimitiveManager->GetRenderTargetByName("bloomDown3RenderTarget");
		mRHI->SetScreenSetViewPort(
			Down3Rendertarget->width,
			Down3Rendertarget->height);
		mRHI->SetScissorRect(
			long(Down3Rendertarget->width),
			long(Down3Rendertarget->height));
		//mRHI->ClearRenderTarget(Rendertarget, "BloomRtvHeap");
		mRHI->ClearRenderTargetView(Down3Rendertarget, mClearColor, 0);
		mRHI->ClearDepthStencilView(Down3Rendertarget);
		mRHI->OMSetRenderTargets(Down3Rendertarget);
		mRHI->SetPipelineState(mRenderPrimitiveManager->GetPipelineByName("bloomDownPipeline"));
		mRHI->SetDescriptorHeap(mRenderPrimitiveManager->GetHeapByName("mCbvHeap"));
		auto cbvheap = mRenderPrimitiveManager->GetHeapByName("mCbvHeap");
		int4 screenSize;
		screenSize.x = int((mClientWidth /4) / 2);
		screenSize.y = int((mClientHeight /4) / 2);
		screenSize.z = -1;
		screenSize.w = -1;
		//ObjectConstants objConstant;
		//objConstant.RenderTargetSize = screenSize;
		//mRHI->CommitResourceToGPU(2, objConstant);
		//mRHI->CommitShaderParameter_ConstantBuffer(2, cbvheap);

		mRHI->SetDescriptorHeap(mRenderPrimitiveManager->GetHeapByName("BloomSrvHeap"));
		mRHI->CommitShaderParameter_Texture(1, Down2Rendertarget);
		mRHI->CommitShaderParameter_Constant(5, 4, screenSize);
		mRHI->BuildTriangleAndDraw(Engine::Get()->GetAssetManager()->GetStaticMeshByName("triangle")->meshBuffer);
		
		mRHI->RenderDocEndEvent();
	}
	//Draw Down4 Scene================================================
	{
		mRHI->RenderDocBeginEvent("BloomDown");

		auto Down3Rendertarget = mRenderPrimitiveManager->GetRenderTargetByName("bloomDown3RenderTarget");
		auto Down4Rendertarget = mRenderPrimitiveManager->GetRenderTargetByName("bloomDown4RenderTarget");
		mRHI->SetScreenSetViewPort(
			Down4Rendertarget->width,
			Down4Rendertarget->height);
		mRHI->SetScissorRect(
			long(Down4Rendertarget->width),
			long(Down4Rendertarget->height));
		//mRHI->ClearRenderTarget(Rendertarget, "BloomRtvHeap");
		mRHI->ClearRenderTargetView(Down4Rendertarget, mClearColor, 0);
		mRHI->ClearDepthStencilView(Down4Rendertarget);
		mRHI->OMSetRenderTargets(Down4Rendertarget);
		mRHI->SetPipelineState(mRenderPrimitiveManager->GetPipelineByName("bloomDownPipeline"));
		mRHI->SetDescriptorHeap(mRenderPrimitiveManager->GetHeapByName("mCbvHeap"));
		auto cbvheap = mRenderPrimitiveManager->GetHeapByName("mCbvHeap");
		int4 screenSize;
		screenSize.x =int (((mClientWidth / 4) / 2) / 2);
		screenSize.y =int (((mClientHeight / 4) / 2) / 2);
		screenSize.z = -1;
		screenSize.w = -1;
		//ObjectConstants objConstant;
		//objConstant.RenderTargetSize = screenSize;
		//mRHI->CommitResourceToGPU(3, objConstant);
		//mRHI->CommitShaderParameter_ConstantBuffer(3, cbvheap);

		mRHI->SetDescriptorHeap(mRenderPrimitiveManager->GetHeapByName("BloomSrvHeap"));
		mRHI->CommitShaderParameter_Texture(1, Down3Rendertarget);
		mRHI->CommitShaderParameter_Constant(5, 4, screenSize);
		mRHI->BuildTriangleAndDraw(Engine::Get()->GetAssetManager()->GetStaticMeshByName("triangle")->meshBuffer);
		
		mRHI->RenderDocEndEvent();
	}
	//Draw Up Scene================================================
	{
		mRHI->RenderDocBeginEvent("BloomUp");

		auto Down3Rendertarget = mRenderPrimitiveManager->GetRenderTargetByName("bloomDown3RenderTarget");
		auto Down4Rendertarget = mRenderPrimitiveManager->GetRenderTargetByName("bloomDown4RenderTarget");
		
		auto UpRendertarget = mRenderPrimitiveManager->GetRenderTargetByName("bloomUpRenderTarget");
		mRHI->SetScreenSetViewPort(
			UpRendertarget->width,
			UpRendertarget->height);
		mRHI->SetScissorRect(
			long(UpRendertarget->width),
			long(UpRendertarget->height));
		//mRHI->ClearRenderTarget(Rendertarget, "BloomRtvHeap");
		mRHI->ClearRenderTargetView(UpRendertarget, mClearColor, 0);
		mRHI->ClearDepthStencilView(UpRendertarget);
		mRHI->OMSetRenderTargets(UpRendertarget);
		mRHI->SetPipelineState(mRenderPrimitiveManager->GetPipelineByName("bloomUpPipeline"));
		mRHI->SetDescriptorHeap(mRenderPrimitiveManager->GetHeapByName("mCbvHeap"));
		auto cbvheap = mRenderPrimitiveManager->GetHeapByName("mCbvHeap");
		int4 screenSize;
		screenSize.x = int(((mClientWidth / 4) / 2)/2);
		screenSize.y = int(((mClientHeight / 4) / 2)/2);
		screenSize.z = 10;
		screenSize.w = 10;
		//ObjectConstants objConstant;
		//objConstant.RenderTargetSize = screenSize;
		//mRHI->CommitResourceToGPU(4, objConstant);
		//mRHI->CommitShaderParameter_ConstantBuffer(4, cbvheap);

		mRHI->SetDescriptorHeap(mRenderPrimitiveManager->GetHeapByName("BloomSrvHeap"));
		mRHI->CommitShaderParameter_Texture(1, Down4Rendertarget);
		mRHI->CommitShaderParameter_Texture(3, Down3Rendertarget);
		mRHI->CommitShaderParameter_Constant(5, 4, screenSize);
		mRHI->BuildTriangleAndDraw(Engine::Get()->GetAssetManager()->GetStaticMeshByName("triangle")->meshBuffer);
		
		mRHI->RenderDocEndEvent();
	}
	//Draw Up2 Scene================================================
	{
		mRHI->RenderDocBeginEvent("BloomUp");

		auto Down2Rendertarget = mRenderPrimitiveManager->GetRenderTargetByName("bloomDown2RenderTarget");
		auto UpRendertarget = mRenderPrimitiveManager->GetRenderTargetByName("bloomUpRenderTarget");

		auto Up2Rendertarget = mRenderPrimitiveManager->GetRenderTargetByName("bloomUp2RenderTarget");
		mRHI->SetScreenSetViewPort(
			Up2Rendertarget->width,
			Up2Rendertarget->height);
		mRHI->SetScissorRect(
			long(Up2Rendertarget->width),
			long(Up2Rendertarget->height));
		//mRHI->ClearRenderTarget(Rendertarget, "BloomRtvHeap");
		mRHI->ClearRenderTargetView(Up2Rendertarget, mClearColor, 0);
		mRHI->ClearDepthStencilView(Up2Rendertarget);
		mRHI->OMSetRenderTargets(Up2Rendertarget);
		mRHI->SetPipelineState(mRenderPrimitiveManager->GetPipelineByName("bloomUpPipeline"));
		mRHI->SetDescriptorHeap(mRenderPrimitiveManager->GetHeapByName("mCbvHeap"));
		auto cbvheap = mRenderPrimitiveManager->GetHeapByName("mCbvHeap");
		int4 screenSize;
		screenSize.x = int((mClientWidth / 4)/2);
		screenSize.y = int((mClientHeight / 4)/2);
		screenSize.z = 10;
		screenSize.w = 10;
		//ObjectConstants objConstant;
		//objConstant.RenderTargetSize = screenSize;
		//mRHI->CommitResourceToGPU(5, objConstant);
		//mRHI->CommitShaderParameter_ConstantBuffer(5, cbvheap);

		mRHI->SetDescriptorHeap(mRenderPrimitiveManager->GetHeapByName("BloomSrvHeap"));
		mRHI->CommitShaderParameter_Texture(1, UpRendertarget);
		mRHI->CommitShaderParameter_Texture(3, Down2Rendertarget);
		mRHI->CommitShaderParameter_Constant(5, 4, screenSize);
		mRHI->BuildTriangleAndDraw(Engine::Get()->GetAssetManager()->GetStaticMeshByName("triangle")->meshBuffer);
		
		mRHI->RenderDocEndEvent();
	}
	//Draw Merge Scene================================================
	{
		mRHI->RenderDocBeginEvent("BloomMerge");

		auto SetupRendertarget = mRenderPrimitiveManager->GetRenderTargetByName("bloomSetupRenderTarget");
		auto Up2Rendertarget = mRenderPrimitiveManager->GetRenderTargetByName("bloomUp2RenderTarget");

		auto bloomMergeRendertarget = mRenderPrimitiveManager->GetRenderTargetByName("bloomMergeRenderTarget");
		mRHI->SetScreenSetViewPort(
			bloomMergeRendertarget->width,
			bloomMergeRendertarget->height);
		mRHI->SetScissorRect(
			long(bloomMergeRendertarget->width),
			long(bloomMergeRendertarget->height));
		//mRHI->ClearRenderTarget(Rendertarget, "BloomRtvHeap");
		mRHI->ClearRenderTargetView(bloomMergeRendertarget, mClearColor, 0);
		mRHI->ClearDepthStencilView(bloomMergeRendertarget);
		mRHI->OMSetRenderTargets(bloomMergeRendertarget);
		mRHI->SetPipelineState(mRenderPrimitiveManager->GetPipelineByName("bloomMergePipeline"));
		mRHI->SetDescriptorHeap(mRenderPrimitiveManager->GetHeapByName("mCbvHeap"));
		auto cbvheap = mRenderPrimitiveManager->GetHeapByName("mCbvHeap");
		int4 screenSize;
		screenSize.x = int(mClientWidth / 4); 
		screenSize.y = int(mClientHeight / 4);
		screenSize.z = 10;
		screenSize.w = 10;
		//ObjectConstants objConstant;
		//objConstant.RenderTargetSize = screenSize;
		//mRHI->CommitResourceToGPU(6, objConstant);
		//mRHI->CommitShaderParameter_ConstantBuffer(6, cbvheap);

		mRHI->SetDescriptorHeap(mRenderPrimitiveManager->GetHeapByName("BloomSrvHeap"));
		mRHI->CommitShaderParameter_Texture(1, Up2Rendertarget);
		mRHI->CommitShaderParameter_Texture(3, SetupRendertarget);
		mRHI->CommitShaderParameter_Constant(5, 4, screenSize);
		mRHI->BuildTriangleAndDraw(Engine::Get()->GetAssetManager()->GetStaticMeshByName("triangle")->meshBuffer);
		
		mRHI->RenderDocEndEvent();
	}
	mRHI->RenderDocEndEvent();
}

void Renderer::DrawScenePass()
{
	//Draw Merge Scene================================================
	/*
	{
		auto HDRRendertarget = mRenderPrimitiveManager->GetRenderTargetByName("HDRRenderTarget");
		auto MergeRenderTarget = mRenderPrimitiveManager->GetRenderTargetByName("bloomMergeRenderTarget");

		auto ToneMapRenderTarget = mRenderPrimitiveManager->GetRenderTargetByName("ToneMapRenderTarget");
		mRHI->SetScreenSetViewPort(
			ToneMapRenderTarget->width,
			ToneMapRenderTarget->height);
		mRHI->SetScissorRect(
			long(ToneMapRenderTarget->width),
			long(ToneMapRenderTarget->height));
		//mRHI->ClearRenderTarget(Rendertarget, "BloomRtvHeap");
		mRHI->ClearRenderTargetView(ToneMapRenderTarget, mClearColor, 0);
		mRHI->ClearDepthStencilView(ToneMapRenderTarget);
		mRHI->OMSetRenderTargets(ToneMapRenderTarget);
		mRHI->SetPipelineState(mRenderPrimitiveManager->GetPipelineByName("ToneMapPipeline"));
		mRHI->SetDescriptorHeap(mRenderPrimitiveManager->GetHeapByName("mCbvHeap"));
		auto cbvheap = mRenderPrimitiveManager->GetHeapByName("mCbvHeap");
		int4 screenSize;
		screenSize.x = 1920 ;
		screenSize.y = 1080 ;
		screenSize.z = 10;
		screenSize.w = 10;
		ObjectConstants objConstant;
		objConstant.RenderTargetSize = screenSize;
		mRHI->CommitResourceToGPU(3, objConstant);
		mRHI->CommitShaderParameter_ConstantBuffer(3, cbvheap);

		mRHI->SetDescriptorHeap(mRenderPrimitiveManager->GetHeapByName("BloomSrvHeap"));
		mRHI->CommitShaderParameter_Table(1, HDRRendertarget);
		mRHI->CommitShaderParameter_Table(3, MergeRenderTarget);
		mRHI->CommitShaderParameter_Constant(5, 4, screenSize);
		mRHI->BuildTriangleAndDraw(Engine::Get()->GetAssetManager()->GetStaticMeshByName("triangle")->meshBuffer);
	}
	*/

	
	{
		mRHI->RenderDocBeginEvent("ToneMap");

		auto HDRRendertarget = mRenderPrimitiveManager->GetRenderTargetByName("HDRRenderTarget");
		auto bloomMergeRendertarget = mRenderPrimitiveManager->GetRenderTargetByName("bloomMergeRenderTarget");

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
		mRHI->SetPipelineState(mRenderPrimitiveManager->GetPipelineByName("ToneMapPipeline"));

		mRHI->SetDescriptorHeap(mRenderPrimitiveManager->GetHeapByName("mCbvHeap"));
		auto cbvheap = mRenderPrimitiveManager->GetHeapByName("mCbvHeap");
		int4 screenSize;
		screenSize.x = int(mClientWidth);
		screenSize.y = int(mClientHeight);
		screenSize.z = 10;
		screenSize.w = 10;
		//ObjectConstants objConstant;
		//objConstant.RenderTargetSize = screenSize;
		//mRHI->CommitResourceToGPU(7, objConstant);
		//mRHI->CommitShaderParameter_ConstantBuffer(7, cbvheap);

		mRHI->SetDescriptorHeap(mRenderPrimitiveManager->GetHeapByName("BloomSrvHeap"));
		mRHI->CommitShaderParameter_Texture(1, HDRRendertarget);
		mRHI->CommitShaderParameter_Texture(3, bloomMergeRendertarget);
		mRHI->CommitShaderParameter_Constant(5, 4, screenSize);
		mRHI->BuildTriangleAndDraw(Engine::Get()->GetAssetManager()->GetStaticMeshByName("triangle")->meshBuffer);

		mRHI->RenderDocEndEvent();
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

