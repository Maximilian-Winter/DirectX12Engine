#ifndef _GRAPHICS_H_
#define _GRAPHICS_H_

#include "OldCamera.h"
#include "Direct3D.h"
#include "../Model.h"

const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = false;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.1f;


class Graphics
{
public:
	Graphics();
	~Graphics();

	bool Initialize(HINSTANCE hinstance, int, int, HWND);
	void Shutdown();
	bool Frame();

private:
	bool Render();

private:
	AnimationStuff::Animator m_Animator;
	Direct3D* m_Direct3D;
	TransformNode m_WorldRootTransform;
	SceneGraph m_SceneGraph;
	Model m_StaticModel1;
	Model m_StaticModel2;
	Model m_StaticModel3;
	Model m_StaticModel4;
	Model m_StaticModel5;
	Model m_StaticModel6;
	Model m_StaticModel7;
	Model m_StaticModel8;
	Model m_StaticModel9;
	Model m_StaticModel10;
	Model m_StaticModel11;
	Model m_StaticModel12;
	OldCamera m_Camera;
	//FlyingFPSCamera m_CameraController;
	float m_MoveLeftRight;
	float m_MoveBackForward;
	float m_Roll = 0.0f;
	float m_Yaw = 0.0f;
};

#endif