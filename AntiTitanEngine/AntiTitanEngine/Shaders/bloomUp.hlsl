//***************************************************************************************
// color.hlsl by Frank Luna (C) 2015 All Rights Reserved.
//
// Transforms and colors geometry.
//***************************************************************************************

Texture2D    gBloomUp : register(t0);
Texture2D    gBloomDown  : register(t1);
Texture2D    gShadowMap  : register(t2);

SamplerState gsamPointWrap        : register(s0);
SamplerState gsamPointClamp       : register(s1);
SamplerState gsamLinearWrap       : register(s2);
SamplerState gsamLinearClamp      : register(s3);
SamplerState gsamAnisotropicWrap  : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);
SamplerComparisonState gsamShadow : register(s6);

float4 CameraLoc:register(b1);
int4 RenderTargetSize:register(b2);

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
	float4 PosH  : SV_POSITION;//��������MVP�任
    float4 Color : COLOR;
	float4 ShadowPosH : POSITION0;//����������Ӱ�任
	float3 PosW    : POSITION1;//��������M�任
	float2 TexCoord  : TEXCOORD;
	float3 NormalW : NORMAL;//Nromal����M�任
	float3 TangentW : TANGENT;//Tangent����M�任
	float3 BitangentW : BITANGENT;//Bitangent����M�任
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

	vin.Normal = mul(vin.Normal, Rotator);
	vin.Tangent = mul(vin.Tangent, Rotator);
	vin.Bitangent = mul(vin.Bitangent, Rotator);
	//vin.Normal = normalize(vin.Normal);

	//vout.Color = (ColorChange * 0.5f + 0.5f);//����ɫ
	vout.Color            = (vin.Normal * 0.5f + 0.5f);
	vout.TexCoord         = vin.TexCoord;
	//vout.PosH             = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
	vout.PosH             = float4(vin.PosL, 1.0f);
	float4 posw = mul(float4(vin.PosL, 1.0f), gWorld);
	vout.PosW             = posw.xyz;
	vout.ShadowPosH       = mul(float4(vin.PosL, 1.0f), gLightWorldViewProjT);
	vout.NormalW          = vin.Normal.xyz;
	vout.TangentW         = vin.Tangent.xyz;
	vout.BitangentW       = vin.Bitangent.xyz;

    return vout;
}

//PS====================================================================================
float4 PS(VertexOut pin) : SV_Target
{
	float4 OutColor = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float BloomWeightScalar = 1.0f / RenderTargetSize[2];
	float BloomWeightScalar1 = 1.0f / RenderTargetSize[3];
	//float BloomWeightScalar = 1.0f / gRenderTargetSize[2];
	//float BloomWeightScalar1 = 1.0f / gRenderTargetSize[3];

	float BloomUpScale = 1.32f;

	int X = floor(pin.PosH.x);
	int Y = floor(pin.PosH.y);

	float2 Tex;
	Tex.x = 1.0f * X / RenderTargetSize[0];
	Tex.y = 1.0f * Y / RenderTargetSize[1];

	float DeltaU = 1.0f / RenderTargetSize[0];
	float DeltaV = 1.0f / RenderTargetSize[1];
	//float2 Tex;
	//Tex.x = 1.0f * X / gRenderTargetSize[0];
	//Tex.y = 1.0f * Y / gRenderTargetSize[1];

	//float DeltaU = 1.0f / gRenderTargetSize[0];
	//float DeltaV = 1.0f / gRenderTargetSize[1];

	float2 DeltaUV = float2(DeltaU, DeltaV);

	float Start = 2.0 / 7.0;
	float4 Color0 = gBloomUp.Sample(gsamLinearWrap, Tex + DeltaUV * BloomUpScale * Circle(Start, 7.0, 0.0f));
	float4 Color1 = gBloomUp.Sample(gsamLinearWrap, Tex + DeltaUV * BloomUpScale * Circle(Start, 7.0, 1.0f));
	float4 Color2 = gBloomUp.Sample(gsamLinearWrap, Tex + DeltaUV * BloomUpScale * Circle(Start, 7.0, 2.0f));
	float4 Color3 = gBloomUp.Sample(gsamLinearWrap, Tex + DeltaUV * BloomUpScale * Circle(Start, 7.0, 3.0f));
	float4 Color4 = gBloomUp.Sample(gsamLinearWrap, Tex + DeltaUV * BloomUpScale * Circle(Start, 7.0, 4.0f));
	float4 Color5 = gBloomUp.Sample(gsamLinearWrap, Tex + DeltaUV * BloomUpScale * Circle(Start, 7.0, 5.0f));
	float4 Color6 = gBloomUp.Sample(gsamLinearWrap, Tex + DeltaUV * BloomUpScale * Circle(Start, 7.0, 6.0f));
	float4 Color7 = gBloomUp.Sample(gsamLinearWrap, Tex);

	float4 Color8 = gBloomDown.Sample(gsamLinearWrap, Tex + DeltaUV * BloomUpScale * Circle(Start, 7.0, 0.0f));
	float4 Color9 = gBloomDown.Sample(gsamLinearWrap, Tex + DeltaUV * BloomUpScale * Circle(Start, 7.0, 1.0f));
	float4 Color10 = gBloomDown.Sample(gsamLinearWrap, Tex + DeltaUV * BloomUpScale * Circle(Start, 7.0, 2.0f));
	float4 Color11 = gBloomDown.Sample(gsamLinearWrap, Tex + DeltaUV * BloomUpScale * Circle(Start, 7.0, 3.0f));
	float4 Color12 = gBloomDown.Sample(gsamLinearWrap, Tex + DeltaUV * BloomUpScale * Circle(Start, 7.0, 4.0f));
	float4 Color13 = gBloomDown.Sample(gsamLinearWrap, Tex + DeltaUV * BloomUpScale * Circle(Start, 7.0, 5.0f));
	float4 Color14 = gBloomDown.Sample(gsamLinearWrap, Tex + DeltaUV * BloomUpScale * Circle(Start, 7.0, 6.0f));
	float4 Color15 = gBloomDown.Sample(gsamLinearWrap, Tex);

	float4 BloomWight = float4(BloomWeightScalar, BloomWeightScalar, BloomWeightScalar, 0.0f);
	float4 BloomWight1 = float4(BloomWeightScalar1, BloomWeightScalar1, BloomWeightScalar1, 0.0f);

	OutColor = (Color0 + Color1 + Color2 + Color3 + Color4 + Color5 + Color6 + Color7) * BloomWight +
		(Color8 + Color9 + Color10 + Color11 + Color12 + Color13 + Color14 + Color15) * BloomWight1;
	OutColor.a = 0.0f;

	return OutColor;
};