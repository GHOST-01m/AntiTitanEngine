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
	STATE_DEPTH_WRITE = 1,
	STATE_RENDER_TARGET = 2,
	STATE_PRESENT = 3,
	STATE_GENERIC_READ = 4
}ResourceStateType;
