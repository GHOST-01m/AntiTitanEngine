#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers.
#endif

#include <windows.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include "d3dx12.h"

#include <string>
#include <wrl.h>
#include <shellapi.h>

#include "d3dUtil.h"
#include "GameTimer.h"

#include <crtdbg.h>

#include <windows.h>
#include <wrl.h>
#include <dxgi1_4.h>
#include <d3d12.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>
#include <string>
#include <memory>
#include <algorithm>
#include <vector>
#include <array>
#include <unordered_map>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <cassert>
#include "d3dx12.h"
#include "Common/DDSTextureLoader.h"
#include "MathHelper.h"

#include "d3d12.h"

#include <Windows.h>
#include <DirectXMath.h>
#include <cstdint>
#include <WindowsX.h>

#include "d3dApp.h"
#include "d3dUtil.h"
#include "d3dx12.h"
#include "GameTimer.h"
#include "UploadBuffer.h"
#include "BoxApp.h"
#include "StaticMesh.h"
#include "Camera.h"
#include "MeshGeometry.h"
#include "App.h"
#include "WindowsApp.h"
#include "ActorsInfo.h"
#include "Asset.h"
#include "Renderer.h"
#include "Engine.h"
#include "Window.h"
#include "Win32Window.h"
#include "TopAPP.h"
#include "WindowsTopApp.h"



#include "iostream"
#include <string.h>
#include "vector"
#include "set"
#include "map"
#include "glm/glm/gtx/quaternion.hpp"
#include "glm/glm/gtc/type_ptr.hpp"