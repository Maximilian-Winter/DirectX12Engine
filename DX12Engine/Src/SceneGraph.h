#pragma once
#ifndef _SCENEGRAPH_H_
#define _SCENEGRAPH_H_
#include <string>

#include "TransformNode.h"


class SceneGraph
{
public:
    SceneGraph(TransformNode* rootTransform);
    ~SceneGraph();

    void Destroy();

    int GetRootLocalChildrenCount() const;
    TransformNode* GetRootLocalChild(int index);
    void AddRootLocalChild(TransformNode* pNode);

    int GetAllChildrenCount() const;
    TransformNode* GetChild(int index);
    void AddChild(TransformNode* parentNode,TransformNode* childNode);

    TransformNode* GetChild(std::string nodeName);

    TransformNode* GetRootTransform();

    void Update();

    void Render();

private:
    TransformNode* m_RootTransform;
    TransformGraphRuntimeData m_SceneGraphRuntimeData;
};

#endif
