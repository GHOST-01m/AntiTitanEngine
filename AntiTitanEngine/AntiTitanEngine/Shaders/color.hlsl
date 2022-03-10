//***************************************************************************************
// color.hlsl by Frank Luna (C) 2015 All Rights Reserved.
//
// Transforms and colors geometry.
//***************************************************************************************

cbuffer cbPerObject : register(b0)
{
	float4x4 gWorldViewProj; 
	float4x4 gWorldViewProjMat4;
	float  Time;
};

struct VertexIn
{
	float3 PosL  : POSITION;
    float4 Color : COLOR;
	float4 Normal: NORMAL;
};

struct VertexOut
{
	float4 PosH  : SV_POSITION;
    float4 Color : COLOR;
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

	// Transform to homogeneous clip space.
	//vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
	vout.PosH = mul(float4(PosW, 1.0f), gWorldViewProj);


	//vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProjMat4);

	// Just pass vertex color into the pixel shader.
	vout.Color = (vin.Normal * 0.5f + 0.5f);
    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    return pin.Color;
}


