#pragma once
#include "stdafx.h"

struct Color
{
	float r;
	float g;
	float b;
	float a;
};

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

struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT4 Color;
	XMFLOAT4 Normal;
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
	XMFLOAT4X4 WorldViewProj = MathHelper::Identity4x4();
	glm::mat4 WorldViewProjMat4 = glm::identity<glm::mat4>();
	XMFLOAT4X4 rotation;
	//glm::mat4 rotation;
	//int CanMove;

	float mTime;
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