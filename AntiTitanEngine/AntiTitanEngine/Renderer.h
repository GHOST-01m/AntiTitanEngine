#pragma once
#include "RHI.h"
#include "DXRHI.h"
#include "RHIResource.h"
#include "MyStruct.h"
class Renderer
{
public:
	Renderer();
	~Renderer();

private:
	std::shared_ptr<RHI> mRHI;

public:
	bool Init();
	void Update();
	void Draw();
	void Destroy();
	
public:
	std::shared_ptr<RHI> GetRHI();
	std::shared_ptr<Camera> GetCamera();

	void CalculateFrameStats();
	void Set4xMsaaState(bool value);

public:
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
	bool m4xMsaaState = false;
};