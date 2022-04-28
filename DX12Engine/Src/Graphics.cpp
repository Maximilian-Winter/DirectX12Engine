////////////////////////////////////////////////////////////////////////////////
// Filename: graphicsclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "Graphics.h"

#include "Input.h"
#include "MeshUtilities.h"


Graphics::Graphics(): m_WorldRootTransform("World"), m_SceneGraph(&m_WorldRootTransform)
{
}


Graphics::~Graphics()
{
	
}


bool Graphics::Initialize(HINSTANCE hinstance, int screenWidth, int screenHeight, HWND hwnd)
{
	bool result;

	m_Camera.Initialze(1000.0f, 0.1f, screenWidth, screenHeight);

	// Create the Direct3D object.
	m_Direct3D = new Direct3D();
	if(!m_Direct3D)
	{
		return false;
	}

	// Initialize the Direct3D object.
	result = m_Direct3D->Initialize(&m_Animator, &m_StaticModel1, &m_Camera, screenWidth, screenHeight, hwnd, VSYNC_ENABLED, FULL_SCREEN);
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize Direct3D.", L"Error", MB_OK);
		return false;
	}
	m_Camera.SetPosition(DirectX::XMVectorSet(0.0f, 2.0f, -10.0f, 0.0f));
	m_Camera.Update();

	TextureManager::Initialize(L"");
	TextureManager::DefaultTextures::Get()->Init(m_Direct3D);

	MaterialManager::Get()->Initialize(m_Direct3D);


	std::string fileName = "HipHopDancing.smo";
	std::string animationFileName = "HipHopDancing.anim";
	std::string sceneName = "CharacterKit";
	m_StaticModel1.LoadModel(m_SceneGraph, m_Direct3D, fileName, sceneName, animationFileName, true);
	AnimationStuff::Animation* animation = m_StaticModel1.GetAnimation(0);
	animation->SetTicksPerSecond(24);
	m_Animator.PlayAnimation(animation);

	DirectX::XMFLOAT3 pos = DirectX::XMFLOAT3(0.0f, 0.0f, 00.0f);
	m_StaticModel1.GetModelRootSceneGraphNode()->SetLocalPosition(DirectX::XMLoadFloat3(&pos));
	//m_StaticModel1.GetModelRootSceneGraphNode()->Rotate(0.0f, 0.0f, 90.0f);
	pos = DirectX::XMFLOAT3(0.01f, 0.01f, 0.01f);
	m_StaticModel1.GetModelRootSceneGraphNode()->SetLocalScale(DirectX::XMLoadFloat3(&pos));

	/*fileName = "ModularCharacters.smo";
	sceneName = "ModularCharacters2";*/
/*	m_StaticModel2.LoadModel(m_SceneGraph, m_Direct3D, fileName, sceneName);

	m_StaticModel3.LoadModel(m_SceneGraph, m_Direct3D, fileName, sceneName);
	m_StaticModel4.LoadModel(m_SceneGraph, m_Direct3D, fileName, sceneName);
	m_StaticModel5.LoadModel(m_SceneGraph, m_Direct3D, fileName, sceneName);
	m_StaticModel6.LoadModel(m_SceneGraph, m_Direct3D, fileName, sceneName);
	m_StaticModel7.LoadModel(m_SceneGraph, m_Direct3D, fileName, sceneName);
	m_StaticModel8.LoadModel(m_SceneGraph, m_Direct3D, fileName, sceneName);
	m_StaticModel9.LoadModel(m_SceneGraph, m_Direct3D, fileName, sceneName);
	m_StaticModel10.LoadModel(m_SceneGraph, m_Direct3D, fileName, sceneName);
	m_StaticModel11.LoadModel(m_SceneGraph, m_Direct3D, fileName, sceneName);
	m_StaticModel12.LoadModel(m_SceneGraph, m_Direct3D, fileName, sceneName);
	
	DirectX::XMFLOAT3 pos = DirectX::XMFLOAT3(0.0f, 0.0f, 00.0f);
	m_StaticModel1.GetModelRootSceneGraphNode()->SetLocalPosition(DirectX::XMLoadFloat3(&pos));

	pos = DirectX::XMFLOAT3(0.01f, 0.01f, 0.01f);
	m_StaticModel1.GetModelRootSceneGraphNode()->SetLocalScale(DirectX::XMLoadFloat3(&pos));*/

	/*pos = DirectX::XMFLOAT3(0.0f, 0.0f, 3.0f);
	m_StaticModel2.GetModelRootSceneGraphNode()->SetWorldPosition(DirectX::XMLoadFloat3(&pos));

	pos = DirectX::XMFLOAT3(0.01f, 0.01f, 0.01f);
	m_StaticModel2.GetModelRootSceneGraphNode()->SetLocalScale(DirectX::XMLoadFloat3(&pos));

	pos = DirectX::XMFLOAT3(0.0f, 0.0f, 6.0f);
	m_StaticModel3.GetModelRootSceneGraphNode()->SetWorldPosition(DirectX::XMLoadFloat3(&pos));

	pos = DirectX::XMFLOAT3(0.01f, 0.01f, 0.01f);
	m_StaticModel3.GetModelRootSceneGraphNode()->SetLocalScale(DirectX::XMLoadFloat3(&pos));

	pos = DirectX::XMFLOAT3(0.0f, 0.0f, 9.0f);
	m_StaticModel4.GetModelRootSceneGraphNode()->SetWorldPosition(DirectX::XMLoadFloat3(&pos));

	pos = DirectX::XMFLOAT3(0.01f, 0.01f, 0.01f);
	m_StaticModel4.GetModelRootSceneGraphNode()->SetLocalScale(DirectX::XMLoadFloat3(&pos));

	pos = DirectX::XMFLOAT3(0.0f, 0.0f, 12.0f);
	m_StaticModel5.GetModelRootSceneGraphNode()->SetWorldPosition(DirectX::XMLoadFloat3(&pos));

	pos = DirectX::XMFLOAT3(0.01f, 0.01f, 0.01f);
	m_StaticModel5.GetModelRootSceneGraphNode()->SetLocalScale(DirectX::XMLoadFloat3(&pos));

	pos = DirectX::XMFLOAT3(0.0f, 0.0f, 15.0f);
	m_StaticModel6.GetModelRootSceneGraphNode()->SetWorldPosition(DirectX::XMLoadFloat3(&pos));

	pos = DirectX::XMFLOAT3(0.01f, 0.01f, 0.01f);
	m_StaticModel6.GetModelRootSceneGraphNode()->SetLocalScale(DirectX::XMLoadFloat3(&pos));

	pos = DirectX::XMFLOAT3(0.0f, 0.0f, 18.0f);
	m_StaticModel7.GetModelRootSceneGraphNode()->SetWorldPosition(DirectX::XMLoadFloat3(&pos));

	pos = DirectX::XMFLOAT3(0.01f, 0.01f, 0.01f);
	m_StaticModel7.GetModelRootSceneGraphNode()->SetLocalScale(DirectX::XMLoadFloat3(&pos));

	pos = DirectX::XMFLOAT3(0.0f, 0.0f, 21.0f);
	m_StaticModel8.GetModelRootSceneGraphNode()->SetWorldPosition(DirectX::XMLoadFloat3(&pos));

	pos = DirectX::XMFLOAT3(0.01f, 0.01f, 0.01f);
	m_StaticModel8.GetModelRootSceneGraphNode()->SetLocalScale(DirectX::XMLoadFloat3(&pos));

	pos = DirectX::XMFLOAT3(0.0f, 0.0f, 24.0f);
	m_StaticModel9.GetModelRootSceneGraphNode()->SetWorldPosition(DirectX::XMLoadFloat3(&pos));

	pos = DirectX::XMFLOAT3(0.01f, 0.01f, 0.01f);
	m_StaticModel9.GetModelRootSceneGraphNode()->SetLocalScale(DirectX::XMLoadFloat3(&pos));

	pos = DirectX::XMFLOAT3(0.01f, 0.01f, 0.01f);
	m_StaticModel10.GetModelRootSceneGraphNode()->SetLocalScale(DirectX::XMLoadFloat3(&pos));

	pos = DirectX::XMFLOAT3(0.0f, 0.0f, 27.0f);
	m_StaticModel10.GetModelRootSceneGraphNode()->SetWorldPosition(DirectX::XMLoadFloat3(&pos));

	pos = DirectX::XMFLOAT3(0.0f, 0.0f, 30.0f);
	m_StaticModel11.GetModelRootSceneGraphNode()->SetWorldPosition(DirectX::XMLoadFloat3(&pos));

	pos = DirectX::XMFLOAT3(0.01f, 0.01f, 0.01f);
	m_StaticModel11.GetModelRootSceneGraphNode()->SetLocalScale(DirectX::XMLoadFloat3(&pos));

	pos = DirectX::XMFLOAT3(0.0f, 0.0f, 33.0f);
	m_StaticModel12.GetModelRootSceneGraphNode()->SetWorldPosition(DirectX::XMLoadFloat3(&pos));
	pos = DirectX::XMFLOAT3(0.01f, 0.01f, 0.01f);
	m_StaticModel12.GetModelRootSceneGraphNode()->SetLocalScale(DirectX::XMLoadFloat3(&pos));
	pos = DirectX::XMFLOAT3(0.02f, 0.02f, 0.02f);
//	m_StaticModel2.GetModelRootSceneGraphNode()->SetWorldScale(DirectX::XMLoadFloat3(&pos));*/
	return true;
}



void Graphics::Shutdown()
{
	delete m_Direct3D;
	m_Direct3D = nullptr;

	m_SceneGraph.Destroy();

	return;
}


bool Graphics::Frame()
{
	bool result;

	float WalkingSpeedFactor = 0.5;


	if (Input::Get()->IsKeyDown(VK_SHIFT))
	{
		WalkingSpeedFactor = 50.0;
	}

	// Get the fps for calculating the speed factor.
	WalkingSpeedFactor *= GameTime::Get()->DeltaTime();

	// Handle the input.
	if (Input::Get()->IsKeyDown('A'))
	{
		m_MoveLeftRight -= WalkingSpeedFactor;
	}

	// Handle the input.
	if (Input::Get()->IsKeyDown('D'))
	{
		m_MoveLeftRight += WalkingSpeedFactor;
	}

	// Handle the input.
	if (Input::Get()->IsKeyDown('W'))
	{
		m_MoveBackForward += WalkingSpeedFactor;
	}

	// Handle the input.
	if (Input::Get()->IsKeyDown('S'))
	{
		m_MoveBackForward -= WalkingSpeedFactor;
	}

	if (Input::Get()->IsKeyDown(37))
	{
		m_Yaw -= WalkingSpeedFactor;
	}

	// Handle the input.
	if (Input::Get()->IsKeyDown(39))
	{
		m_Yaw += WalkingSpeedFactor;
	}

	// Handle the input.
	if (Input::Get()->IsKeyDown(38))
	{
		m_Roll -= WalkingSpeedFactor;
	}

	// Handle the input.
	if (Input::Get()->IsKeyDown(40))
	{
		m_Roll += WalkingSpeedFactor;
	}
	m_Camera.SetCameraYaw(m_Yaw);
	m_Camera.SetCameraPitch(m_Roll);
	m_Camera.SetMoveBackForward(m_MoveBackForward);
	m_Camera.SetMoveLeftRight(m_MoveLeftRight);
	m_MoveBackForward = 0.0f;
	m_MoveLeftRight = 0.0f;
	m_Camera.Update();
	m_SceneGraph.Update();
	m_Animator.UpdateAnimation(GameTime::Get()->DeltaTime());
	
	// Update the graphics scene.
	result = Render();
	
	if(!result)
	{
		return false;
	}

	return true;
}


bool Graphics::Render()
{
	bool result;
	m_Direct3D->RendererStart();
	m_SceneGraph.Render();
	result = m_Direct3D->Render();
	if(!result)
	{
		return false;
	}

	return true;
}
