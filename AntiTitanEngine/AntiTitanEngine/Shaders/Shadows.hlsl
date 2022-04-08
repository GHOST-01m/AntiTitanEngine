//***************************************************************************************
// color.hlsl by Frank Luna (C) 2015 All Rights Reserved.
//
// Transforms and colors geometry.
//***************************************************************************************

Texture2D    gDiffuseMap : register(t0);
Texture2D    gNormalMap : register(t1);
Texture2D    gShadowMap : register(t2);

SamplerState gsamPointWrap        : register(s0);
SamplerState gsamPointClamp       : register(s1);
SamplerState gsamLinearWrap       : register(s2);
SamplerState gsamLinearClamp      : register(s3);
SamplerState gsamAnisotropicWrap  : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);
SamplerComparisonState gsamShadow : register(s6);

float4 CameraLoc:register(b1);

cbuffer cbPerObject : register(b0)
{
	float4x4 gWorldViewProj;
	float4x4 gWorldViewProjMat4;
	float4x4 Rotator;
	float4x4 gWorld;
	float4x4 gLightVP;
	float4x4 gShadowTransform;
	float4x4 gLightMVP;
	float    Time;
};

struct VertexIn
{
	float3 PosL      : POSITION;
	float4 Color     : COLOR;
	float4 Normal    : NORMAL;
	float4 Tangent   : TANGENT;
	float4 Biangent  : BITANGENT;
	float2 TexCoord  : TEXCOORD;
};

struct VertexOut
{
	float4 PosH  : SV_POSITION;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	//float QuickSpeed = 15;
	//float MidSpeed = 10;
	//float SlowSpeed = 5;

	//float3 PosW;
	//PosW.x = vin.PosL.x + sin(Time * 1.5) * 75;
	//PosW.y = vin.PosL.y + sin(Time * MidSpeed);
	//PosW.z = vin.PosL.z + sin(Time * SlowSpeed);

	//float4 posW = mul(float4(vin.PosL, 1.0f), gWorld);
	//vout.PosH = mul(posW, gLightViewProj);
	vout.PosH = mul(float4(vin.PosL, 1.0f), gLightMVP);

	//vout.PosH = mul(float4(PosW, 1.0f), gLightWorldViewProj);

	return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
	return 0;
}