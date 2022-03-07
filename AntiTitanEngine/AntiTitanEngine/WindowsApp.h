#pragma once
#include "App.h"
#include "StaticMesh.h"
#include "Camera.h"
#include "vector"
#include "ActorsInfo.h"
#include "map"
#include "Asset.h"


//原来在BoxApp里面
struct ObjectConstants
{
	XMFLOAT4X4 WorldViewProj = MathHelper::Identity4x4();
	glm::mat4 WorldViewProjMat4 = glm::identity<glm::mat4>();
};

class WindowsApp : public App {

public:

	WindowsApp(HINSTANCE hInstance);
	WindowsApp(const WindowsApp& rhs) = delete;
	WindowsApp& operator=(const WindowsApp& rhs) = delete;
	~WindowsApp();

public:
	static WindowsApp* mWindowsApp;
	HINSTANCE AppInst()const;
	HWND      MainWnd()const;

	int Run() ;

	static WindowsApp* GetApp();
	virtual bool Initialize();
	virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	void OnResize()override;
	void Update(const GameTimer& gt) override;
	void Draw(const GameTimer& gt) override;

	// Convenience overrides for handling mouse input.
	virtual void OnMouseDown(WPARAM btnState, int x, int y);
	virtual void OnMouseUp(WPARAM btnState, int x, int y);
	virtual void OnMouseMove(WPARAM btnState, int x, int y);

	void OnKeyboardInput(const GameTimer& gt);

	void BuildDescriptorHeaps();
	void SetDescriptorHeaps();//往Heap里面灌数据
	//void BuildConstantBuffers();
	void BuildRootSignature();
	void BuildShadersAndInputLayout();
	//void BuildBoxGeometry();
	void BuildPSO();

protected:

	HINSTANCE mhAppInst = nullptr; // application instance handle
	HWND      mhMainWnd = nullptr; // main window handle
	bool      mAppPaused = false;  // is the application paused?
	bool      mMinimized = false;  // is the application minimized?
	bool      mMaximized = false;  // is the application maximized?
	bool      mResizing = false;   // are the resize bars being dragged?
	bool      mFullscreenState = false;// fullscreen enabled


private:
	ComPtr<ID3D12RootSignature> mRootSignature = nullptr;
	ComPtr<ID3D12DescriptorHeap> mCbvHeap = nullptr;

	//std::unique_ptr<MeshGeometry> mBoxGeo = nullptr;
	//std::vector<std::unique_ptr<MeshGeometry>> Geos;
	//ActorsInfo MapActor;
	//std::map<int, std::string> MapofGeosMesh;

	ComPtr<ID3DBlob> mvsByteCode = nullptr;
	ComPtr<ID3DBlob> mpsByteCode = nullptr;

	std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;

	std::unique_ptr<UploadBuffer<ObjectConstants>> mObjectCB = nullptr;


	ComPtr<ID3D12PipelineState> mPSO = nullptr;

protected:
	bool InitMainWindow();
	void CreateSwapChain();

	void CalculateFrameStats();//窗口上面变动的fps和mspf


private:
POINT mLastMousePos;
//Camera mCamera;

Asset mAsset;

};