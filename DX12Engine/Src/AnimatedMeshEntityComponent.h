#pragma once
#ifndef _ANIMATEDMESHTRANSFORMCOMPONENT_H_
#define _ANIMATEDMESHTRANSFORMCOMPONENT_H_
#include <string>
#include <map>

#include "AnimatedMesh.h"
#include "../EntityComponentBase.h"

class Direct3D;
class AnimatedMeshEntityComponent : EntityComponentBase
{
public:
	AnimatedMeshEntityComponent(const AnimatedMeshEntityComponent& other)
		: EntityComponentBase(other),
		  m_StaticMesh(other.m_StaticMesh)
	{
	}

	AnimatedMeshEntityComponent(AnimatedMeshEntityComponent&& other) noexcept
		: EntityComponentBase(std::move(other)),
		  m_StaticMesh(other.m_StaticMesh)
	{
	}

	AnimatedMeshEntityComponent& operator=(const AnimatedMeshEntityComponent& other)
	{
		if (this == &other)
			return *this;
		EntityComponentBase::operator =(other);
		m_StaticMesh = other.m_StaticMesh;
		return *this;
	}

	AnimatedMeshEntityComponent& operator=(AnimatedMeshEntityComponent&& other) noexcept
	{
		if (this == &other)
			return *this;
		EntityComponentBase::operator =(std::move(other));
		m_StaticMesh = other.m_StaticMesh;
		return *this;
	}

	AnimatedMeshEntityComponent(Direct3D* direct_3d, const std::string& component_name,
	                            TransformNode* transform_that_component_belongs_to, std::vector<AnimatedVertex> vertices, std::vector<int> indicies, std::vector<Subset> subsets);
	~AnimatedMeshEntityComponent();

	void Initialize(Direct3D* direct_3d) override;

	void Update() override;
	void Render() override;

	void RenderMeshFromModelComponent();

	void Destroy() override;

	std::string GetComponentName() override;

private:

	AnimatedMesh* m_StaticMesh;
};

#endif

