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

	//Get��Set����ռ����������λ��
	DirectX::XMVECTOR GetPosition()const;
	DirectX::XMFLOAT3 GetPosition3f()const;
	glm::vec3 GetPositionVec3()const;

	void SetPosition(float x, float y, float z);
	void SetPosition(const DirectX::XMFLOAT3& v);
	void SetPosition(glm::vec3 v);

	//��ȡ������Ļ��������۲�ռ��������������ռ��µı�ʾ��
	DirectX::XMVECTOR GetRight()const;
	DirectX::XMFLOAT3 GetRight3f()const;
	DirectX::XMVECTOR GetUp()const;
	DirectX::XMFLOAT3 GetUp3f()const;
	DirectX::XMVECTOR GetLook()const;
	DirectX::XMFLOAT3 GetLook3f()const;
	glm::vec3 GetRightVec3()const;
	glm::vec3 GetUpVec3()const;
	glm::vec3 GetLookVec3()const;


	//��ȡ��׶������
	float GetNearZ()const;//���ü������
	float GetFarZ()const;//Զ�ü������
	float GetAspect()const;//�ӿ��ݺ��
	float GetFovY()const;//��ֱ�ӳ���
	float GetFovX()const;//ˮƽ�ӳ���

	//��ȡ�ù۲�ռ������ʾ�Ľ���Զƽ���С
	float GetNearWindowWidth()const;
	float GetNearWindowHeight()const;
	float GetFarWindowWidth()const;
	float GetFarWindowHeight()const;

	//��ֵ��׶�����������ͶӰ����(ʵ��ʹ�������ú���XMMatrixPerspectiveFovLH)
	void SetLens(float fovY, float aspect, float zn, float zf);

	//׼���۲��������
	void LookAt(DirectX::FXMVECTOR pos, DirectX::FXMVECTOR target, DirectX::FXMVECTOR worldUp);
	void LookAt(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& target, const DirectX::XMFLOAT3& up);
	void LookAt(glm::vec3 pos, glm::vec3 target, glm::vec3 up);

	//��ȡ�۲�����ͶӰ����
	DirectX::XMMATRIX GetView()const;
	DirectX::XMMATRIX GetProj()const;
	DirectX::XMFLOAT4X4 GetView4x4f()const;
	DirectX::XMFLOAT4X4 GetProj4x4f()const;
	glm::mat4 GetViewMat4()const;//ֱ�Ӹ��˵�λ���󣬲�ȷ���Բ���
	glm::mat4 GetProjMat4()const;//ֱ�Ӹ��˵�λ���󣬲�ȷ���Բ���

	//�������������d����ƽ�ƣ�Strafe����ǰ���ƶ���Walk��
	void Strafe(float distance);//����ƽ�������;glm�Ѿ����½�ȥ
	void Walk(float distance);//ǰ�����������
	void Fly(float distance);//�����ƶ������

	//��ת�����
	void Pitch(float angle);//Pitch����
	void Yaw(float angle);//Yaw����
	void Roll(float angle);//Roll����

	//�޸��������λ�úͳ���󣬵��ô˺��������¹����۲����
	void UpdateViewMatrix();

	//��дһ��glm��DX��XMVector3TransformNormal����
	glm::vec3 Vec3TransformNormal(glm::vec3 V, glm::mat4 M);

public:
	DirectX::XMFLOAT3 mPosition = { 0.0f, 0.0f, 0.0f };
	glm::vec3 mPositionVec3 = { 0 , 0 , 0 };//ת����glm���ͬ������


private:
	//���������ϵ��������Ļ�������������ռ��µ�����
	DirectX::XMFLOAT3 mRight = { -1.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT3 mUp = { 0.0f, 0.0f, 1.0f };
	DirectX::XMFLOAT3 mLook = { 0.0f, 1.0f, 0.0f };
	//ת����glm���ͬ������
	glm::vec3 mRightVec3 = { -1.0f, 0.0f, 0.0f };
	glm::vec3 mUpVec3 = { 0.0f, 0.0f, 1.0f };
	glm::vec3 mLookVec3 = { 0.0f, 1.0f, 0.0f };

	//��׶������
	float mNearZ = 0.0f;
	float mFarZ = 0.0f;
	float mAspect = 0.0f;
	float mFovY = 0.0f;
	float mNearWindowHeight = 0.0f;
	float mFarWindowHeight = 0.0f;

	bool mViewDirty = true;

	//�۲�����ͶӰ����
	DirectX::XMFLOAT4X4 mView = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 mProj = MathHelper::Identity4x4();
	//ת����glm���ͬ������
	glm::mat4 mViewMat4 = glm::mat4(1.0f);
	glm::mat4 mProjMat4 = glm::mat4(1.0f);

public:
	//����ƶ��ٶ�
	float MoveSpeed=100.0;
};

#endif // !CAMERA_H