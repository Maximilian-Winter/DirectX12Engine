#include "StaticModelEntityComponent.h"

#include "Logger.h"
#include "SimpleMath.h"
#include "StaticMeshEntityComponent.h"


StaticModelEntityComponent::StaticModelEntityComponent(const StaticModelEntityComponent& other): EntityComponentBase(other),
	m_StaticMeshes(other.m_StaticMeshes),
	m_AABBWidth(other.m_AABBWidth),
	m_AABBHeight(other.m_AABBHeight),
	m_AABBDepth(other.m_AABBDepth)
{
}

StaticModelEntityComponent::StaticModelEntityComponent(StaticModelEntityComponent&& other) noexcept: EntityComponentBase(std::move(other)),
	m_StaticMeshes(std::move(other.m_StaticMeshes)),
	m_AABBWidth(other.m_AABBWidth),
	m_AABBHeight(other.m_AABBHeight),
	m_AABBDepth(other.m_AABBDepth)
{
}

StaticModelEntityComponent& StaticModelEntityComponent::operator=(const StaticModelEntityComponent& other)
{
	if (this == &other)
		return *this;
	EntityComponentBase::operator =(other);
	m_StaticMeshes = other.m_StaticMeshes;
	m_AABBWidth = other.m_AABBWidth;
	m_AABBHeight = other.m_AABBHeight;
	m_AABBDepth = other.m_AABBDepth;
	return *this;
}

StaticModelEntityComponent& StaticModelEntityComponent::operator=(StaticModelEntityComponent&& other) noexcept
{
	if (this == &other)
		return *this;
	EntityComponentBase::operator =(std::move(other));
	m_StaticMeshes = std::move(other.m_StaticMeshes);
	m_AABBWidth = other.m_AABBWidth;
	m_AABBHeight = other.m_AABBHeight;
	m_AABBDepth = other.m_AABBDepth;
	return *this;
}

StaticModelEntityComponent::StaticModelEntityComponent(const std::string& component_name,
                                                             TransformNode* transform_that_component_belongs_to, Direct3D* direct_3d,
                                                             const std::string& meshFileName, SceneGraph& sceneGraph): EntityComponentBase(component_name,
                                                                                                                                              transform_that_component_belongs_to, direct_3d), m_AABBWidth(0), m_AABBHeight(0),
                                                                                                                       m_AABBDepth(0)
{
	LoadStaticModel(meshFileName, sceneGraph);
}


void StaticModelEntityComponent::Initialize(Direct3D* direct_3d)
{
	EntityComponentBase::Initialize(direct_3d);
}

void StaticModelEntityComponent::Update()
{
	EntityComponentBase::Update();
}

void StaticModelEntityComponent::Render()
{
	EntityComponentBase::Render();
}

void StaticModelEntityComponent::Destroy()
{
	EntityComponentBase::Destroy();
}

std::string StaticModelEntityComponent::GetComponentName()
{
	return EntityComponentBase::GetComponentName();
}

void StaticModelEntityComponent::LoadStaticModel(std::string meshFilename, SceneGraph& sceneGraph)
{
	RemoveStaticModel();
	DataFileContainer MeshInputFile;
	DataFileContainer MaterialInputFile;
	MaterialInputFile.ClearDataContainer();
	MeshInputFile.ClearDataContainer();
	bool result = MeshInputFile.LoadDataContainerBinaryFile(meshFilename);
	if (!result)
	{
		Logger::Get()->Log("ModelLoadingError: ", "MeshFileNotFound!");
		return;
	}
	std::string materialLibraryFilename;
	result = MeshInputFile.GetStringValueByName("Model_Information", "MaterialLibrary", materialLibraryFilename);
	if (!result)
	{
		Logger::Get()->Log("ModelLoadingError", "MaterialLibraryNotFound");
		return;
	}


	result = MaterialManager::Get()->AddMaterialFile(materialLibraryFilename);
	if (!result)
	{
		Logger::Get()->Log("ModelLoadFailed", "MaterialLibraryNotFound");
		return;
	}

	std::map<int, TransformNode*> nodeIDToSceneGraphNode;
	for (int i = 0; i < MeshInputFile.GetNumberOfRootSections(); i++)
	{
		std::string sectionName;
		std::vector<int> sectionIndex;
		sectionIndex.push_back(i);
		if (MeshInputFile.GetSectionName(sectionIndex, sectionName) && sectionName != "Model_Information")
		{
			int numberOfMeshes = 0;
			result = MeshInputFile.GetIntValueByName(sectionName, "Number_of_Animated_Meshes", numberOfMeshes);
			if (!result)
			{
				Logger::Get()->Log("ModelLoadingError", "AttributeNumberOfMeshesNotFound");
				return;
			}

			int nodeId = 0;
			result = MeshInputFile.GetIntValueByName(sectionName, "Node_ID", nodeId);
			if (!result)
			{
				Logger::Get()->Log("ModelLoadingError", "AttributeNodeIDNotFound");
				return;
			}

			int parentNodeId = 0;
			result = MeshInputFile.GetIntValueByName(sectionName, "Parent_Node_ID", parentNodeId);
			if (!result)
			{
				Logger::Get()->Log("ModelLoadingError", "AttributeParentNodeIDNotFound");
				return;
			}

			std::string nodeName = "";
			result = MeshInputFile.GetStringValueByName(sectionName, "Node_Name", nodeName);
			if (!result)
			{
				Logger::Get()->Log("ModelLoadingError", "AttributeNodeNameNotFound");
				return;
			}

			std::vector<float> nodePos;
			result = MeshInputFile.GetFloatVectorByName(sectionName, "Node_Position", nodePos);
			if (!result)
			{
				Logger::Get()->Log("ModelLoadingError", "AttributeNode_PositionNotFound");
				return;
			}

			std::vector<float> nodeRot;
			result = MeshInputFile.GetFloatVectorByName(sectionName, "Node_Rotation", nodeRot);
			if (!result)
			{
				Logger::Get()->Log("ModelLoadingError", "AttributeNode_RotationNotFound");
				return;
			}

			std::vector<float> nodeScale;
			result = MeshInputFile.GetFloatVectorByName(sectionName, "Node_Scale", nodeScale);
			if (!result)
			{
				Logger::Get()->Log("ModelLoadingError", "AttributeNode_ScaleNotFound");
				return;
			}

			DirectX::SimpleMath::Vector3 position = DirectX::SimpleMath::Vector3(nodePos[0], nodePos[1], nodePos[2]);
			DirectX::SimpleMath::Quaternion rotation = DirectX::SimpleMath::Quaternion(nodeRot[0], nodeRot[1], nodeRot[2], nodeRot[3]);
			DirectX::SimpleMath::Vector3 scale = DirectX::SimpleMath::Vector3(nodeScale[0], nodeScale[1], nodeScale[2]);


			TransformNode* node;
			if (parentNodeId >= 0)
			{
				node = new TransformNode(nodeName, nodeIDToSceneGraphNode[parentNodeId], position, rotation, scale);
			}
			else
			{
				node = new TransformNode(nodeName, m_TransformThatComponentBelongsTo, position, rotation, scale);
			}

			nodeIDToSceneGraphNode[nodeId] = node;

			if (numberOfMeshes > 0)
			{
				for (int t = 0; t < numberOfMeshes; t++)
				{
					std::vector<float> vertices;
					std::vector<StaticVertex> vertexData;
					result = MeshInputFile.GetFloatVectorByName(sectionName + ".AnimatedMesh_" + std::to_string(t), "Vertices", vertices);
					if (!result)
					{
						Logger::Get()->Log("ModelLoadingError", "AttributeVerticesNotFound");
						return;
					}


					for (int j = 0; j < vertices.size(); j += 8)
					{
						vertexData.push_back(StaticVertex(vertices[j], vertices[j + 1], vertices[j + 2], vertices[j + 3], vertices[j + 4], vertices[j + 5], vertices[j + 6], vertices[j + 7]));
					}

					std::vector<int> indicies;
					result = MeshInputFile.GetIntVectorByName(sectionName + +".AnimatedMesh_" + std::to_string(t), "Indicies", indicies);
					if (!result)
					{
						Logger::Get()->Log("ModelLoadingError", "AttributeIndiciesNotFound");
						return;
					}

					std::vector<Subset> subsets;
					int numberOfSubsets = 0;

					result = MeshInputFile.GetIntValueByName(sectionName + ".AnimatedMesh_" + std::to_string(t), "Subset_Count", numberOfSubsets);
					if (!result)
					{
						Logger::Get()->Log("ModelLoadingError", "AttributeSubsetCountNotFound");
						return;
					}

					for (int g = 0; g < numberOfSubsets; g++)
					{
						std::string meshSubsetSection = ".AnimatedMesh_" + std::to_string(t) + "_Subset_" + std::to_string(g);
						std::string subsetMaterialName = "";
						int drawAmount = 0;
						int subsetStart = 0;

						result = MeshInputFile.GetStringValueByName(sectionName + meshSubsetSection, "Subset_Material_Name", subsetMaterialName);
						if (!result)
						{
							Logger::Get()->Log("ModelLoadingError", "NoSubset_Material_NameFound");
							return;
						}

						result = MeshInputFile.GetIntValueByName(sectionName + meshSubsetSection, "Subset_Draw_Amount", drawAmount);
						if (!result)
						{
							Logger::Get()->Log("ModelLoadingError", "NoSubset_Draw_AmountFound");
							return;
						}

						result = MeshInputFile.GetIntValueByName(sectionName + meshSubsetSection, "Subset_Start", subsetStart);
						if (!result)
						{
							Logger::Get()->Log("ModelLoadingError", "NoSubset_StartFound");
							return;
						}
						subsets.push_back(Subset(subsetMaterialName, subsetStart, drawAmount));
					}

					StaticMeshEntityComponent* mesh = new StaticMeshEntityComponent(m_Direct_3d, nodeName, node, vertexData, indicies, subsets);
					node->AddComponent(reinterpret_cast<EntityComponentBase*>(mesh));

				}
			}
			
			m_StaticMeshes.push_back(node);
		}

	}

	sceneGraph.AddRootLocalChild(m_TransformThatComponentBelongsTo);
}

void StaticModelEntityComponent::RemoveStaticModel()
{
	for (const auto element : m_StaticMeshes)
	{
		element->Destroy();
	}
}


StaticModelEntityComponent::~StaticModelEntityComponent()
{
}
