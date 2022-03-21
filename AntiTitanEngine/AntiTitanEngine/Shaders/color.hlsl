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
	float4x4   Rotator;
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
    float4 Color : COLOR;
	float2 TexCoord  : TEXCOORD;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	float QuickSpeed = 15;
	float MidSpeed   = 10;
	float SlowSpeed  = 5;

	float3 PosW;
	PosW.x = vin.PosL.x + sin(Time * 1.5)* 75;
	PosW.y = vin.PosL.y + sin(Time * MidSpeed);
	PosW.z = vin.PosL.z + sin(Time * SlowSpeed);

	//Transform to homogeneous clip space.
	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);

	//if (CanMove==1) {
	//	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
	//}
	//else {
		vout.PosH = mul(float4(PosW, 1.0f), gWorldViewProj);
	//}

	//vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProjMat4);

	float4 ColorChange;
	float frequency=2;

	ColorChange.x = vin.Normal.x * sin(Time*20)+0.5;
	ColorChange.y = vin.Normal.y ;
	ColorChange.z = vin.Normal.z ;
	ColorChange.w = vin.Normal.w ;

	//对normal做正确的旋转处理
	vin.Normal = mul(vin.Normal, Rotator);
	//vin.Normal = normalize(vin.Normal);

	// Just pass vertex color into the pixel shader.
	vout.Color = (vin.Normal * 0.5f + 0.5f);
	//vout.Color = (ColorChange * 0.5f + 0.5f);//动颜色
	vout.TexCoord = vin.TexCoord;

    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
	float4 diffuseAlbedo = gDiffuseMap.Sample(gsamAnisotropicWrap, pin.TexCoord);
	float4 NormalMap = gNormalMap.Sample(gsamAnisotropicWrap, pin.TexCoord);

	//return pin.Color;
	//return pow(pin.Color,1/2.2f);
	return diffuseAlbedo+ NormalMap;
}


