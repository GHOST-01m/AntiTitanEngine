#pragma once
#include "RHI.h"
#include "DXRHI.h"
#include "RenderPrimitiveManager.h"
#include "MyStruct.h"

class Renderer
{
public:
	Renderer();
	~Renderer();

private:
	std::shared_ptr<RHI> mRHI;
	std::shared_ptr<Camera> mCamera;
	std::shared_ptr<RenderPrimitiveManager> mRenderPrimitiveManager;

public:
	std::shared_ptr<RHI> GetRHI();
	std::shared_ptr<Camera> GetCamera();
	std::shared_ptr<RenderPrimitiveManager> GetRenderPrimitiveManager();

public:
	bool Init();
		void CreateHeap();
		void CreateShader();
		void CreatePipeline();
		void CreateRenderTarget();
	void Update();
		void UpdateMesh(int index, ObjectConstants& objConstants);
		void UpdateShadow(int index, ObjectConstants& objConstants);
	void Draw();
		void ShadowPass();
		void DrawScenePass();
	void Destroy();
	
public:
	bool m4xMsaaState = false;

private:
	ObjectConstants objConstants;
	ScreenViewport mViewport{
		0,
		0,
		1920,
		1080,
		0.0f,
		1.0f
	};
	ScissorRect mScissorRect{ 0, 0, 1920, 1080 };//这俩的初始化还没有做！
	Color mClearColor = { 0,0,0,0 };

	int m4xMsaaQuality = 0;
	float mClientWidth = 1920;
	float mClientHeight = 1080;
	static const int SwapChainBufferCount = 2;

	std::string MapActorLoadPath = "MapActorInfo/MapActorInfo.bat";
	std::string MapLightLoadPath = "MapLightInfo/MapLightInfo.bat";
	//std::string MapLightLoadPath = "E:/DX12Homework/AntiTitanEngine/AntiTitanEngine/AntiTitanEngine/AntiTitanEngine/MapLightInfo/MapLightInfo.bat";

	std::wstring TextureLoadPath = L"Texture/Stone_Texture.dds";
	std::wstring NormalLoadPath = L"Texture/Stone_Normal.dds";
	std::wstring ShaderPath = L"Shaders\\color.hlsl";
};