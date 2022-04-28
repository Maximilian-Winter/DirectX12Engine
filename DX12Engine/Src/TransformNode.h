#ifndef _TRANSFORM_H_
#define _TRANSFORM_H_

#include <DirectXMath.h>
#include <memory>
#include <vector>

#include "Logger.h"
#include "../EntityComponentBase.h"

class TransformGraphRuntimeData
{
public:
	[[nodiscard]] int all_children_count() const;

	void set_all_children_count(int all_children_count);

	[[nodiscard]] int root_children_count() const;

	void set_root_children_count(int root_children_count);

	TransformGraphRuntimeData(const TransformGraphRuntimeData& other);

	TransformGraphRuntimeData(TransformGraphRuntimeData&& other) noexcept;

	TransformGraphRuntimeData& operator=(const TransformGraphRuntimeData& other);

	TransformGraphRuntimeData& operator=(TransformGraphRuntimeData&& other) noexcept;

	TransformGraphRuntimeData(int all_children_count, int root_children_count);

	~TransformGraphRuntimeData();

private:
	int AllChildrenCount;
	int RootChildrenCount;
};

class EntityComponentBase;

class TransformNode
{
public:
	TransformNode(const TransformNode& other);

	TransformNode(TransformNode&& other) noexcept;

	TransformNode& operator=(const TransformNode& other);

	TransformNode& operator=(TransformNode&& other) noexcept;

	TransformNode(std::string transformName);
	TransformNode(std::string transformName, TransformNode* parent);
	TransformNode(std::string transformName, TransformNode* parent, DirectX::FXMVECTOR position, DirectX::FXMVECTOR quaternionRotation, DirectX::FXMVECTOR scale);
	~TransformNode();

	void Destroy();

	DirectX::XMMATRIX GetLocalToWorldSpaceTransformMatrix() const;
	DirectX::XMMATRIX GetWorldToLocalTransformMatrix() const;

	DirectX::XMMATRIX GetAssimpTransformMatrix() const;

	DirectX::XMMATRIX GetAssimpAccumulatedTransformMatrix() const;

	DirectX::XMMATRIX GetLocalToParentSpaceTransformMatrix() const;
	DirectX::XMMATRIX GetParentToLocalTransformMatrix() const;

	DirectX::FXMVECTOR GetLocalPosition();
	DirectX::FXMVECTOR GetWorldPosition();

	DirectX::FXMVECTOR GetLocalRotation();
	DirectX::FXMVECTOR GetWorldRotation();

	DirectX::FXMVECTOR GetLocalScale();
	DirectX::FXMVECTOR GetWorldScale();

	void SetLocalPosition(DirectX::FXMVECTOR position);
	void SetWorldPosition(DirectX::FXMVECTOR position);

	void SetLocalRotation(DirectX::FXMVECTOR rotation);
	void SetWorldRotation(DirectX::FXMVECTOR rotation);

	void SetLocalScale(DirectX::FXMVECTOR scale);
	void SetWorldScale(DirectX::FXMVECTOR scale);

	int GetLocalChildCount() const;
	int GetComponentCount() const;

	TransformNode* GetLocalChild(int i);
	EntityComponentBase* GetComponent(int i);
	TransformNode* GetChild(std::string childName);
	EntityComponentBase* GetComponent(std::string componentName);
	TransformNode* GetChild(int index);

	void AddLocalChild(TransformNode* child);
	static void AddChild(TransformNode* parent, TransformNode* child);
	void AddComponent(EntityComponentBase* component);

	void UpdateComponents(TransformGraphRuntimeData& transformGraphRuntimeData );
	void RenderComponents();

	static TransformNode FromAssimpTransformMatrix(std::string transformName, TransformNode* parent, DirectX::XMMATRIX m);

	static TransformNode* FromAssimpTransformMatrixPointer(std::string transformName, TransformNode* parent, DirectX::XMMATRIX m);

	static TransformNode FromTransformMatrix(std::string transformName, TransformNode* parent, const DirectX::XMMATRIX m);
	[[nodiscard]] std::string TransFormName() const;

	void TransFormName(const std::string& m_transform_name);

	void Rotate(double Pitch, double Yaw, double Roll);
private:

	void CalculateInternals();

	std::string m_TransformName;

	DirectX::XMFLOAT3 m_LocalPosition;
	DirectX::XMFLOAT4 m_LocalRotation;
	DirectX::XMFLOAT3 m_LocalScale;

	DirectX::XMFLOAT4X4 m_AssimpTransform;
	DirectX::XMFLOAT4X4 m_AssimpAccumulatedTransform;

	DirectX::XMFLOAT4X4 m_LocalToWorldSpaceMatrix;
	DirectX::XMFLOAT4X4 m_WorldToLocalSpaceMatrix;
	DirectX::XMFLOAT4X4 m_LocalToParentSpaceMatrix;
	DirectX::XMFLOAT4X4 m_ParentToLocalSpaceMatrix;


	TransformNode* m_Parent;
	std::vector<TransformNode*> m_Children;
	std::vector<EntityComponentBase*> m_Components;

	int m_LocalChildrenSize;

};

inline TransformNode::~TransformNode()
{
}

#endif
