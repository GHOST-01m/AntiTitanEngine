//***************************************************************************************
// color.hlsl by Frank Luna (C) 2015 All Rights Reserved.
//
// Transforms and colors geometry.
//***************************************************************************************

Texture2D    gBloomInput : register(t0);
Texture2D    gNormalMap  : register(t1);
Texture2D    gShadowMap  : register(t2);

SamplerState gsamPointWrap        : register(s0);
SamplerState gsamPointClamp       : register(s1);
SamplerState gsamLinearWrap       : register(s2);
SamplerState gsamLinearClamp      : register(s3);
SamplerState gsamAnisotropicWrap  : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);
SamplerComparisonState gsamShadow : register(s6);

float4 RenderTargetSize:register(b1);
int4 bagabaga:register(b2);

cbuffer cbPerObject : register(b0)
{
	float4x4 gWorldViewProj; 
	float4x4 gWorldViewProjMat4;
	float4x4 Rotator;
	float4x4 gWorld;                  //ת�ã��������
	float4x4 gLightVP;                //ת��            (���V���� * ���P����)
	float4x4 gShadowTransform;        //ת��            (���V���� * ���P���� * T����
	float4x4 gLightWorldViewProj;     //ת�ã��������  * ���V���� * ���P����)
	float4x4 gLightWorldViewProjT;    //ת�ã��������  * ���V���� * ���P���� * T����
	float4 LightDirection;
	float4 LightStrength;
	float4 gDiffuseAlbedo;
	float3 gFresnelR0;
	float  gRoughness;
	float3 LightLocation;
	float LightLocationW;
	float3 CameraLocation;
	float CameraLocationW;
	int4 gRenderTargetSize;
	//float    Time;
};

struct VertexIn
{
	float3 PosL      : POSITION;
    float4 Color     : COLOR;
	float4 Normal    : NORMAL;
	float4 Tangent   : TANGENT;
	float4 Bitangent  : BITANGENT;
	float2 TexCoord  : TEXCOORD;
};

struct VertexOut
{
	float4 PosH  : SV_POSITION;
};

float2 Circle(float Start, float Points, float Point)
{
	float Radians = (2.0f * 3.141592f * (1.0f / Points)) * (Start + Point);
	return float2(cos(Radians), sin(Radians));
}

//VS=================================================================================
VertexOut VS(VertexIn vin)
{
	VertexOut vout;

	vout.PosH = float4(vin.PosL, 1.0f);

    return vout;
}

//PS====================================================================================
float4 PS(VertexOut pin) : SV_Target
{
	float BloomDownScale = 2.5f;

	int X = floor(pin.PosH.x);
	int Y = floor(pin.PosH.y);

	float Width = RenderTargetSize[0] *0.5;
	float Height = RenderTargetSize[1] *0.5;

	float DeltaU = 1.0f / (RenderTargetSize[0]);
	float DeltaV = 1.0f / (RenderTargetSize[1]);
	
	//float Width = gRenderTargetSize[0] *0.5;
	//float Height = gRenderTargetSize[1] *0.5;

	//float DeltaU = 1.0f / (gRenderTargetSize[0]);
	//float DeltaV = 1.0f / (gRenderTargetSize[1]);

	float2 Tex;
	Tex.x = 1.0f * X / Width;
	Tex.y = 1.0f * Y / Height;

	float2 DeltaUV = float2(DeltaU, DeltaV);

	float StartRaduas = 2.0f / 14.0f;
	float4 Color0 = gBloomInput.Sample(gsamPointClamp, Tex + DeltaUV * BloomDownScale * Circle(StartRaduas, 14.0f, 0.0f));
	float4 Color1 = gBloomInput.Sample(gsamLinearClamp, Tex + DeltaUV * BloomDownScale * Circle(StartRaduas, 14.0f, 1.0f));
	float4 Color2 = gBloomInput.Sample(gsamLinearClamp, Tex + DeltaUV * BloomDownScale * Circle(StartRaduas, 14.0f, 2.0f));
	float4 Color3 = gBloomInput.Sample(gsamLinearClamp, Tex + DeltaUV * BloomDownScale * Circle(StartRaduas, 14.0f, 3.0f));
	float4 Color4 = gBloomInput.Sample(gsamLinearClamp, Tex + DeltaUV * BloomDownScale * Circle(StartRaduas, 14.0f, 4.0f));
	float4 Color5 = gBloomInput.Sample(gsamLinearClamp, Tex + DeltaUV * BloomDownScale * Circle(StartRaduas, 14.0f, 5.0f));
	float4 Color6 = gBloomInput.Sample(gsamLinearClamp, Tex + DeltaUV * BloomDownScale * Circle(StartRaduas, 14.0f, 6.0f));
	float4 Color7 = gBloomInput.Sample(gsamLinearClamp, Tex + DeltaUV * BloomDownScale * Circle(StartRaduas, 14.0f, 7.0f));
	float4 Color8 = gBloomInput.Sample(gsamLinearClamp, Tex + DeltaUV * BloomDownScale * Circle(StartRaduas, 14.0f, 8.0f));
	float4 Color9 = gBloomInput.Sample(gsamLinearClamp, Tex + DeltaUV * BloomDownScale * Circle(StartRaduas, 14.0f, 9.0f));
	float4 Color10 = gBloomInput.Sample(gsamLinearClamp, Tex + DeltaUV * BloomDownScale * Circle(StartRaduas, 14.0f, 10.0f));
	float4 Color11 = gBloomInput.Sample(gsamLinearClamp, Tex + DeltaUV * BloomDownScale * Circle(StartRaduas, 14.0f, 11.0f));
	float4 Color12 = gBloomInput.Sample(gsamLinearClamp, Tex + DeltaUV * BloomDownScale * Circle(StartRaduas, 14.0f, 12.0f));
	float4 Color13 = gBloomInput.Sample(gsamLinearClamp, Tex + DeltaUV * BloomDownScale * Circle(StartRaduas, 14.0f, 13.0f));

	float4 Color = gBloomInput.Sample(gsamLinearClamp, Tex);

	float Weight = 1.0f / 15.0f;

	Color = Weight * (Color + Color0 + Color1 + Color2 + Color3
		+ Color4 + Color5 + Color6 + Color7
		+ Color8 + Color9 + Color10 + Color11 + Color12 + Color13);

	return Color;
};