////////////////////////////////////////////////////////////////////////////////
// Filename: cameraclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "OldCamera.h"


OldCamera::OldCamera()
{
	m_Position = DirectX::XMFLOAT3(0.0, 0.0, 0.0);
	m_CameraYaw = 0.0f;
	m_CameraPitch = 0.0f;
	m_MoveLeftRight = 0.0f;
	m_MoveBackForward = 0.0f;

	m_CameraForward = DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f);
	m_CameraRight = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_CameraUp = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_CameraTarget = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
}

OldCamera::OldCamera(const OldCamera& other)
{
}

OldCamera::~OldCamera()
{
}

void OldCamera::Initialze(float screenDepth, float screenNear, int screenWidth, int screenHeight)
{
	// Setup the projection matrix.
	float fieldOfView = DirectX::XM_PIDIV4;
	float screenAspect = screenWidth / screenHeight;

	// Create the projection matrix for 3D rendering.
	DirectX::XMStoreFloat4x4(&m_projectionMatrix, DirectX::XMMatrixPerspectiveFovLH(fieldOfView, screenAspect, screenNear, screenDepth));


}

DirectX::XMMATRIX OldCamera::GetProjectionMatrix()
{
	return DirectX::XMLoadFloat4x4(&m_projectionMatrix);
}

void OldCamera::SetPosition(DirectX::XMVECTOR Position)
{
	DirectX::XMStoreFloat3(&m_Position ,Position);
	return;
}

void OldCamera::SetCameraYaw(float CamYaw)
{
	m_CameraYaw = CamYaw;

	return;
}

void OldCamera::SetCameraPitch(float CamPitch)
{
	m_CameraPitch = CamPitch;

	return;
}

void OldCamera::SetMoveLeftRight(float MoveLeftRight)
{
	m_MoveLeftRight = MoveLeftRight;

	return;
}

void OldCamera::SetMoveBackForward(float MoveBackForward)
{
	m_MoveBackForward = MoveBackForward;

	return;
}

DirectX::XMVECTOR OldCamera::GetPosition()
{
	return DirectX::XMLoadFloat3(&m_Position);
}

void OldCamera::Update()
{
	DirectX::XMVECTOR DefaultForward = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	DirectX::XMVECTOR DefaultRight = DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);

	DirectX::XMVECTOR camForward = DirectX::XMLoadFloat3(&m_CameraForward);
	DirectX::XMVECTOR camRight = DirectX::XMLoadFloat3(&m_CameraRight);
	DirectX::XMVECTOR camUp = DirectX::XMLoadFloat3(&m_CameraUp);
	DirectX::XMVECTOR camTarget = DirectX::XMLoadFloat3(&m_CameraTarget); 

	DirectX::XMVECTOR camPosition = DirectX::XMLoadFloat3(&m_Position);

	DirectX::XMMATRIX camRotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(m_CameraPitch, m_CameraYaw, 0);
	camTarget = DirectX::XMVector3TransformCoord(DefaultForward, camRotationMatrix );
	camTarget = DirectX::XMVector3Normalize(camTarget);

	DirectX::XMStoreFloat3(&m_CameraTarget, camTarget);
	/* FPS like movement. ()
	DirectX::XMMATRIX RotateYTempMatrix;
	RotateYTempMatrix = DirectX::XMMatrixRotationY(m_CameraYaw);
	*/
	camRight = DirectX::XMVector3TransformCoord(DefaultRight, camRotationMatrix);
	camUp = DirectX::XMVector3TransformCoord(camUp, camRotationMatrix);
	camForward = DirectX::XMVector3TransformCoord(DefaultForward, camRotationMatrix);

	camRight = DirectX::XMVectorScale(camRight, m_MoveLeftRight);
	camPosition = DirectX::XMVectorAdd(camPosition, camRight);
	camForward = DirectX::XMVectorScale(camForward, m_MoveBackForward);
	camPosition = DirectX::XMVectorAdd(camPosition, camForward);

	m_MoveLeftRight = 0.0f;
	m_MoveBackForward = 0.0f;

	camTarget = DirectX::XMVectorAdd(camTarget, camPosition);
	DirectX::XMMATRIX camView = DirectX::XMMatrixLookAtLH( camPosition, camTarget, camUp );
	
	DirectX::XMStoreFloat3(&m_Position, camPosition);
	DirectX::XMStoreFloat4x4(&m_viewMatrix, camView);
	return;

}

DirectX::XMMATRIX OldCamera::GetViewMatrix()
{
	return DirectX::XMLoadFloat4x4(&m_viewMatrix);
}

DirectX::XMVECTOR OldCamera::GetLookAt()
{
	return DirectX::XMLoadFloat3(&m_CameraTarget);
}