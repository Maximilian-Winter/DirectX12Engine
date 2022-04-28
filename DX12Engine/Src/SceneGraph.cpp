#include "SceneGraph.h"


SceneGraph::SceneGraph(TransformNode* rootTransform): m_RootTransform(nullptr), m_SceneGraphRuntimeData(0, 0)
{
	m_RootTransform = new TransformNode("World");
}

SceneGraph::~SceneGraph()
{
}

void SceneGraph::Destroy()
{
	m_RootTransform->Destroy();
}

int SceneGraph::GetRootLocalChildrenCount() const
{
	return m_RootTransform->GetLocalChildCount();
}

TransformNode* SceneGraph::GetRootLocalChild(int index)
{
	return m_RootTransform->GetLocalChild(index);
}

void SceneGraph::AddRootLocalChild(TransformNode* pNode)
{
	m_RootTransform->AddLocalChild(pNode);
}

int SceneGraph::GetAllChildrenCount() const
{
	return m_SceneGraphRuntimeData.all_children_count();
}

TransformNode* SceneGraph::GetChild(int index)
{
	return m_RootTransform->GetChild(index);
}

void SceneGraph::AddChild(TransformNode* parentNode, TransformNode* childNode)
{
	parentNode->AddLocalChild(childNode);
}

TransformNode* SceneGraph::GetChild(std::string nodeName)
{
	return GetChild(nodeName);
}

TransformNode* SceneGraph::GetRootTransform()
{
	return m_RootTransform;
}

void SceneGraph::Update()
{
	m_RootTransform->UpdateComponents(m_SceneGraphRuntimeData);
}

void SceneGraph::Render()
{
	m_RootTransform->RenderComponents();
}
