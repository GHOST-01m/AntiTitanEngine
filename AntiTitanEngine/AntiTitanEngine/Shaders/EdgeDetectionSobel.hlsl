
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

float intensity(float4 color)
{
	return sqrt((color.x * color.x) + (color.y * color.y) + (color.z * color.z));
}

float sobel(float DeltaU, float DeltaV, float2 Tex) 
{
	float topLeft     = intensity(gDiffuseMap.Sample(gsamLinearClamp, Tex + float2(-DeltaU , DeltaV)));
	float midLeft     = intensity(gDiffuseMap.Sample(gsamLinearClamp, Tex + float2(-DeltaU, 0)));
	float bottomLeft  = intensity(gDiffuseMap.Sample(gsamLinearClamp, Tex + float2(-DeltaU, -DeltaV)));
	
	float topRight = intensity(gDiffuseMap.Sample(gsamLinearClamp, Tex + float2(DeltaU, DeltaV)));
	float midRight = intensity(gDiffuseMap.Sample(gsamLinearClamp, Tex + float2(DeltaU, 0)));
	float bottomRight = intensity(gDiffuseMap.Sample(gsamLinearClamp, Tex + float2(DeltaU, -DeltaV)));
	
	float topMid = intensity(gDiffuseMap.Sample(gsamLinearClamp, Tex + float2(0, DeltaV)));
	float bottomMid = intensity(gDiffuseMap.Sample(gsamLinearClamp, Tex + float2(0, -DeltaV)));

	float Gx = topLeft + 2.0 * midLeft + bottomLeft - topRight - 2.0 * midRight - bottomRight;
	float Gy = -topLeft - 2.0 * topMid - topRight + bottomLeft + 2.0 * bottomMid + bottomRight;
	float sobelGradient = sqrt((Gx * Gx) + (Gy * Gy));
	return sobelGradient;
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

	float4 _BackgroundColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
	float4 _BackgroundFade = float4(0.0f, 0.0f, 1.0f, 1.0f);
	float4 _EdgeColor = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float4 sceneColor = gDiffuseMap.Sample(gsamLinearClamp, Tex);
	float  sobelGradient = sobel(DeltaU, DeltaV, Tex);
	{
		//输出test，中间一个点*8减周围八个点
		float4 test;

		float4 topLeft = gDiffuseMap.Sample(gsamLinearClamp, Tex + float2(-DeltaU, DeltaV));
		float4 midLeft = (gDiffuseMap.Sample(gsamLinearClamp, Tex + float2(-DeltaU, 0)));
		float4 bottomLeft = (gDiffuseMap.Sample(gsamLinearClamp, Tex + float2(-DeltaU, -DeltaV)));

		float4 topRight = (gDiffuseMap.Sample(gsamLinearClamp, Tex + float2(DeltaU, DeltaV)));
		float4 midRight = (gDiffuseMap.Sample(gsamLinearClamp, Tex + float2(DeltaU, 0)));
		float4 bottomRight = (gDiffuseMap.Sample(gsamLinearClamp, Tex + float2(DeltaU, -DeltaV)));

		float4 topMid = (gDiffuseMap.Sample(gsamLinearClamp, Tex + float2(0, DeltaV)));
		float4 bottomMid = (gDiffuseMap.Sample(gsamLinearClamp, Tex + float2(0, -DeltaV)));

		float4 mid = gDiffuseMap.Sample(gsamLinearClamp, Tex);

		test = (mid * 8.0f) - topLeft - midLeft - bottomLeft - topRight - midRight - bottomRight - topMid - bottomMid;
	}
	float4 backgroundColor = lerp(_BackgroundColor, _BackgroundColor, _BackgroundColor);
	float3 edgeColor = lerp(backgroundColor.rgb, _EdgeColor.rgb, sobelGradient);

	float4 OutColor = float4(edgeColor,1.0f);

	return OutColor;
};
