#pragma once
#include "RHI.h"
#include "DXRHI.h"
#include "RHIResourceManager.h"
#include "MyStruct.h"
#include "RenderResourceManager.h"
class Renderer
{
public:
	Renderer();
	~Renderer();

private:
	std::shared_ptr<RHI> mRHI;

public:
//û�м��Init֮��ķ�����ʹ�õ�Resource�ǲ��Ǵ�RHIResourceManager�л�ȡ�ģ���
//û�м��Init֮��ķ�����ʹ�õ�Resource�ǲ��Ǵ�RHIResourceManager�л�ȡ�ģ���
//û�м��Init֮��ķ�����ʹ�õ�Resource�ǲ��Ǵ�RHIResourceManager�л�ȡ�ģ���
	bool Init();
	void Update();
	void Draw();
	void Destroy();
	
public:
	std::shared_ptr<RHI> GetRHI();
	std::shared_ptr<Camera> GetCamera();

	void CalculateFrameStats();
	void Set4xMsaaState(bool value);
	bool m4xMsaaState = false;
private:
	std::shared_ptr<RenderResourceManager> mRenderResourceManager;

	ObjectConstants objConstants;
	ScreenViewport mViewport{
		0,
		0,
		1920,
		1080,
		0.0f,
		1.0f
	};
	ScissorRect mScissorRect{ 0, 0, 1920, 1080 };//�����ĳ�ʼ����û������
	Color mClearColor = { 0,0,0,0 };

	int m4xMsaaQuality = 0;
	int mClientWidth = 1920;
	int mClientHeight = 1080;
	static const int SwapChainBufferCount = 2;

	std::string MapActorLoadPath = "MapActorInfo/MapActorInfo.bat";
	std::string MapLightLoadPath = "MapLightInfo/MapLightInfo.bat";
	//std::string MapLightLoadPath = "E:/DX12Homework/AntiTitanEngine/AntiTitanEngine/AntiTitanEngine/AntiTitanEngine/MapLightInfo/MapLightInfo.bat";

	std::wstring TextureLoadPath = L"Texture/Stone_Texture.dds";
	std::wstring NormalLoadPath = L"Texture/Stone_Normal.dds";
	std::wstring ShaderPath = L"Shaders\\color.hlsl";
};