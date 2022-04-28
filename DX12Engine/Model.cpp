#include "Model.h"

#include "Src/AnimatedModelEntityComponent.h"
#include "Src/DataFileContainer.h"
#include "Src/Logger.h"
#include "Src/StaticMeshEntityComponent.h"
#include "Src/StaticModelEntityComponent.h"

Model::Model()
{
}

Model::Model(SceneGraph& sceneGraph, Direct3D* direct_3d, std::string& staticModelFilename, std::string& staticModelSceneName, std::string modelAnimationFile, bool isAnimated): m_AABBWidth(0), m_AABBHeight(0), m_AABBDepth(0)
{
	LoadModel(sceneGraph, direct_3d, staticModelFilename, staticModelSceneName, modelAnimationFile, isAnimated);
}

Model::Model(const Model&)
{
}

Model::~Model()
{
}

void Model::Destroy() const
{
	m_ModelRootSceneGraphNode->Destroy();
}

void Model::LoadModel(SceneGraph& sceneGraph, Direct3D* direct_3d,
                            std::string& staticModelFilename, std::string& staticModelSceneName, std::string modelAnimationFile, bool isAnimated)
{
	if(isAnimated)
	{
		m_RendererComponentName = staticModelSceneName + "Renderer";
		m_ModelRootSceneGraphNode = TransformNode::FromAssimpTransformMatrixPointer(staticModelSceneName, nullptr, DirectX::XMMatrixIdentity());
		m_AnimatedModelComponent = new AnimatedModelEntityComponent(m_RendererComponentName, m_ModelRootSceneGraphNode, direct_3d, staticModelFilename, modelAnimationFile, sceneGraph);
		m_ModelRootSceneGraphNode->AddComponent(reinterpret_cast<EntityComponentBase*>(m_AnimatedModelComponent));
	}
	else
	{
		m_RendererComponentName = staticModelSceneName + "Renderer";
		m_ModelRootSceneGraphNode = TransformNode::FromAssimpTransformMatrixPointer(staticModelSceneName, nullptr, DirectX::XMMatrixIdentity());
		m_StaticModelComponent = new StaticModelEntityComponent(m_RendererComponentName, m_ModelRootSceneGraphNode, direct_3d, staticModelFilename, sceneGraph);
		m_ModelRootSceneGraphNode->AddComponent(reinterpret_cast<EntityComponentBase*>(m_StaticModelComponent));
	}
}

AnimationStuff::Animation* Model::GetAnimation(int i)
{
	return m_AnimatedModelComponent->GetAnimation(i);
}

TransformNode* Model::GetModelRootSceneGraphNode()
{
	return m_ModelRootSceneGraphNode;
}
