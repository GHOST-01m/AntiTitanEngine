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
int2 RenderTargetSize:register(b2);
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

float CalcShadowFactor(float4 shadowPosH)//���żӸ�: SV_Position?
{
	// Complete projection by doing division by w.
	shadowPosH.xyz /= shadowPosH.w;

	// Depth in NDC space.
	float depth = shadowPosH.z;

	uint width, height, numMips;
	gShadowMap.GetDimensions(0, width, height, numMips);

	// Texel size.
	float dx = 1.0f / (float)width;

	float percentLit = 0.0f;
	const float2 offsets[9] =
	{
		float2(-dx,  -dx), float2(0.0f,  -dx), float2(dx,  -dx),
		float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
		float2(-dx,  +dx), float2(0.0f,  +dx), float2(dx,  +dx)
	};

	[unroll]
	for (int i = 0; i < 9; ++i)
	{
		percentLit += gShadowMap.SampleCmpLevelZero(gsamShadow,
			shadowPosH.xy + offsets[i], depth).r;
	}
	return percentLit / 9.0f;

	//---------------------------------------------
	//shadowPosH.xyz /= shadowPosH.w;
	//float currentDepth = shadowPosH.z;
	//uint width, height, numMips;
	//gShadowMap.GetDimensions(0, width, height, numMips);
	//float2 PiexlPos = shadowPosH.xy * width;
	//float depthInMap = gShadowMap.Load(int3(PiexlPos, 0)).r;

	//return currentDepth > depthInMap ? 0 : 1;
}

float3 SchlickFresnel(float3 R0, float3 normal, float3 lightVec)
{
	float cosIncidentAngle = saturate(dot(normal, lightVec));

	float f0 = 1.0f - cosIncidentAngle;
	float3 reflectPercent = R0 + (1.0f - R0) * (f0 * f0 * f0 * f0 * f0);

	return reflectPercent;
}

float3 BlinnPhong(
	float3 lightStrength,	float3 lightVec,
	float3 normal,	float3 toEye, 
	float4 gDiffuseAlbedo,	float3 gFresnelR0,	float gRoughness)
{
	const float shininess = 1.0f - gRoughness;
	const float m = shininess * 256.0f;
	float3 halfVec = normalize(toEye + lightVec);

	float roughnessFactor = (m + 8.0f) * pow(max(dot(halfVec, normal), 0.00000001f), m) / 8.0f;
	float3 fresnelFactor = SchlickFresnel(gFresnelR0, halfVec, lightVec);

	float3 specAlbedo = fresnelFactor * roughnessFactor;

	specAlbedo = specAlbedo / (specAlbedo + 1.0f);

	return (gDiffuseAlbedo.rgb + specAlbedo*50) * lightStrength;
}

float3 ComputeDirectionalLight(
	float3 LightDirection, float3 LightStrength,
	float4 gDiffuseAlbedo,float3 gFresnelR0,float gRoughness,
	float3 normal, float3 toEye)
{
	//float3 lightVec = LightLocation;

	float3 lightVec = -LightDirection;
	float  ndotl = max(dot(lightVec, normal), 0.0f);
	float3 lightStrength = LightStrength * ndotl;

	return BlinnPhong(lightStrength, lightVec, normal, toEye, gDiffuseAlbedo, gFresnelR0, gRoughness);
}

float Luminance(float3 InColor)
{
	return dot(InColor, float3(0.3f, 0.59f, 0.11f));
}
//VS=================================================================================
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

	//��NTB����ȷ����ת����
	vin.Normal = mul(vin.Normal, Rotator);
	vin.Tangent = mul(vin.Tangent, Rotator);
	vin.Bitangent = mul(vin.Bitangent, Rotator);
	//vin.Normal = normalize(vin.Normal);

	//vout.Color = (ColorChange * 0.5f + 0.5f);//����ɫ
	vout.Color            = (vin.Normal * 0.5f + 0.5f);
	vout.TexCoord         = vin.TexCoord;
	vout.PosH             = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
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
	float4 diffuseAlbedo = gDiffuseMap.Sample(gsamAnisotropicWrap, pin.TexCoord);
	float4 NormalMap = gNormalMap.Sample(gsamAnisotropicWrap, pin.TexCoord);

	NormalMap = NormalMap * 2 - 1;
	float3x3 worldToTangent = float3x3(pin.TangentW, pin.BitangentW, pin.NormalW);

	NormalMap.xyz = normalize(mul(NormalMap.xyz, worldToTangent));
	//NormalMap.xyz = mul(NormalMap.xyz, Rotator);
	// Just pass vertex color into the pixel shader.
	float shadowFactor = CalcShadowFactor(pin.ShadowPosH);
	//float4 FinalColor = (shadowFactor + 0.1) * (diffuseAlbedo);
	//float4 FinalColor = (shadowFactor + 0.1) * (pin.Color);

	float3 Fresnel = gFresnelR0;
	//float3 Fresnel = float3(0.95, 0.93, 0.88);//�������Fresnel
	//float4 FinalColor = (float4( 0.98 ,0.97 ,0.95 ,1.0));//������Ļ�����ɫ

	//float4 FinalColor = diffuseAlbedo;//����ͼ��Ϊ��ɫ
	float4 FinalColor = (pin.Color);//��Normal��Ϊ��ɫ

	//DiffuseAlbedo��ԭ������ɫ
	float4 directLight = float4(ComputeDirectionalLight(
		LightDirection.xyz, LightStrength.xyz,
		FinalColor, Fresnel, gRoughness,
		//NormalMap.xyz, normalize(CameraLocation-pin.PosW)), 1);//��Normal��ͼ
		normalize(pin.NormalW), normalize(CameraLocation-pin.PosW)), 1)*2;//��ԭ��������Normal


	float4 AmbientAlbedo = FinalColor * 0.03;
	FinalColor = AmbientAlbedo + (shadowFactor + 0.1) * (directLight);
	//return FinalColor+directLight;

	float TotalLuminance = Luminance(FinalColor.rgb);
	float BloomLuminance = TotalLuminance - 1.0f;
	float Amount = saturate(BloomLuminance * 0.5f);
	float4 OutColor;
	OutColor.rgb = FinalColor.rgb;
	OutColor.rgb *= Amount;
	OutColor.a = 0.0f;

	// if(FinalColor.x<1 && FinalColor.y<1 && FsinalColor.z<1){
	// 	FinalColor.xyz=0;
	// }
	

	return pow(FinalColor, 1 / 2.2f);
	//return FinalColor;
	//return OutColor;

};