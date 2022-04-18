
Texture2D    gDiffuseMap : register(t0);
Texture2D    gSunMergeColor  : register(t1);
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

float3 ACESToneMapping(float3 color, float adapted_lum)
{
	const float A = 2.51f;
	const float B = 0.03f;
	const float C = 2.43f;
	const float D = 0.59f;
	const float E = 0.14f;

	color *= adapted_lum;
	return (color * (A * color + B)) / (color * (C * color + D) + E);
}

float randomNoise(float x)
{
	return frac(sin(dot(x,78.233)) * 43758.5453);
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
	int X = floor(pin.PosH.x);
	int Y = floor(pin.PosH.y);

	float Width = RenderTargetSize[0] ;
	float Height = RenderTargetSize[1] ;

	float DeltaU = 1.0f / RenderTargetSize[0];
	float DeltaV = 1.0f / RenderTargetSize[1];

	float2 Tex;
	Tex.x = 1.0f * X / Width;
	Tex.y = 1.0f * Y / Height;

	//float4 Color0 = gDiffuseMap.Sample(gsamLinearClamp, Tex + float2(-DeltaU, -DeltaV));
	//float4 Color1 = gDiffuseMap.Sample(gsamLinearClamp, Tex + float2(+DeltaU, -DeltaV));
	//float4 Color2 = gDiffuseMap.Sample(gsamLinearClamp, Tex + float2(-DeltaU, +DeltaV));
	//float4 Color3 = gDiffuseMap.Sample(gsamLinearClamp, Tex + float2(+DeltaU, +DeltaV));
	//float4 Color4 = gDiffuseMap.Sample(gsamLinearClamp, Tex + float2(-DeltaU, 0));
	//float4 Color5 = gDiffuseMap.Sample(gsamLinearClamp, Tex + float2(+DeltaU, 0));
	//float4 Color6 = gDiffuseMap.Sample(gsamLinearClamp, Tex + float2(0, +DeltaV));
	//float4 Color7 = gDiffuseMap.Sample(gsamLinearClamp, Tex + float2(0, +DeltaV));
	//float4 AvailableColor = Color0 * 0.25f + Color1 * 0.25f + Color2 * 0.25f + Color3 * 0.25f;
	//float4 AvailableColor = Color4 * 0.25f + Color5 * 0.25f + Color6 * 0.25f + Color7 * 0.25f;

	//float4 AvailableColor = (Color0  + Color1  + Color2  + Color3  +Color4  + Color5 + Color6  + Color7 ) * 0.125f;

	float4 OutColor;

	float  BlockSize = 100.0f;
	float2 block = randomNoise(floor((Tex * BlockSize).x));
	float  displaceNoise = pow(block.x, 8.0) * pow(block.x, 3.0);

	OutColor.r = gDiffuseMap.Sample(gsamLinearClamp, Tex);
	//OutColor.g = gDiffuseMap.Sample(gsamLinearClamp, Tex + float2(-DeltaU * 5, -DeltaV));
	//OutColor.b = gDiffuseMap.Sample(gsamLinearClamp, Tex + float2(+DeltaU * 5, -DeltaV));
	OutColor.g = gDiffuseMap.Sample(gsamLinearClamp, Tex + float2(DeltaU*10, 0.0f));
	OutColor.b = gDiffuseMap.Sample(gsamLinearClamp, Tex + float2(DeltaU*10, 0.0f));

	//OutColor.rgb = (AvailableColor.rgb);

	OutColor.a = 1.0f;

	return OutColor;
};
