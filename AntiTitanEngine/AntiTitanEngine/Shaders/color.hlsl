//***************************************************************************************
// color.hlsl by Frank Luna (C) 2015 All Rights Reserved.
//
// Transforms and colors geometry.
//***************************************************************************************

Texture2D    gDiffuseMap : register(t0);
Texture2D    gNormalMap  : register(t1);
Texture2D    gShadowMap  : register(t2);

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
	float4x4 gWorld;                  //ת�ã��������
	float4x4 gLightVP;                //ת��            (���V���� * ���P����)
	float4x4 gShadowTransform;        //ת��            (���V���� * ���P���� * T����
	float4x4 gLightWorldViewProj;     //ת�ã��������  * ���V���� * ���P����)
	float4x4 gLightWorldViewProjT;    //ת�ã��������  * ���V���� * ���P���� * T����
	//float    Time;
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
	float4 PosH  : SV_POSITION;//��������MVP�任
    float4 Color : COLOR;
	float4 ShadowPosH : POSITION0;//����������Ӱ�任
	float3 PosW    : POSITION1;//��������M�任
	float2 TexCoord  : TEXCOORD;
	float3 NormalW : NORMAL;//Nromal����M�任
};


float CalcShadowFactor(float4 shadowPosH)//���żӸ�: SV_Position?
{
	//// Complete projection by doing division by w.
	//shadowPosH.xyz /= shadowPosH.w;

	//// Depth in NDC space.
	//float depth = shadowPosH.z;

	//uint width, height, numMips;
	//gShadowMap.GetDimensions(0, width, height, numMips);

	//// Texel size.
	//float dx = 1.0f / (float)width;

	//float percentLit = 0.0f;
	//const float2 offsets[9] =
	//{
	//	float2(-dx,  -dx), float2(0.0f,  -dx), float2(dx,  -dx),
	//	float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
	//	float2(-dx,  +dx), float2(0.0f,  +dx), float2(dx,  +dx)
	//};

	//[unroll]
	//for (int i = 0; i < 9; ++i)
	//{
	//	percentLit += gShadowMap.SampleCmpLevelZero(gsamShadow,
	//		shadowPosH.xy + offsets[i], depth).r;
	//}
	//return percentLit / 9.0f;

	//---------------------------------------------
	shadowPosH.xyz /= shadowPosH.w;
	float currentDepth = shadowPosH.z;
	uint width, height, numMips;
	gShadowMap.GetDimensions(0, width, height, numMips);
	float2 PiexlPos = shadowPosH.xy * width;
	float depthInMap = gShadowMap.Load(int3(PiexlPos, 0)).r;

	return currentDepth > depthInMap ? 0 : 1;
}

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	float QuickSpeed = 15;
	float MidSpeed   = 10;
	float SlowSpeed  = 5;

	//float3 PosW;
	//PosW.x = vin.PosL.x + sin(Time * 1.5)* 75;
	//PosW.y = vin.PosL.y + sin(Time * MidSpeed);
	//PosW.z = vin.PosL.z + sin(Time * SlowSpeed);

	//Transform to homogeneous clip space.
	//vout.PosH = mul(float4(PosW, 1.0f), gWorldViewProj);

	//vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProjMat4);

	//float4 ColorChange;
	//float frequency=2;

	//ColorChange.x = vin.Normal.x * sin(Time*20)+0.5;
	//ColorChange.y = vin.Normal.y ;
	//ColorChange.z = vin.Normal.z ;
	//ColorChange.w = vin.Normal.w ;

	//��normal����ȷ����ת����
	vin.Normal            = mul(vin.Normal, Rotator);
	//vin.Normal = normalize(vin.Normal);

	//vout.Color = (ColorChange * 0.5f + 0.5f);//����ɫ
	vout.Color            = (vin.Normal * 0.5f + 0.5f);
	vout.TexCoord         = vin.TexCoord;
	vout.PosH             = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
	float4 posw = mul(float4(vin.PosL, 1.0f), gWorld);
	vout.PosW             = posw.xyz;
	vout.ShadowPosH       = mul(float4(vin.PosL, 1.0f), gLightWorldViewProjT);
	vout.NormalW          = vin.Normal;
    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
	float4 diffuseAlbedo = gDiffuseMap.Sample(gsamAnisotropicWrap, pin.TexCoord);
	float4 NormalMap = gNormalMap.Sample(gsamAnisotropicWrap, pin.TexCoord);

	// Just pass vertex color into the pixel shader.
	float shadowFactor = CalcShadowFactor(pin.ShadowPosH);
	float4 FinalColor =  (shadowFactor + 0.1) * (pin.Color);
	//pow(FinalColor, 1 / 2.2f)
	return pow(FinalColor, 1 / 2.2f);
	//return FinalColor;
	//return pow(pin.Color, 1 / 2.2f);
	//return diffuseAlbedo+ NormalMap;
}


