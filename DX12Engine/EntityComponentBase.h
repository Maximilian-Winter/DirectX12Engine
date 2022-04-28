#pragma once
#ifndef _TRANSFORMCOMPONENTBASE_H_
#define _TRANSFORMCOMPONENTBASE_H_

#include "Src/MaterialManager.h"
#include "Src/TransformNode.h"

class TransformNode;

class EntityComponentBase
{
public:
	EntityComponentBase(const EntityComponentBase& other);

	EntityComponentBase(EntityComponentBase&& other) noexcept;

	EntityComponentBase& operator=(const EntityComponentBase& other);

	EntityComponentBase& operator=(EntityComponentBase&& other) noexcept;


	EntityComponentBase(std::string componentName, TransformNode* transformThatComponentBelongsTo, Direct3D* direct_3d);

protected:
	~EntityComponentBase();

public:
	virtual void Initialize(Direct3D* direct_3d);
	virtual void Update();
	virtual void Render();
	virtual void Destroy();

	virtual std::string GetComponentName();

protected:
	std::string m_ComponentName;
	TransformNode* m_TransformThatComponentBelongsTo;
	Direct3D* m_Direct_3d;
};


#endif
