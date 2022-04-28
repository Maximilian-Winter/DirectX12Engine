#pragma once
#ifndef _ANIMATEDMODELTRANSFORMCOMPONENT_H_
#define _ANIMATEDMODELTRANSFORMCOMPONENT_H_
#include <string>
#include <map>

#include "AnimatedMesh.h"
#include "SceneGraph.h"
#include "../StaticMesh.h"
#include "../EntityComponentBase.h"

class AnimatedModelEntityComponent : EntityComponentBase
{
public:
	AnimatedModelEntityComponent(const AnimatedModelEntityComponent& other)
		: EntityComponentBase(other),
		  m_Meshes(other.m_Meshes),
		  m_AABBWidth(other.m_AABBWidth),
		  m_AABBHeight(other.m_AABBHeight),
		  m_AABBDepth(other.m_AABBDepth)
	{
	}

	AnimatedModelEntityComponent(AnimatedModelEntityComponent&& other) noexcept
		: EntityComponentBase(std::move(other)),
		  m_Meshes(std::move(other.m_Meshes)),
		  m_AABBWidth(other.m_AABBWidth),
		  m_AABBHeight(other.m_AABBHeight),
		  m_AABBDepth(other.m_AABBDepth)
	{
	}

	AnimatedModelEntityComponent& operator=(const AnimatedModelEntityComponent& other)
	{
		if (this == &other)
			return *this;
		EntityComponentBase::operator =(other);
		m_Meshes = other.m_Meshes;
		m_AABBWidth = other.m_AABBWidth;
		m_AABBHeight = other.m_AABBHeight;
		m_AABBDepth = other.m_AABBDepth;
		return *this;
	}

	AnimatedModelEntityComponent& operator=(AnimatedModelEntityComponent&& other) noexcept
	{
		if (this == &other)
			return *this;
		EntityComponentBase::operator =(std::move(other));
		m_Meshes = std::move(other.m_Meshes);
		m_AABBWidth = other.m_AABBWidth;
		m_AABBHeight = other.m_AABBHeight;
		m_AABBDepth = other.m_AABBDepth;
		return *this;
	}

	AnimatedModelEntityComponent(const std::string& component_name,
	                             TransformNode* transform_that_component_belongs_to, Direct3D* direct_3d,
	                             const std::string& meshFileName,const std::string& meshAnimationFile, SceneGraph& sceneGraph);
	~AnimatedModelEntityComponent();

	void Initialize(Direct3D* direct_3d) override;
	void Update() override;
	void Render() override;
	void Destroy() override;
	std::string GetComponentName() override;

	void LoadModel(std::string meshFilename, SceneGraph& sceneGraph);
	void LoadAnimation(std::string meshAnimationFile);
	void RemoveStaticModel();
	AnimationStuff::Animation* GetAnimation(int i);
private:

	std::vector<AnimationStuff::Animation> m_Animations;
	std::vector<TransformNode*> m_Meshes;
	float m_AABBWidth;
	float m_AABBHeight;
	float m_AABBDepth;
};

#endif

