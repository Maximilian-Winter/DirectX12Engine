#ifndef _STATICMODEL_H_
#define _STATICMODEL_H_
#include <d3d12.h>
#include <SimpleMath.h>
#include <string>
#include <vector>
#include <wrl/client.h>

#include "Src/AnimatedModelEntityComponent.h"
#include "Src/MaterialManager.h"
#include "Src/SceneGraph.h"
#include "Src/StaticModelEntityComponent.h"


class Model
{
public:

	Model();
	Model(SceneGraph& sceneGraph, Direct3D* direct_3d, std::string& staticModelFilename, std::string& staticModelSceneName, std::string modelAnimationFile, bool isAnimated );
	Model(const Model&);
	~Model();

	void Destroy() const;

	void LoadModel(SceneGraph& sceneGraph, Direct3D* direct_3d, std::string& staticModelFilename, std::string& staticModelSceneName, std::string modelAnimationFile, bool isAnimated);
	AnimationStuff::Animation* GetAnimation(int i);
	TransformNode* GetModelRootSceneGraphNode();
private:
	float m_AABBWidth;
	float m_AABBHeight;
	float m_AABBDepth;

	AnimatedModelEntityComponent* m_AnimatedModelComponent;
	StaticModelEntityComponent* m_StaticModelComponent;
	TransformNode* m_ModelRootSceneGraphNode;
	std::string m_RendererComponentName;
};

#endif
