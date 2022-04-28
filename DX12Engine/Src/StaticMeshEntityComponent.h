#pragma once
#ifndef _STATICMESHTRANSFORMCOMPONENT_H_
#define _STATICMESHTRANSFORMCOMPONENT_H_
#include <string>
#include <map>

#include "../StaticMesh.h"
#include "../EntityComponentBase.h"

class Direct3D;
class StaticMeshEntityComponent : EntityComponentBase
{
public:
	StaticMeshEntityComponent(const StaticMeshEntityComponent& other);

	StaticMeshEntityComponent(StaticMeshEntityComponent&& other) noexcept;

	StaticMeshEntityComponent& operator=(const StaticMeshEntityComponent& other);

	StaticMeshEntityComponent& operator=(StaticMeshEntityComponent&& other) noexcept;

	StaticMeshEntityComponent(Direct3D* direct_3d, const std::string& component_name,
	                             TransformNode* transform_that_component_belongs_to, std::vector<StaticVertex> vertices, std::vector<int> indicies, std::vector<Subset> subsets);
	~StaticMeshEntityComponent();

	void Initialize(Direct3D* direct_3d) override;

	void Update() override;
	void Render() override;

	void Destroy() override;

	std::string GetComponentName() override;

private:

	StaticMesh* m_StaticMesh;
};

#endif
