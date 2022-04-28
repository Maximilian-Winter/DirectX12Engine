#include "EntityComponentBase.h"


EntityComponentBase::EntityComponentBase(const EntityComponentBase& other): m_ComponentName(other.m_ComponentName),
                                                                            m_TransformThatComponentBelongsTo(other.m_TransformThatComponentBelongsTo),
                                                                            m_Direct_3d(other.m_Direct_3d)
{
}

EntityComponentBase::EntityComponentBase(EntityComponentBase&& other) noexcept: m_ComponentName(std::move(other.m_ComponentName)),
	m_TransformThatComponentBelongsTo(other.m_TransformThatComponentBelongsTo),
	m_Direct_3d(other.m_Direct_3d)
{
}

EntityComponentBase& EntityComponentBase::operator=(const EntityComponentBase& other)
{
	if (this == &other)
		return *this;
	m_ComponentName = other.m_ComponentName;
	m_TransformThatComponentBelongsTo = other.m_TransformThatComponentBelongsTo;
	m_Direct_3d = other.m_Direct_3d;
	return *this;
}

EntityComponentBase& EntityComponentBase::operator=(EntityComponentBase&& other) noexcept
{
	if (this == &other)
		return *this;
	m_ComponentName = std::move(other.m_ComponentName);
	m_TransformThatComponentBelongsTo = other.m_TransformThatComponentBelongsTo;
	m_Direct_3d = other.m_Direct_3d;
	return *this;
}

EntityComponentBase::EntityComponentBase(std::string componentName, TransformNode* transformThatComponentBelongsTo, Direct3D* direct_3d)
	: m_ComponentName(componentName),
	m_TransformThatComponentBelongsTo(transformThatComponentBelongsTo),
	m_Direct_3d(direct_3d)
{
}

EntityComponentBase::~EntityComponentBase() = default;

void EntityComponentBase::Initialize(Direct3D* direct_3d)
{
	m_Direct_3d = direct_3d;
}

void EntityComponentBase::Update()
{
}

void EntityComponentBase::Render()
{
}

void EntityComponentBase::Destroy()
{
	delete this;
}

std::string EntityComponentBase::GetComponentName()
{
	return m_ComponentName;
}
