//***************************************************************************************
// color.hlsl by Frank Luna (C) 2015 All Rights Reserved.
//
// Transforms and colors geometry.
//***************************************************************************************

Texture2D    gDiffuseMap : register(t0);
Texture2D    gNormalMap : register(t1);

SamplerState gsamPointWrap        : register(s0);
SamplerState gsamPointClamp       : register(s1);
SamplerState gsamLinearWrap       : register(s2);
SamplerState gsamLinearClamp      : register(s3);
SamplerState gsamAnisotropicWrap  : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);

float4 CameraLoc:register(b1);

cbuffer cbPerObject : register(b0)
{
	float4x4 gWorldViewProj;
	float4x4 gWorldViewProjMat4;
	float4x4 Rotator;
	float4x4 gLightWorldViewProj;
	//int CanMove;
	float    Time;
};

struct VertexIn
{
	float3 PosL      : POSITION;
	float4 Color     : COLOR;
	float4 Normal    : NORMAL;
	float2 TexCoord  : TEXCOORD;
};

struct VertexOut
{
	float4 PosH  : SV_POSITION;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	float QuickSpeed = 15;
	float MidSpeed = 10;
	float SlowSpeed = 5;

	float3 PosW;
	PosW.x = vin.PosL.x + sin(Time * 1.5) * 75;
	PosW.y = vin.PosL.y + sin(Time * MidSpeed);
	PosW.z = vin.PosL.z + sin(Time * SlowSpeed);

	//Transform to homogeneous clip space.
	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);

	vout.PosH = mul(float4(PosW, 1.0f), gWorldViewProj);

	return vout;
}

float4 PS(VertexOut pin) : SV_Target
{

	//return pin.Color;
	return 0;
	//return diffuseAlbedo+ NormalMap;
}

