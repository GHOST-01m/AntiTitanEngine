#include "stdafx.h"
#include "FLight.h"

FLight::FLight()
{
	mSceneBounds.Center = XMFLOAT3(0.0f, 0.0f, 0.0f);
	//mSceneBounds.Center = XMFLOAT3(-955.0f, -335.0f, 158.0f);
	mSceneBounds.Radius = sqrtf(10.0f* 10.0f+ 15.0f* 15.0f);
	//mSceneBounds.Radius = sqrtf(3000000);
}

void FLight::LoadLightFromBat(const std::string& filepath)
{
	std::ifstream BatFile(filepath, std::ios::in | std::ios::binary);
	int32_t DataLength;

	BatFile.read((char*)&DataLength, sizeof(int32_t));//第一个是Light数量
	BatFile.read((char*)&DataLength, sizeof(int32_t));
	mLightInfo.ActorName.resize(DataLength);
	BatFile.read((char*)mLightInfo.ActorName.data(), sizeof(char) * DataLength);
	BatFile.read((char*)&mLightInfo.mTransform, sizeof(Transform) );
	BatFile.read((char*)&mLightInfo.Rotation, sizeof(Float3));
	BatFile.read((char*)&mLightInfo.Intensity, sizeof(float));
	BatFile.read((char*)&mLightInfo.Direction, sizeof(FVector));
	BatFile.read((char*)&mLightInfo.LinearColor, sizeof(Color));//Mesh索引长度

	BatFile.close();
}

XMMATRIX FLight::GetView()
{
	XMVECTOR R = XMLoadFloat3(&mRight);
	XMVECTOR U = XMLoadFloat3(&mUp);
	XMVECTOR L = XMLoadFloat3(&mLook);
	XMFLOAT3 Pf3 = { mLightInfo.mTransform.translation.x,mLightInfo.mTransform.translation.y, mLightInfo.mTransform.translation .z};
	XMVECTOR P = XMLoadFloat3(&Pf3);


	// Keep camera's axes orthogonal to each other and of unit length.
	L = XMVector3Normalize(L);
	U = XMVector3Normalize(XMVector3Cross(L, R));

	// U, L already ortho-normal, so no need to normalize cross product.
	R = XMVector3Cross(U, L);

	// Fill in the view matrix entries.
	float x = -XMVectorGetX(XMVector3Dot(P, R));
	float y = -XMVectorGetX(XMVector3Dot(P, U));
	float z = -XMVectorGetX(XMVector3Dot(P, L));

	XMStoreFloat3(&mRight, R);
	XMStoreFloat3(&mUp, U);
	XMStoreFloat3(&mLook, L);

	//填观察矩阵
	mView(0, 0) = mRight.x;
	mView(1, 0) = mRight.y;
	mView(2, 0) = mRight.z;
	mView(3, 0) = x;

	mView(0, 1) = mUp.x;
	mView(1, 1) = mUp.y;
	mView(2, 1) = mUp.z;
	mView(3, 1) = y;

	mView(0, 2) = mLook.x;
	mView(1, 2) = mLook.y;
	mView(2, 2) = mLook.z;
	mView(3, 2) = z;

	mView(0, 3) = 0.0f;
	mView(1, 3) = 0.0f;
	mView(2, 3) = 0.0f;
	mView(3, 3) = 1.0f;

	return XMLoadFloat4x4(&mView);
}

XMMATRIX FLight::GetProj()
{
	XMMATRIX lightProj = XMMatrixOrthographicOffCenterLH(-1080, 1080, -1920, 1920, 0, 200000);//P
	return lightProj;
}
