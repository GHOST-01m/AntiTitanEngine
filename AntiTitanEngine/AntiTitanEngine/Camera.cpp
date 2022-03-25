#include "stdafx.h"
#include "Camera.h"

using namespace DirectX;

Camera::Camera()
{
	SetLens(0.25f * MathHelper::Pi, 1.0f, 1.0f, 100000.0f);
}

Camera::~Camera()
{
}

XMVECTOR Camera::GetPosition()const
{
	return XMLoadFloat3(&mPosition);
}

XMFLOAT3 Camera::GetPosition3f()const
{
	return mPosition;
}

glm::vec3 Camera::GetPositionVec3()const {
	return mPositionVec3;
};

void Camera::SetPosition(float x, float y, float z)
{
	mPosition = XMFLOAT3(x, y, z);
	mPositionVec3 = { x, y, z };
	mViewDirty = true;
}

void Camera::SetPosition(const XMFLOAT3& v)
{
	mPosition = v;
	mPositionVec3 = { v.x, v.y, v.z };

	mViewDirty = true;
}

void Camera::SetPosition(glm::vec3 v) {
	mPositionVec3 = v;
	mViewDirty = true;
};


XMVECTOR Camera::GetRight()const
{
	return XMLoadFloat3(&mRight);
}

XMFLOAT3 Camera::GetRight3f()const
{
	return mRight;
}

glm::vec3 Camera::GetRightVec3()const {

	return mRightVec3;
};


XMVECTOR Camera::GetUp()const
{
	return XMLoadFloat3(&mUp);
}

XMFLOAT3 Camera::GetUp3f()const
{
	return mUp;
}


glm::vec3 Camera::GetUpVec3()const {
	return mUpVec3;
};


XMVECTOR Camera::GetLook()const
{
	return XMLoadFloat3(&mLook);
}

XMFLOAT3 Camera::GetLook3f()const
{
	return mLook;
}

glm::vec3 Camera::GetLookVec3()const {
	return mLookVec3;
};

float Camera::GetNearZ()const
{
	return mNearZ;
}

float Camera::GetFarZ()const
{
	return mFarZ;
}

float Camera::GetAspect()const
{
	return mAspect;
}

float Camera::GetFovY()const
{
	return mFovY;
}

float Camera::GetFovX()const
{
	float halfWidth = 0.5f * GetNearWindowWidth();
	return 2.0f * atan(halfWidth / mNearZ);
}

float Camera::GetNearWindowWidth()const
{
	return mAspect * mNearWindowHeight;
}

float Camera::GetNearWindowHeight()const
{
	return mNearWindowHeight;
}

float Camera::GetFarWindowWidth()const
{
	return mAspect * mFarWindowHeight;
}

float Camera::GetFarWindowHeight()const
{
	return mFarWindowHeight;
}

void Camera::SetLens(float fovY, float aspect, float zn, float zf)
{
	// cache properties
	mFovY = fovY;
	mAspect = aspect;
	mNearZ = zn;
	mFarZ = zf;

	mNearWindowHeight = 2.0f * mNearZ * tanf(0.5f * mFovY);
	mFarWindowHeight = 2.0f * mFarZ * tanf(0.5f * mFovY);

	XMMATRIX P = XMMatrixPerspectiveFovLH(mFovY, mAspect, mNearZ, mFarZ);
	XMStoreFloat4x4(&mProj, P);

	glm::mat4 Pmat4 = glm::perspectiveFovLH(mFovY, mAspect * mNearWindowHeight, mNearWindowHeight, mNearZ, mFarZ);
	mProjMat4 = Pmat4;
}

//准备观察矩阵数据，计算观察矩阵
void Camera::LookAt(FXMVECTOR pos, FXMVECTOR target, FXMVECTOR worldUp)
{
	XMVECTOR L = XMVector3Normalize(XMVectorSubtract(target, pos));
	XMVECTOR R = XMVector3Normalize(XMVector3Cross(worldUp, L));
	XMVECTOR U = XMVector3Cross(L, R);

	XMStoreFloat3(&mPosition, pos);
	XMStoreFloat3(&mLook, L);
	XMStoreFloat3(&mRight, R);
	XMStoreFloat3(&mUp, U);

	mViewDirty = true;
}
//准备观察矩阵数据，计算观察矩阵
void Camera::LookAt(const XMFLOAT3& pos, const XMFLOAT3& target, const XMFLOAT3& up)
{
	XMVECTOR P = XMLoadFloat3(&pos);
	XMVECTOR T = XMLoadFloat3(&target);
	XMVECTOR U = XMLoadFloat3(&up);

	LookAt(P, T, U);

	mViewDirty = true;
}

void Camera::LookAt(glm::vec3 pos, glm::vec3 target, glm::vec3 up) {

	glm::vec3 Lvec3 = glm::normalize(target - pos);
	glm::vec3 Rvec3 = glm::normalize(glm::cross(up, Lvec3));
	glm::vec3 Uvec3 = glm::cross(Lvec3, Rvec3);

	mPositionVec3 = pos;
	mLookVec3 = Lvec3;
	mRightVec3 = Rvec3;
	mUpVec3 = Uvec3;

	mViewDirty = true;

};


XMMATRIX Camera::GetView()const
{
	//assert(!mViewDirty);
	return XMLoadFloat4x4(&mView);
}

XMMATRIX Camera::GetProj()const
{
	return XMLoadFloat4x4(&mProj);
}

XMFLOAT4X4 Camera::GetView4x4f()const
{
	//assert(!mViewDirty);
	return mView;
}

XMFLOAT4X4 Camera::GetProj4x4f()const
{
	return mProj;
}

glm::mat4 Camera::GetViewMat4()const {//直接给了单位矩阵，不确定对不对
	return mViewMat4;
};
glm::mat4 Camera::GetProjMat4()const {//直接给了单位矩阵，不确定对不对
	return mProjMat4;
};

void Camera::Strafe(float d)//right方向向量上平移，即左右平移
{
	// mPosition += d*mRight
	XMVECTOR s = XMVectorReplicate(d);
	XMVECTOR r = XMLoadFloat3(&mRight);
	XMVECTOR p = XMLoadFloat3(&mPosition);
	XMStoreFloat3(&mPosition, XMVectorMultiplyAdd(s, r, p));

	glm::vec3 Svec3 = {d,d,d};
	glm::vec3 Rvec3 = mRightVec3;
	glm::vec3 Pvec3 = mPositionVec3;
	mPositionVec3 = Svec3 * Rvec3 + Pvec3;


	mViewDirty = true;
}

void Camera::Walk(float d)//look方向向量上平移，即前后平移
{
	// mPosition += d*mLook
	XMVECTOR s = XMVectorReplicate(d);
	XMVECTOR l = XMLoadFloat3(&mLook);
	XMVECTOR p = XMLoadFloat3(&mPosition);
	XMStoreFloat3(&mPosition, XMVectorMultiplyAdd(s, l, p));

	glm::vec3 Svec3 = { d,d,d };
	glm::vec3 Lvec3 = mLookVec3;
	glm::vec3 Pvec3 = mPositionVec3;
	mPositionVec3 = Svec3 * Lvec3 + Pvec3;

	mViewDirty = true;
}

void Camera::Fly(float d) //up方向向量上平移，即上下平移
{
	// mPosition += d*mUp
	XMVECTOR s = XMVectorReplicate(d);
	XMVECTOR u = XMLoadFloat3(&mUp);
	XMVECTOR p = XMLoadFloat3(&mPosition);
	XMStoreFloat3(&mPosition, XMVectorMultiplyAdd(s, u, p));

	glm::vec3 Svec3 = { d,d,d };
	glm::vec3 Uvec3 = mUpVec3;
	glm::vec3 Pvec3 = mPositionVec3;
	mPositionVec3 = Svec3 * Uvec3 + Pvec3;

	mViewDirty = true;
}

void Camera::Pitch(float angle)
{
	// Rotate up and look vector about the right vector.

	XMMATRIX R = XMMatrixRotationAxis(XMLoadFloat3(&mRight), angle);
	XMStoreFloat3(&mUp, XMVector3TransformNormal(XMLoadFloat3(&mUp), R));
	XMStoreFloat3(&mLook, XMVector3TransformNormal(XMLoadFloat3(&mLook), R));


	glm::mat4 RMat4 = glm::identity<glm::mat4>();
	RMat4 = glm::rotate(RMat4, angle, mUpVec3);
	mRightVec3 = Vec3TransformNormal(mRightVec3, RMat4);
	mUpVec3 = Vec3TransformNormal(mUpVec3, RMat4);
	mLookVec3 = Vec3TransformNormal(mLookVec3, RMat4);

	mViewDirty = true;
}

void Camera::Yaw(float angle)
{
	// Rotate the basis vectors about the world y-axis.

	XMMATRIX R = XMMatrixRotationZ(angle);
	XMStoreFloat3(&mRight, XMVector3TransformNormal(XMLoadFloat3(&mRight), R));
	XMStoreFloat3(&mUp, XMVector3TransformNormal(XMLoadFloat3(&mUp), R));
	XMStoreFloat3(&mLook, XMVector3TransformNormal(XMLoadFloat3(&mLook), R));

	glm::mat4 RMat4 = glm::identity<glm::mat4>();
	RMat4 = glm::rotate(RMat4, angle, mRightVec3);
	mRightVec3 = Vec3TransformNormal(mRightVec3, RMat4);
	mUpVec3 = Vec3TransformNormal(mUpVec3, RMat4);
	mLookVec3 = Vec3TransformNormal(mLookVec3, RMat4);

	mViewDirty = true;
}

void Camera::Roll(float angle) {

	XMMATRIX R = XMMatrixRotationY(angle);
	XMStoreFloat3(&mRight, XMVector3TransformNormal(XMLoadFloat3(&mRight), R));
	XMStoreFloat3(&mUp, XMVector3TransformNormal(XMLoadFloat3(&mUp), R));
	XMStoreFloat3(&mLook, XMVector3TransformNormal(XMLoadFloat3(&mLook), R));

	glm::mat4 RMat4 = glm::identity<glm::mat4>();
	RMat4 = glm::rotate(RMat4, angle, mLookVec3);
	mRightVec3 = Vec3TransformNormal(mRightVec3, RMat4);
	mUpVec3 = Vec3TransformNormal(mUpVec3, RMat4);
	mLookVec3 = Vec3TransformNormal(mLookVec3, RMat4);

	mViewDirty = true;
};//Roll方向


void Camera::UpdateViewMatrix()
{
	if (mViewDirty)
	{
		XMVECTOR R = XMLoadFloat3(&mRight);
		XMVECTOR U = XMLoadFloat3(&mUp);
		XMVECTOR L = XMLoadFloat3(&mLook);
		XMVECTOR P = XMLoadFloat3(&mPosition);
		glm::vec3 RVec3 = mRightVec3;
		glm::vec3 UVec3 = mUpVec3;
		glm::vec3 LVec3 = mLookVec3;
		glm::vec3 PVec3 = mPositionVec3;


		// Keep camera's axes orthogonal to each other and of unit length.
		L = XMVector3Normalize(L);
		U = XMVector3Normalize(XMVector3Cross(L, R));
		LVec3 = glm::normalize(LVec3);
		UVec3 = glm::normalize(glm::cross(LVec3,RVec3));

		// U, L already ortho-normal, so no need to normalize cross product.
		R = XMVector3Cross(U, L);
		RVec3 = glm::cross(UVec3, LVec3);

		// Fill in the view matrix entries.
		float x = -XMVectorGetX(XMVector3Dot(P, R));
		float y = -XMVectorGetX(XMVector3Dot(P, U));
		float z = -XMVectorGetX(XMVector3Dot(P, L));
		float VM_x = -glm::dot(mPositionVec3, RVec3);
		float VM_y = -glm::dot(mPositionVec3, UVec3);
		float VM_z = -glm::dot(mPositionVec3, LVec3);


		XMStoreFloat3(&mRight, R);
		XMStoreFloat3(&mUp, U);
		XMStoreFloat3(&mLook, L);
		mRightVec3 = RVec3;
		mUpVec3 = UVec3;
		mLookVec3 = LVec3;

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

		//填观察矩阵glm
		float viewArray[16];
		viewArray[0] = mRightVec3.x;
		viewArray[1] = mRightVec3.y;
		viewArray[2] = mRightVec3.z;
		viewArray[3] = VM_x;

		viewArray[4] = mUpVec3.x;
		viewArray[5] = mUpVec3.y;
		viewArray[6] = mUpVec3.z;
		viewArray[7] = VM_y;

		viewArray[8] = mLookVec3.x;
		viewArray[9] = mLookVec3.y;
		viewArray[10] = mLookVec3.z;
		viewArray[11] = VM_z;

		viewArray[12] = 0.0f;
		viewArray[13] = 0.0f;
		viewArray[14] = 0.0f;
		viewArray[15] = 1.0f;

		mViewMat4 = glm::make_mat4(viewArray);

		mViewDirty = false;
	}
}

glm::vec3 Camera::Vec3TransformNormal(glm::vec3 V, glm::mat4 M) {

	glm::vec3 Z = { V.z,V.z,V.z };
	glm::vec3 Y = { V.y,V.y,V.y };
	glm::vec3 X = { V.x,V.x,V.x };

	glm::vec3 Result = Z * (M[2][0],M[2][1],M[2][2]);
	Result = Y * (M[1][0], M[1][1], M[1][2]) + Result;
	Result = X * (M[0][0], M[0][1], M[0][2]) + Result;

	return Result;
};
