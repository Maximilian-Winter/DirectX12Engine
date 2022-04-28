#ifndef _CAMERA_H_
#define _CAMERA_H_


// Includes
#include <d3d11.h>
#include <DirectXMath.h>



class OldCamera
{
public:
	OldCamera();
	OldCamera(const OldCamera&);
	~OldCamera();

	void Initialze(float screenDepth, float screenNear, int screenWidth, int screenHeight);


	void SetPosition(DirectX::XMVECTOR);
	void SetCameraYaw(float);
	void SetCameraPitch(float);
	void SetMoveLeftRight(float);
	void SetMoveBackForward (float);

	DirectX::XMVECTOR GetPosition();
	DirectX::XMVECTOR GetLookAt();

	void Update();

	DirectX::XMMATRIX GetViewMatrix();
	DirectX::XMMATRIX GetProjectionMatrix();
private:

	DirectX::XMFLOAT3 m_Position;
	DirectX::XMFLOAT3 m_CameraForward;
	DirectX::XMFLOAT3 m_CameraRight;
	DirectX::XMFLOAT3 m_CameraUp;
	DirectX::XMFLOAT3 m_CameraTarget;

	float m_MoveLeftRight;
	float m_MoveBackForward;
	float m_CameraYaw ;
	float m_CameraPitch ;

	DirectX::XMFLOAT4X4 m_viewMatrix, m_baseViewMatrix, m_projectionMatrix;
};

#endif