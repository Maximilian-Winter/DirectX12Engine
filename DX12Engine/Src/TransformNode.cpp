#include "TransformNode.h"

int TransformGraphRuntimeData::all_children_count() const
{
	return AllChildrenCount;
}

void TransformGraphRuntimeData::set_all_children_count(int all_children_count)
{
	AllChildrenCount = all_children_count;
}

int TransformGraphRuntimeData::root_children_count() const
{
	return RootChildrenCount;
}

void TransformGraphRuntimeData::set_root_children_count(int root_children_count)
{
	RootChildrenCount = root_children_count;
}

TransformGraphRuntimeData::TransformGraphRuntimeData(const TransformGraphRuntimeData& other): AllChildrenCount(other.AllChildrenCount),
	RootChildrenCount(other.RootChildrenCount)
{
}

TransformGraphRuntimeData::TransformGraphRuntimeData(TransformGraphRuntimeData&& other) noexcept: AllChildrenCount(other.AllChildrenCount),
	RootChildrenCount(other.RootChildrenCount)
{
}

TransformGraphRuntimeData& TransformGraphRuntimeData::operator=(const TransformGraphRuntimeData& other)
{
	if (this == &other)
		return *this;
	AllChildrenCount = other.AllChildrenCount;
	RootChildrenCount = other.RootChildrenCount;
	return *this;
}

TransformGraphRuntimeData& TransformGraphRuntimeData::operator=(TransformGraphRuntimeData&& other) noexcept
{
	if (this == &other)
		return *this;
	AllChildrenCount = other.AllChildrenCount;
	RootChildrenCount = other.RootChildrenCount;
	return *this;
}

TransformGraphRuntimeData::TransformGraphRuntimeData(int all_children_count, int root_children_count): AllChildrenCount(all_children_count),
	RootChildrenCount(root_children_count)
{
}

TransformGraphRuntimeData::~TransformGraphRuntimeData()
{

}

TransformNode::TransformNode(const TransformNode& other): m_TransformName(other.m_TransformName),
                                                          m_LocalPosition(other.m_LocalPosition),
                                                          m_LocalRotation(other.m_LocalRotation),
                                                          m_LocalScale(other.m_LocalScale),
                                                          m_AssimpTransform(other.m_AssimpTransform),
                                                          m_LocalToWorldSpaceMatrix(other.m_LocalToWorldSpaceMatrix),
                                                          m_WorldToLocalSpaceMatrix(other.m_WorldToLocalSpaceMatrix),
                                                          m_LocalToParentSpaceMatrix(other.m_LocalToParentSpaceMatrix),
                                                          m_ParentToLocalSpaceMatrix(other.m_ParentToLocalSpaceMatrix),
                                                          m_Parent(other.m_Parent),
                                                          m_Children(other.m_Children),
                                                          m_Components(other.m_Components),
                                                          m_LocalChildrenSize(other.m_LocalChildrenSize)
{
}

TransformNode::TransformNode(TransformNode&& other) noexcept: m_TransformName(std::move(other.m_TransformName)),
                                                              m_LocalPosition(std::move(other.m_LocalPosition)),
                                                              m_LocalRotation(std::move(other.m_LocalRotation)),
                                                              m_LocalScale(std::move(other.m_LocalScale)),
                                                              m_AssimpTransform(std::move(other.m_AssimpTransform)),
                                                              m_LocalToWorldSpaceMatrix(std::move(other.m_LocalToWorldSpaceMatrix)),
                                                              m_WorldToLocalSpaceMatrix(std::move(other.m_WorldToLocalSpaceMatrix)),
                                                              m_LocalToParentSpaceMatrix(std::move(other.m_LocalToParentSpaceMatrix)),
                                                              m_ParentToLocalSpaceMatrix(std::move(other.m_ParentToLocalSpaceMatrix)),
                                                              m_Parent(other.m_Parent),
                                                              m_Children(std::move(other.m_Children)),
                                                              m_Components(std::move(other.m_Components)),
                                                              m_LocalChildrenSize(other.m_LocalChildrenSize)
{
}

TransformNode& TransformNode::operator=(const TransformNode& other)
{
	if (this == &other)
		return *this;
	m_TransformName = other.m_TransformName;
	m_LocalPosition = other.m_LocalPosition;
	m_LocalRotation = other.m_LocalRotation;
	m_LocalScale = other.m_LocalScale;
	m_AssimpTransform = other.m_AssimpTransform;
	m_LocalToWorldSpaceMatrix = other.m_LocalToWorldSpaceMatrix;
	m_WorldToLocalSpaceMatrix = other.m_WorldToLocalSpaceMatrix;
	m_LocalToParentSpaceMatrix = other.m_LocalToParentSpaceMatrix;
	m_ParentToLocalSpaceMatrix = other.m_ParentToLocalSpaceMatrix;
	m_Parent = other.m_Parent;
	m_Children = other.m_Children;
	m_Components = other.m_Components;
	m_LocalChildrenSize = other.m_LocalChildrenSize;
	return *this;
}

TransformNode& TransformNode::operator=(TransformNode&& other) noexcept
{
	if (this == &other)
		return *this;
	m_TransformName = std::move(other.m_TransformName);
	m_LocalPosition = std::move(other.m_LocalPosition);
	m_LocalRotation = std::move(other.m_LocalRotation);
	m_LocalScale = std::move(other.m_LocalScale);
	m_AssimpTransform = std::move(other.m_AssimpTransform);
	m_LocalToWorldSpaceMatrix = std::move(other.m_LocalToWorldSpaceMatrix);
	m_WorldToLocalSpaceMatrix = std::move(other.m_WorldToLocalSpaceMatrix);
	m_LocalToParentSpaceMatrix = std::move(other.m_LocalToParentSpaceMatrix);
	m_ParentToLocalSpaceMatrix = std::move(other.m_ParentToLocalSpaceMatrix);
	m_Parent = other.m_Parent;
	m_Children = std::move(other.m_Children);
	m_Components = std::move(other.m_Components);
	m_LocalChildrenSize = other.m_LocalChildrenSize;
	return *this;
}

std::string TransformNode::TransFormName() const
{
	return m_TransformName;
}

void TransformNode::TransFormName(const std::string& m_transform_name)
{
	m_TransformName = m_transform_name;
}

void TransformNode::Rotate(double Pitch, double Yaw, double Roll)
{
	DirectX::XMStoreFloat4(&m_LocalRotation, DirectX::XMQuaternionMultiply(DirectX::XMLoadFloat4(&m_LocalRotation), DirectX::XMQuaternionRotationRollPitchYaw(Pitch, Yaw, Roll)));
	CalculateInternals();
}


TransformNode::TransformNode(std::string transformName)
	: m_TransformName(transformName)
	, m_LocalPosition(0.f, 0.f, 0.f)
	, m_LocalRotation()
	, m_LocalScale(1.f, 1.f, 1.f)
	, m_Parent(nullptr)
{
	DirectX::XMStoreFloat4(&m_LocalRotation, DirectX::XMQuaternionIdentity());
	CalculateInternals();
}

TransformNode::TransformNode(std::string transformName, TransformNode* parent)
	: m_TransformName(transformName)
	, m_LocalPosition(0.f, 0.f, 0.f)
	, m_LocalRotation()
	, m_LocalScale(1.f, 1.f, 1.f)
	, m_Parent(parent)
{
	if(m_Parent != nullptr)
	{
		m_Parent->AddLocalChild(this);
	}
	
	DirectX::XMStoreFloat4(&m_LocalRotation, DirectX::XMQuaternionIdentity());
	CalculateInternals();
}



TransformNode::TransformNode(std::string transformName, TransformNode* parent, DirectX::FXMVECTOR position, DirectX::FXMVECTOR quaternionRotation,
	DirectX::FXMVECTOR scale)
	: m_TransformName(transformName)
	, m_Parent(parent)
{
	if (m_Parent != nullptr)
	{
		m_Parent->AddLocalChild(this);
	}

	DirectX::XMStoreFloat3(&m_LocalPosition, position);
	DirectX::XMStoreFloat4(&m_LocalRotation, quaternionRotation);
	DirectX::XMStoreFloat3(&m_LocalScale, scale);
	CalculateInternals();
}

void TransformNode::CalculateInternals()
{
	/*	DirectX::XMMATRIX worldMatrix = DirectX::XMMatrixIdentity();

	const DirectX::XMMATRIX ScaleMatrix = DirectX::XMMatrixScaling(m_LocalScale.x, m_LocalScale.y, m_LocalScale.z);

	const DirectX::XMMATRIX RotationMatrix = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&m_LocalRotation));
	//RotationMatrix =  DirectX::XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&m_Rotation));

	const DirectX::XMMATRIX TranslateMatrix = DirectX::XMMatrixTranslation(m_LocalPosition.x, m_LocalPosition.y, m_LocalPosition.z);

	worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, ScaleMatrix);
	worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, RotationMatrix);
	worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, TranslateMatrix);
	if (m_Parent != nullptr)
	{
		worldMatrix = DirectX::XMMatrixMultiply(m_Parent->GetLocalToWorldSpaceTransformMatrix(), worldMatrix);
	}
	else
	{
		worldMatrix = DirectX::XMMatrixMultiply(DirectX::XMMatrixIdentity(), worldMatrix);
	}


	XMStoreFloat4x4(&m_LocalToWorldSpaceMatrix, worldMatrix);
	XMStoreFloat4x4(&m_WorldToLocalSpaceMatrix, DirectX::XMMatrixInverse(nullptr, worldMatrix));

if (fromLocal)
	{
		DirectX::XMMATRIX worldMatrix = DirectX::XMMatrixIdentity();

		const DirectX::XMMATRIX ScaleMatrix = DirectX::XMMatrixScaling(m_LocalScale.x, m_LocalScale.y, m_LocalScale.z);

		const DirectX::XMMATRIX RotationMatrix = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&m_LocalRotation));
		//RotationMatrix =  DirectX::XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&m_Rotation));

		const DirectX::XMMATRIX TranslateMatrix = DirectX::XMMatrixTranslation(m_LocalPosition.x, m_LocalPosition.y, m_LocalPosition.z);

		worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, ScaleMatrix);
		worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, RotationMatrix);
		worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, TranslateMatrix);
		if(m_Parent != nullptr)
		{
			worldMatrix = DirectX::XMMatrixMultiply(m_Parent->GetLocalToWorldSpaceTransformMatrix(), worldMatrix);
		}
		else
		{
			worldMatrix = DirectX::XMMatrixMultiply(DirectX::XMMatrixIdentity(), worldMatrix);
		}
		

		XMStoreFloat4x4(&m_LocalToWorldSpaceMatrix, worldMatrix);
		XMStoreFloat4x4(&m_WorldToLocalSpaceMatrix, DirectX::XMMatrixInverse(nullptr, worldMatrix));
		DirectX::XMStoreFloat3(&m_Position, DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&m_LocalPosition), worldMatrix));
		DirectX::XMStoreFloat4(&m_Rotation, DirectX::XMVector3Transform(DirectX::XMLoadFloat4(&m_LocalRotation), worldMatrix));
		DirectX::XMStoreFloat3(&m_Scale, DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&m_LocalScale), worldMatrix));
	}
	else
	{
		DirectX::XMMATRIX worldMatrix = DirectX::XMMatrixIdentity();

		const DirectX::XMMATRIX ScaleMatrix = DirectX::XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z);

		const DirectX::XMMATRIX RotationMatrix = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&m_Rotation));
		//RotationMatrix =  DirectX::XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&m_Rotation));

		const DirectX::XMMATRIX TranslateMatrix = DirectX::XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);

		worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, ScaleMatrix);
		worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, RotationMatrix);
		worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, TranslateMatrix);

		XMStoreFloat4x4(&m_LocalToWorldSpaceMatrix, worldMatrix);
		const DirectX::XMMATRIX worldToLocalMatrix = DirectX::XMMatrixInverse(nullptr, worldMatrix);
		XMStoreFloat4x4(&m_WorldToLocalSpaceMatrix, worldToLocalMatrix);
		DirectX::XMStoreFloat3(&m_LocalPosition, DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&m_Position), worldToLocalMatrix));
		DirectX::XMStoreFloat4(&m_LocalRotation, DirectX::XMVector3Transform(DirectX::XMLoadFloat4(&m_Rotation), worldToLocalMatrix));
		DirectX::XMStoreFloat3(&m_LocalScale, DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&m_Scale), worldToLocalMatrix));
	}
	*/
	//LocalToParent
	const DirectX::XMMATRIX ScaleMatrix = DirectX::XMMatrixScaling(m_LocalScale.x, m_LocalScale.y, m_LocalScale.z);

	const DirectX::XMMATRIX RotationMatrix = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&m_LocalRotation));

	const DirectX::XMMATRIX TranslateMatrix = DirectX::XMMatrixTranslation(m_LocalPosition.x, m_LocalPosition.y, m_LocalPosition.z);


	DirectX::XMStoreFloat4x4(&m_LocalToParentSpaceMatrix, ScaleMatrix * RotationMatrix * TranslateMatrix);

	//ParentToLocal
	DirectX::XMStoreFloat4x4(&m_ParentToLocalSpaceMatrix, DirectX::XMMatrixInverse(nullptr, ScaleMatrix * RotationMatrix * TranslateMatrix));

	//World to local
	if (m_Parent != nullptr)
	{
		const DirectX::XMMATRIX worldTolocal = m_Parent->GetWorldToLocalTransformMatrix() * GetParentToLocalTransformMatrix();
		DirectX::XMStoreFloat4x4(&m_WorldToLocalSpaceMatrix, worldTolocal);
	}
	else
	{
		const DirectX::XMMATRIX worldTolocal = GetParentToLocalTransformMatrix();
		DirectX::XMStoreFloat4x4(&m_WorldToLocalSpaceMatrix, worldTolocal);
	}

	//local to world

	if (m_Parent != nullptr)
	{
		const DirectX::XMMATRIX localToWorld = m_Parent->GetLocalToWorldSpaceTransformMatrix() * GetLocalToParentSpaceTransformMatrix();
		DirectX::XMStoreFloat4x4(&m_LocalToWorldSpaceMatrix, localToWorld);
	}
	else 
	{
		const DirectX::XMMATRIX localToWorld = GetLocalToParentSpaceTransformMatrix();
		DirectX::XMStoreFloat4x4(&m_LocalToWorldSpaceMatrix, localToWorld);
	}

	

	for (const auto element : m_Children)
	{
		element->CalculateInternals();
	}
}

void TransformNode::Destroy()
{
	for (const auto element : m_Children)
	{
		element->Destroy();
	}

	for (const auto element : m_Components)
	{
		element->Destroy();
	}

	delete this;
}



DirectX::XMMATRIX TransformNode::GetLocalToWorldSpaceTransformMatrix() const
{
	return DirectX::XMLoadFloat4x4(&m_LocalToWorldSpaceMatrix);
}

DirectX::XMMATRIX TransformNode::GetWorldToLocalTransformMatrix() const
{
	return DirectX::XMLoadFloat4x4(&m_WorldToLocalSpaceMatrix);
}

DirectX::XMMATRIX TransformNode::GetAssimpTransformMatrix() const
{
	return DirectX::XMLoadFloat4x4(&m_AssimpTransform);
}

DirectX::XMMATRIX TransformNode::GetAssimpAccumulatedTransformMatrix() const
{
	return DirectX::XMLoadFloat4x4(&m_AssimpAccumulatedTransform);
}

DirectX::XMMATRIX TransformNode::GetLocalToParentSpaceTransformMatrix() const
{
	return DirectX::XMLoadFloat4x4(&m_LocalToParentSpaceMatrix);
}

DirectX::XMMATRIX TransformNode::GetParentToLocalTransformMatrix() const
{
	return DirectX::XMLoadFloat4x4(&m_ParentToLocalSpaceMatrix);
}

DirectX::FXMVECTOR TransformNode::GetLocalPosition()
{
	return DirectX::XMLoadFloat3(&m_LocalPosition);
}

DirectX::FXMVECTOR TransformNode::GetWorldPosition()
{
	return DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&m_LocalPosition), GetLocalToWorldSpaceTransformMatrix());
}

DirectX::FXMVECTOR TransformNode::GetLocalRotation()
{
	return DirectX::XMLoadFloat4(&m_LocalRotation);
}

DirectX::FXMVECTOR TransformNode::GetWorldRotation()
{
	return DirectX::XMVector3Transform(DirectX::XMLoadFloat4(&m_LocalRotation), GetLocalToWorldSpaceTransformMatrix());
}

DirectX::FXMVECTOR TransformNode::GetLocalScale()
{
	return DirectX::XMLoadFloat3(&m_LocalScale);
}

DirectX::FXMVECTOR TransformNode::GetWorldScale()
{
	return DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&m_LocalScale), GetLocalToWorldSpaceTransformMatrix());
}

void TransformNode::SetLocalPosition(DirectX::FXMVECTOR position)
{
	DirectX::XMStoreFloat3(&m_LocalPosition, position);
	CalculateInternals();
}

void TransformNode::SetWorldPosition(DirectX::FXMVECTOR position)
{
	DirectX::XMStoreFloat3(&m_LocalPosition, DirectX::XMVector3Transform(position, GetWorldToLocalTransformMatrix()));
	CalculateInternals();
}

void TransformNode::SetLocalRotation(DirectX::FXMVECTOR rotation)
{
	DirectX::XMStoreFloat4(&m_LocalRotation, rotation);
	CalculateInternals();
}

void TransformNode::SetWorldRotation(DirectX::FXMVECTOR rotation)
{
	DirectX::XMStoreFloat4(&m_LocalRotation, DirectX::XMVector4Transform(rotation, GetWorldToLocalTransformMatrix()));
	CalculateInternals();
}

void TransformNode::SetLocalScale(DirectX::FXMVECTOR scale)
{
	DirectX::XMStoreFloat3(&m_LocalScale, scale);
	CalculateInternals();
}

void TransformNode::SetWorldScale(DirectX::FXMVECTOR scale)
{
	DirectX::XMStoreFloat3(&m_LocalScale, DirectX::XMVector3Transform(scale, GetWorldToLocalTransformMatrix()));
	CalculateInternals();
}

int TransformNode::GetLocalChildCount() const
{
	return m_Children.size();
}

TransformNode* TransformNode::GetLocalChild(int i)
{
	return m_Children[i];
}

void TransformNode::AddLocalChild(TransformNode* child)
{
	child->m_Parent = this;
	m_Children.push_back(child);
	m_LocalChildrenSize = m_Children.size();
	child->CalculateInternals();
}

void TransformNode::AddChild(TransformNode* parent, TransformNode* child)
{
	parent->AddLocalChild(child);
}

int TransformNode::GetComponentCount() const
{
	return m_Components.size();
}

EntityComponentBase* TransformNode::GetComponent(int i)
{
	return m_Components[i];
}

TransformNode* TransformNode::GetChild(std::string childName)
{
	for (const auto element : m_Children)
	{
		if(element->m_TransformName == childName)
		{
			return element;
		}
	}

	for (const auto element : m_Children)
	{
		return element->GetChild(childName);
	}

	return nullptr;
}

EntityComponentBase* TransformNode::GetComponent(std::string componentName)
{
	for (const auto element : m_Components)
	{
		if (element->GetComponentName() == componentName)
		{
			return element;
		}
	}
	return nullptr;
}

TransformNode* TransformNode::GetChild(int index)
{
	int indexCounter = 0;
	for (const auto element : m_Children)
	{
		if (indexCounter == index)
		{
			return element;
		}
		indexCounter++;
	}

	for (const auto element : m_Children)
	{
		return element->GetChild(index);
	}

	return nullptr;
}


void TransformNode::AddComponent(EntityComponentBase* component)
{
	m_Components.push_back(component);
}

void TransformNode::UpdateComponents(TransformGraphRuntimeData& transformGraphRuntimeData)
{
	transformGraphRuntimeData.set_all_children_count(transformGraphRuntimeData.all_children_count() + m_LocalChildrenSize);

	for (const auto element : m_Children)
	{
		element->UpdateComponents(transformGraphRuntimeData);
	}

	for (const auto element : m_Components)
	{
		element->Update();
	}
}

void TransformNode::RenderComponents()
{
	for (const auto element : m_Children)
	{
		element->RenderComponents();
	}

	for (const auto element : m_Components)
	{
		element->Render();
	}
}

TransformNode TransformNode::FromAssimpTransformMatrix(std::string transformName, TransformNode* parent,
	DirectX::XMMATRIX m)
{
	TransformNode node = TransformNode(transformName, parent);
	DirectX::XMStoreFloat4x4(&node.m_AssimpTransform, m);
	if(parent != nullptr)
	{
		DirectX::XMStoreFloat4x4(&node.m_AssimpAccumulatedTransform, parent->GetAssimpAccumulatedTransformMatrix() * m);
	}
	else
	{
		DirectX::XMStoreFloat4x4(&node.m_AssimpAccumulatedTransform, m);
	}
	return node;
}

TransformNode* TransformNode::FromAssimpTransformMatrixPointer(std::string transformName, TransformNode* parent,
	DirectX::XMMATRIX m)
{
	TransformNode* node = new TransformNode(transformName, parent);
	DirectX::XMStoreFloat4x4(&node->m_AssimpTransform, m);
	if (parent != nullptr)
	{
		DirectX::XMStoreFloat4x4(&node->m_AssimpAccumulatedTransform, parent->GetAssimpAccumulatedTransformMatrix() * m);
	}
	else
	{
		DirectX::XMStoreFloat4x4(&node->m_AssimpAccumulatedTransform, m);
	}
	return node;
}


TransformNode TransformNode::FromTransformMatrix(std::string transformName, TransformNode* parent, const DirectX::XMMATRIX m)
{
	DirectX::XMVECTOR pos;
	DirectX::XMVECTOR rotation;
	DirectX::XMVECTOR scale;
	DirectX::XMMatrixDecompose(&scale, &rotation, &pos, m);

	return TransformNode(transformName, parent, pos, rotation, scale);
}
