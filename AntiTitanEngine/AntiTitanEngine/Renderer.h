#pragma once
#include "RHI.h"
#include "DXRHI.h"
#include "RenderPrimitiveManager.h"
#include "MyStruct.h"
#include "PipelineState_DESC.h"

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
		void LoadMeshAndSetBuffer();

	void Update();
		void UpdateMesh(int index, ObjectConstants& objConstants);
		void UpdateShadow(int index, ObjectConstants& objConstants);

	void Draw();
		void DrawShadowPass();
		void DrawScenePass();

	void Destroy();
	
public:
	bool m4xMsaaState = false;

private:
	Color mClearColor = { 0,0,0,0 };

	int m4xMsaaQuality = 0;
	float mClientWidth = 1920;
	float mClientHeight = 1080;

	std::string MapActorLoadPath = "MapActorInfo/MapActorInfo.bat";

	std::wstring TextureLoadPath = L"Texture/Stone_Texture.dds";
	std::wstring NormalLoadPath = L"Texture/Stone_Normal.dds";
	//std::wstring TextureLoadPath = L"Texture/jacket_diff.dds";
	//std::wstring NormalLoadPath = L"Texture/jacket_norm.dds";
	//std::wstring TextureLoadPath = L"Texture/Hex_Texture.dds";
	//std::wstring NormalLoadPath = L"Texture/Hex_Normal.dds";

	std::wstring ShaderPath = L"Shaders\\color.hlsl";
};