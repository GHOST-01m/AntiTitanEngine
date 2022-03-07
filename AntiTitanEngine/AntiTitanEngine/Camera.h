#pragma once
#include "stdafx.h"
#include "glm/glm/gtx/quaternion.hpp"

#ifndef CAMERA_H
#define CAMERA_H

class Camera
{
public:
	Camera();
	~Camera();

	//Get及Set世界空间下摄像机的位置
	DirectX::XMVECTOR GetPosition()const;
	DirectX::XMFLOAT3 GetPosition3f()const;
	glm::vec3 GetPositionVec3()const;

	void SetPosition(float x, float y, float z);
	void SetPosition(const DirectX::XMFLOAT3& v);
	void SetPosition(glm::vec3 v);

	//获取摄像机的基向量（观察空间的三个轴在世界空间下的表示）
	DirectX::XMVECTOR GetRight()const;
	DirectX::XMFLOAT3 GetRight3f()const;
	DirectX::XMVECTOR GetUp()const;
	DirectX::XMFLOAT3 GetUp3f()const;
	DirectX::XMVECTOR GetLook()const;
	DirectX::XMFLOAT3 GetLook3f()const;
	glm::vec3 GetRightVec3()const;
	glm::vec3 GetUpVec3()const;
	glm::vec3 GetLookVec3()const;


	//获取视锥体属性
	float GetNearZ()const;//近裁剪面距离
	float GetFarZ()const;//远裁剪面距离
	float GetAspect()const;//视口纵横比
	float GetFovY()const;//垂直视场角
	float GetFovX()const;//水平视场角

	//获取用观察空间坐标表示的近、远平面大小
	float GetNearWindowWidth()const;
	float GetNearWindowHeight()const;
	float GetFarWindowWidth()const;
	float GetFarWindowHeight()const;

	//赋值视锥体变量，设置投影矩阵(实现使用了内置函数XMMatrixPerspectiveFovLH)
	void SetLens(float fovY, float aspect, float zn, float zf);

	//准备观察矩阵数据
	void LookAt(DirectX::FXMVECTOR pos, DirectX::FXMVECTOR target, DirectX::FXMVECTOR worldUp);
	void LookAt(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& target, const DirectX::XMFLOAT3& up);
	void LookAt(glm::vec3 pos, glm::vec3 target, glm::vec3 up);

	//获取观察矩阵和投影矩阵
	DirectX::XMMATRIX GetView()const;
	DirectX::XMMATRIX GetProj()const;
	DirectX::XMFLOAT4X4 GetView4x4f()const;
	DirectX::XMFLOAT4X4 GetProj4x4f()const;
	glm::mat4 GetViewMat4()const;//直接给了单位矩阵，不确定对不对
	glm::mat4 GetProjMat4()const;//直接给了单位矩阵，不确定对不对

	//将摄像机按距离d左右平移（Strafe）或前后移动（Walk）
	void Strafe(float distance);//左右平移摄像机;glm已经更新进去
	void Walk(float distance);//前后推拉摄像机
	void Fly(float distance);//上下移动摄像机

	//旋转摄像机
	void Pitch(float angle);//Pitch方向
	void Yaw(float angle);//Yaw方向
	void Roll(float angle);//Roll方向

	//修改摄像机的位置和朝向后，调用此函数来重新构建观察矩阵
	void UpdateViewMatrix();

	//重写一个glm版DX的XMVector3TransformNormal函数
	glm::vec3 Vec3TransformNormal(glm::vec3 V, glm::mat4 M);

public:
	DirectX::XMFLOAT3 mPosition = { 0.0f, 0.0f, 0.0f };
	glm::vec3 mPositionVec3 = { 0 , 0 , 0 };//转换成glm版的同款数据


private:
	//摄像机坐标系（三个轴的基向量）在世界空间下的坐标
	DirectX::XMFLOAT3 mRight = { -1.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT3 mUp = { 0.0f, 0.0f, 1.0f };
	DirectX::XMFLOAT3 mLook = { 0.0f, 1.0f, 0.0f };
	//转换成glm版的同款数据
	glm::vec3 mRightVec3 = { -1.0f, 0.0f, 0.0f };
	glm::vec3 mUpVec3 = { 0.0f, 0.0f, 1.0f };
	glm::vec3 mLookVec3 = { 0.0f, 1.0f, 0.0f };

	//视锥体属性
	float mNearZ = 0.0f;
	float mFarZ = 0.0f;
	float mAspect = 0.0f;
	float mFovY = 0.0f;
	float mNearWindowHeight = 0.0f;
	float mFarWindowHeight = 0.0f;

	bool mViewDirty = true;

	//观察矩阵和投影矩阵
	DirectX::XMFLOAT4X4 mView = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 mProj = MathHelper::Identity4x4();
	//转换成glm版的同款数据
	glm::mat4 mViewMat4 = glm::mat4(1.0f);
	glm::mat4 mProjMat4 = glm::mat4(1.0f);

public:
	//相机移动速度
	float MoveSpeed=100.0;
};

#endif // !CAMERA_H