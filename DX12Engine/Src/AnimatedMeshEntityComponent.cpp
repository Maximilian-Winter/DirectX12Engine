#include "AnimatedMeshEntityComponent.h"
#include "Direct3D.h"
#include "StaticMeshEntityComponent.h"


AnimatedMeshEntityComponent::AnimatedMeshEntityComponent(Direct3D* direct_3d,
                                                         const std::string& component_name,
                                                         TransformNode* transform_that_component_belongs_to, std::vector<AnimatedVertex> vertices, std::vector<int> indicies,
                                                         std::vector<Subset> subsets) : EntityComponentBase(component_name, transform_that_component_belongs_to, direct_3d)
{
	m_StaticMesh = new AnimatedMesh(direct_3d, vertices, indicies, subsets);
}

void AnimatedMeshEntityComponent::Initialize(Direct3D* direct_3d)
{
	EntityComponentBase::Initialize(direct_3d);
}

void AnimatedMeshEntityComponent::Update()
{
	EntityComponentBase::Update();
}

void AnimatedMeshEntityComponent::Render()
{
	EntityComponentBase::Render();
}

void AnimatedMeshEntityComponent::RenderMeshFromModelComponent()
{
	m_StaticMesh->Render(m_Direct_3d);
}

void AnimatedMeshEntityComponent::Destroy()
{
	delete m_StaticMesh;
	EntityComponentBase::Destroy();
}

std::string AnimatedMeshEntityComponent::GetComponentName()
{
	return EntityComponentBase::GetComponentName();
}

AnimatedMeshEntityComponent::~AnimatedMeshEntityComponent()
= default;
