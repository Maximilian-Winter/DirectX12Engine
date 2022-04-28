#pragma once
#ifndef _STATICMODELTRANSFORMCOMPONENT_H_
#define _STATICMODELTRANSFORMCOMPONENT_H_
#include <string>
#include <map>

#include "SceneGraph.h"
#include "../StaticMesh.h"
#include "../EntityComponentBase.h"

class StaticModelEntityComponent : EntityComponentBase
{
public:
	StaticModelEntityComponent(const StaticModelEntityComponent& other);

	StaticModelEntityComponent(StaticModelEntityComponent&& other) noexcept;

	StaticModelEntityComponent& operator=(const StaticModelEntityComponent& other);

	StaticModelEntityComponent& operator=(StaticModelEntityComponent&& other) noexcept;

	StaticModelEntityComponent(const std::string& component_name,
	                              TransformNode* transform_that_component_belongs_to, Direct3D* direct_3d,
	                             const std::string& meshFileName, SceneGraph& sceneGraph);
	~StaticModelEntityComponent();

	void Initialize(Direct3D* direct_3d) override;
	void Update() override;
	void Render() override;
	void Destroy() override;
	std::string GetComponentName() override;

	void LoadStaticModel(std::string meshFilename, SceneGraph& sceneGraph);
	void RemoveStaticModel();

private:

	std::vector<TransformNode*> m_StaticMeshes;
	float m_AABBWidth;
	float m_AABBHeight;
	float m_AABBDepth;
};

#endif
