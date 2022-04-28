#include "StaticMeshEntityComponent.h"
#include "Direct3D.h"

StaticMeshEntityComponent::StaticMeshEntityComponent(const StaticMeshEntityComponent& other): EntityComponentBase(other),
	m_StaticMesh(other.m_StaticMesh)
{
}

StaticMeshEntityComponent::StaticMeshEntityComponent(StaticMeshEntityComponent&& other) noexcept: EntityComponentBase(std::move(other)),
	m_StaticMesh(other.m_StaticMesh)
{
}

StaticMeshEntityComponent& StaticMeshEntityComponent::operator=(const StaticMeshEntityComponent& other)
{
	if (this == &other)
		return *this;
	EntityComponentBase::operator =(other);
	m_StaticMesh = other.m_StaticMesh;
	return *this;
}

StaticMeshEntityComponent& StaticMeshEntityComponent::operator=(StaticMeshEntityComponent&& other) noexcept
{
	if (this == &other)
		return *this;
	EntityComponentBase::operator =(std::move(other));
	m_StaticMesh = other.m_StaticMesh;
	return *this;
}

StaticMeshEntityComponent::StaticMeshEntityComponent(Direct3D* direct_3d,
                                                           const std::string& component_name,
                                                           TransformNode* transform_that_component_belongs_to, std::vector<StaticVertex> vertices, std::vector<int> indicies,
                                                           std::vector<Subset> subsets): EntityComponentBase(component_name, transform_that_component_belongs_to,direct_3d)
{
	m_StaticMesh = new StaticMesh(direct_3d, vertices, indicies, subsets);
}

void StaticMeshEntityComponent::Initialize(Direct3D* direct_3d)
{
	EntityComponentBase::Initialize(direct_3d);
}

void StaticMeshEntityComponent::Update()
{
	EntityComponentBase::Update();
}

void StaticMeshEntityComponent::Render()
{
	m_Direct_3d->SetWVPCBV(m_TransformThatComponentBelongsTo->GetLocalToWorldSpaceTransformMatrix());
	m_StaticMesh->Render(m_Direct_3d);

	EntityComponentBase::Render();
}

void StaticMeshEntityComponent::Destroy()
{
	delete m_StaticMesh;
	EntityComponentBase::Destroy();
}

std::string StaticMeshEntityComponent::GetComponentName()
{
	return EntityComponentBase::GetComponentName();
}

StaticMeshEntityComponent::~StaticMeshEntityComponent()
= default;
