#pragma once
#include "DirectXMath.h"
#include "glm/glm/gtx/quaternion.hpp"
#include "glm/glm/gtc/type_ptr.hpp"

struct Float3 
{
	float x;
	float y;
	float z;
};

struct int4
{
	int x;
	int y;
	int z;
	int w;
};

struct Color
{
	float r;
	float g;
	float b;
	float a;
};
//struct Material
//{
//	Color BaseColor;
//	float Metallic;
//	float DiffuseAlbedo;
//	Float3 FresnelR0;
//	float Roughness;
//};

struct ScissorRect
{
	long    left;
	long    top;
	long    right;
	long     bottom;
};

struct ScreenViewport 
{
	float TopLeftX;
	float TopLeftY;
	float Width;
	float Height;
	float MinDepth;
	float MaxDepth;
};

struct Location
{
	float x;
	float y;
	float z;

};

struct Rotator
{
	/** Rotation around the right axis (around Y axis), Looking up and down (0=Straight Ahead, +Up, -Down) */
	float Pitch;

	/** Rotation around the up axis (around Z axis), Running in circles 0=East, +North, -South. */
	float Yaw;

	/** Rotation around the forward axis (around X axis), Tilting your head, 0=Straight, +Clockwise, -CCW. */
	float Roll;

	float w;
};

struct Scale
{
	float x;
	float y;
	float z;
};

struct Transform
{
	Rotator rotation;
	Location translation;
	Scale scale3D;
};

struct Quat
{
	float X;
	float Y;
	float Z;
	float W;
};

//===============================================
struct FVector
{
	float X;
	float Y;
	float Z;

	operator DirectX::XMFLOAT3() const {
		return { X,Y,Z };
	}
};


struct FVector2D
{
	float X;
	float Y;
};

struct FVector4
{
	float X;
	float Y;
	float Z;
	float W;

	operator DirectX::XMFLOAT4() const {
		return { X,Y,Z,W };
	}
};
//===========================================
struct Vertex
{
	DirectX::XMFLOAT3 Pos;
	DirectX::XMFLOAT4 Color;
	DirectX::XMFLOAT4 Normal;
	DirectX::XMFLOAT4 Tangent;
	DirectX::XMFLOAT4 Bitangent;
	FVector2D TexCoord;

	Vertex SetValueFromFVector(FVector& a) {
		Pos.x = a.X;
		Pos.y = a.Y;
		Pos.z = a.Z;
		Color = { 1.0f, 0.8f, 0.0f, 1.0f };
	}
};

struct ObjectConstants
{
	DirectX::XMFLOAT4X4 WorldViewProj = MathHelper::Identity4x4();
	glm::mat4  WorldViewProjMat4 = glm::identity<glm::mat4>();
	DirectX::XMFLOAT4X4 rotation;
	DirectX::XMFLOAT4X4 gWorld = MathHelper::Identity4x4();            //转置（世界矩阵）
	DirectX::XMFLOAT4X4 gLightVP = MathHelper::Identity4x4();          //转置            (光的V矩阵 * 光的P矩阵)
	DirectX::XMFLOAT4X4 gShadowTransform = MathHelper::Identity4x4();  //转置            (光的V矩阵 * 光的P矩阵 * T矩阵）
	DirectX::XMFLOAT4X4 gLightMVP= MathHelper::Identity4x4();          //转置（世界矩阵  * 光的V矩阵 * 光的P矩阵)
	DirectX::XMFLOAT4X4 gLightMVPT = MathHelper::Identity4x4();        //转置（世界矩阵  * 光的V矩阵 * 光的P矩阵 * T矩阵）
	DirectX::XMFLOAT4 LightDirection;
	DirectX::XMFLOAT4 LightStrength;
	DirectX::XMFLOAT4 gDiffuseAlbedo;
	DirectX::XMFLOAT3 gFresnelR0;
	float  gRoughness;
	Location LightLocation;
	float LightLocationW;
	DirectX::XMFLOAT3 CameraLocation;
	float CameraLocationW;
	int4 RenderTargetSize;
	//glm::mat4 rotation;
	//int CanMove;
	//float mTime;
};

struct Texture
{
public:
	// Unique material name for lookup.
	std::string Name;

	std::wstring Filename;

	Microsoft::WRL::ComPtr<ID3D12Resource> Resource = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> UploadHeap = nullptr;
};

struct LightInfo {
public:
	std::string ActorName;
	Transform mTransform;
	Float3 Rotation;
	float Intensity;
	FVector Direction;
	Color LinearColor;
};

typedef
enum HeapTyme
{
	CBVSRVUAV = 0,
	SAMPLE = 1,
	RTV = 2,
	DSV = 3,
	NUMTYPE = 4
}HeapTyme;

typedef
enum ResourceType
{
	UNKNOWN     = 0,
	BUFFER      = 1,
	TEXTURE1D   = 2,
	TEXTURE2D   = 3,
	TEXTURE3D   = 4
}ResourceType;

typedef
enum ResourceStateType
{
	STATE_COMMON          = 0,
	STATE_DEPTH_WRITE     = 1,
	STATE_RENDER_TARGET   = 2,
	STATE_PRESENT         = 3,
	STATE_GENERIC_READ    = 4

}ResourceStateType;

typedef
enum Pipeline_FillMode
{
	FILL_MODE_WIREFRAME = 2,
	FILL_MODE_SOLID = 3

}Pipeline_FillMode;

typedef
enum Pipeline_CullMode
{
	CULL_MODE_NONE = 1,
	CULL_MODE_FRONT = 2,
	CULL_MODE_BACK = 3
}Pipeline_CullMode;

typedef
struct Pipeline_Rasterizer_DESC
{
	//D3D12_FILL_MODE FillMode;
	Pipeline_FillMode FillMode;
	//D3D12_CULL_MODE CullMode;
	Pipeline_CullMode CullMode;
	bool FrontCounterClockwise;
	int DepthBias;
	float DepthBiasClamp;
	float SlopeScaledDepthBias;
	bool DepthClipEnable;
	bool MultisampleEnable;
	bool AntialiasedLineEnable;
	int ForcedSampleCount;
	int ConservativeRasterizationMode;
} Pipeline_Rasterizer_DESC;


typedef
enum Pipeline_Blend_DESC
{

}Pipeline_Blend_DESC;


typedef
enum RenderTargetFormats
{
	RenderTargetFormat_R8G8B8A8_UNORM = 0,
	RenderTargetFormat_UNKNOWN = 1,
	RenderTargetFormat_R16G16B16A16_FLOAT= 2,
	RenderTargetFormat_R32G32B32A32_FLOAT= 3

}RenderTargetFormats;

typedef
enum DescriptorHeapFlags
{
	NONE = 0,
	SHADER_VISIBLE = 1
}DescriptorHeapFlags;


typedef
enum ResourceFormat
{
	R8G8B8A8_UNORM = 0,
	R16G16B16A16_FLOAT = 1,
	D24_UNORM_S8_UINT=2
}ResourceFormat;
