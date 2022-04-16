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

float4 RenderTargetSize:register(b1);
float4 bagabaga:register(b2);

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


float Luminance(float3 InColor)
{
	return dot(InColor, float3(0.3f, 0.59f, 0.11f));
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

	float Width = RenderTargetSize[0] * 0.25f;
	float Height = RenderTargetSize[1] * 0.25f;

	float DeltaU = 1.0f / RenderTargetSize[0];
	float DeltaV = 1.0f / RenderTargetSize[1];
	
	//float Width = gRenderTargetSize[0] * 0.25f;
	//float Height = gRenderTargetSize[1] * 0.25f;

	//float DeltaU = 1.0f / (gRenderTargetSize[0]);
	//float DeltaV = 1.0f / (gRenderTargetSize[1]);

	int X = floor(pin.PosH.x);
	int Y = floor(pin.PosH.y);

	float2 Tex;
	Tex.x = 1.0f * X / Width;
	Tex.y = 1.0f * Y / Height;

	float4 Color0 = gDiffuseMap.Sample(gsamLinearWrap, Tex + float2(-DeltaU, -DeltaV));
	float4 Color1 = gDiffuseMap.Sample(gsamLinearWrap, Tex + float2(+DeltaU, -DeltaV));
	float4 Color2 = gDiffuseMap.Sample(gsamLinearWrap, Tex + float2(-DeltaU, +DeltaV));
	float4 Color3 = gDiffuseMap.Sample(gsamLinearWrap, Tex + float2(+DeltaU, +DeltaV));

	float4 AvailableColor = Color0 * 0.25f + Color1 * 0.25f + Color2 * 0.25f + Color3 * 0.25f;

	float TotalLuminance = Luminance(AvailableColor.rgb);
	float BloomLuminance = TotalLuminance - 1.0f;
	float Amount = saturate(BloomLuminance * 0.5f);
	float4 OutColor;
	OutColor.rgb = AvailableColor.rgb;
	OutColor.rgb *= Amount;
	OutColor.a = 1.0f;

	return OutColor;
};