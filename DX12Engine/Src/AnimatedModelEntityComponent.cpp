#include "AnimatedModelEntityComponent.h"


#include "Logger.h"
#include "SimpleMath.h"
#include "AnimatedMeshEntityComponent.h"



AnimatedModelEntityComponent::AnimatedModelEntityComponent(const std::string& component_name,
	TransformNode* transform_that_component_belongs_to, Direct3D* direct_3d,
	const std::string& meshFileName, const std::string& meshAnimationFileName, SceneGraph& sceneGraph) : EntityComponentBase(component_name,
		transform_that_component_belongs_to, direct_3d), m_AABBWidth(0), m_AABBHeight(0),
	m_AABBDepth(0)
{
	LoadModel(meshFileName,sceneGraph);
	LoadAnimation(meshAnimationFileName);
}


void AnimatedModelEntityComponent::Initialize(Direct3D* direct_3d)
{
	EntityComponentBase::Initialize(direct_3d);
}

void AnimatedModelEntityComponent::Update()
{
	EntityComponentBase::Update();
}

void AnimatedModelEntityComponent::Render()
{
	m_Direct_3d->SetWVPCBV(m_TransformThatComponentBelongsTo->GetLocalToWorldSpaceTransformMatrix());
	for (auto element : m_Meshes)
	{
		if(element->GetComponentCount()> 0)
		{
			for(int i = 0; i < element->GetComponentCount(); i++)
			{
				if (AnimatedMeshEntityComponent* animated_mesh_entity_component = reinterpret_cast<AnimatedMeshEntityComponent*>(element->GetComponent(i)))
				{
					animated_mesh_entity_component->RenderMeshFromModelComponent();
				}
			}
		}
	}
	

	EntityComponentBase::Render();
}

void AnimatedModelEntityComponent::Destroy()
{
	EntityComponentBase::Destroy();
}

std::string AnimatedModelEntityComponent::GetComponentName()
{
	return EntityComponentBase::GetComponentName();
}

void AnimatedModelEntityComponent::LoadModel(std::string meshFilename, SceneGraph& sceneGraph)
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

			/*std::vector<float> matrixRow;
			DirectX::XMFLOAT4X4 assimpTrans;
			*MeshInputFile.GetFloatVectorByName(sectionName, "AssimpTransformRowOne", matrixRow);
			assimpTrans._11 = matrixRow[0];
			assimpTrans._12 = matrixRow[1];
			assimpTrans._13 = matrixRow[2];
			assimpTrans._14 = matrixRow[3];
			matrixRow.clear();

			MeshInputFile.GetFloatVectorByName(sectionName, "AssimpTransformRowTwo", matrixRow);
			assimpTrans._21 = matrixRow[0];
			assimpTrans._22 = matrixRow[1];
			assimpTrans._23 = matrixRow[2];
			assimpTrans._24 = matrixRow[3];
			matrixRow.clear();

			MeshInputFile.GetFloatVectorByName(sectionName, "AssimpTransformRowThree", matrixRow);
			assimpTrans._31 = matrixRow[0];
			assimpTrans._32 = matrixRow[1];
			assimpTrans._33 = matrixRow[2];
			assimpTrans._34 = matrixRow[3];
			matrixRow.clear();

			MeshInputFile.GetFloatVectorByName(sectionName, "AssimpTransformRowFour", matrixRow);
			assimpTrans._41 = matrixRow[0];
			assimpTrans._42 = matrixRow[1];
			assimpTrans._43 = matrixRow[2];
			assimpTrans._44 = matrixRow[3];
			matrixRow.clear();*/

			TransformNode* node;
			if (parentNodeId >= 0)
			{
				//node = TransformNode::FromAssimpTransformMatrixPointer(nodeName, nodeIDToSceneGraphNode[parentNodeId], DirectX::XMLoadFloat4x4(&assimpTrans));
				node = new TransformNode(nodeName, nodeIDToSceneGraphNode[parentNodeId], position, rotation, scale);
			}
			else
			{
				//node = TransformNode::FromAssimpTransformMatrixPointer(nodeName, nullptr, DirectX::XMLoadFloat4x4(&assimpTrans));
				node = new TransformNode(nodeName, m_TransformThatComponentBelongsTo, position, rotation, scale);
			}

			nodeIDToSceneGraphNode[nodeId] = node;

			if (numberOfMeshes > 0)
			{
				for (int t = 0; t < numberOfMeshes; t++)
				{
					std::vector<float> vertices;
					std::vector<AnimatedVertex> vertexData;
					result = MeshInputFile.GetFloatVectorByName(sectionName + ".AnimatedMesh_" + std::to_string(t), "Vertices", vertices);
					if (!result)
					{
						Logger::Get()->Log("ModelLoadingError", "AttributeVerticesNotFound");
						return;
					}


					for (int k = 0; k < vertices.size();  k+= 16)
					{
						vertexData.push_back(AnimatedVertex(vertices[k], vertices[k + 1], vertices[k + 2], vertices[k + 3], vertices[k + 4], vertices[k + 5], vertices[k + 6], vertices[k + 7], vertices[k + 8], vertices[k + 9], vertices[k + 10], vertices[k + 11], vertices[k + 12], vertices[k + 13], vertices[k + 14], vertices[k + 15]));
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

					AnimatedMeshEntityComponent* mesh = new AnimatedMeshEntityComponent(m_Direct_3d, nodeName, node, vertexData, indicies, subsets);
					node->AddComponent(reinterpret_cast<EntityComponentBase*>(mesh));

				}
			}

			m_Meshes.push_back(node);
		}

	}
	sceneGraph.AddRootLocalChild(m_TransformThatComponentBelongsTo);

}

void AnimatedModelEntityComponent::RemoveStaticModel()
{
	for (const auto element : m_Meshes)
	{
		element->Destroy();
	}
}

AnimationStuff::Animation* AnimatedModelEntityComponent::GetAnimation(int i)
{
	return &m_Animations[i];
}

void AnimatedModelEntityComponent::LoadAnimation(std::string meshAnimationFile)
{
	DataFileContainer AnimationInputFile;
		AnimationInputFile.ClearDataContainer();
	bool result = AnimationInputFile.LoadDataContainerBinaryFile(meshAnimationFile);
	if (!result)
	{
		Logger::Get()->Log("ModelLoadingError: ", "MeshFileNotFound!");
		return;
	}
	int numberOfAnimations = 0;
	AnimationInputFile.GetIntValueByName("Animations", "NumberOfAnimations", numberOfAnimations);

	for(int animationIndex = 0; animationIndex < numberOfAnimations ; animationIndex++)
	{
		DataContainerSection* animationSection = AnimationInputFile.GetLocalChildSectionFromRoot(0, animationIndex);
		std::string AnimationName = animationSection->SectionName;
		float AnimationDuration = 0;
		float AnimationTicksPerSecond = 0;
		animationSection->GetFloatVal("AnimationDuration", AnimationDuration);
		animationSection->GetFloatVal("AnimationTicksPerSecond", AnimationTicksPerSecond);
		std::vector<AnimationStuff::BoneAnimationKeyframes> BoneKeyframes;
		std::map<std::string, AnimationStuff::BoneInfo> BoneInfoMap;
		for (int boneKeyframeIndex = 0; boneKeyframeIndex < animationSection->ChildSections.size(); boneKeyframeIndex++)
		{
			std::vector<AnimationStuff::KeyPosition> keyPositons;
			std::vector<AnimationStuff::KeyRotation> keyRotations;
			std::vector<AnimationStuff::KeyScale> keyScales;
			DataContainerSection* boneSection = &animationSection->ChildSections[boneKeyframeIndex];

			DataContainerSection* positionKeysSection = boneSection->GetLocalChild("PositionKeys");
			if(positionKeysSection != nullptr)
			{
				for (int positionKeyIndex = 0; positionKeyIndex < positionKeysSection->ChildSections.size(); positionKeyIndex++)
				{
					float positionTimestamp;
					std::vector<float> position;
					DataContainerSection* positionKeySection = &positionKeysSection->ChildSections[positionKeyIndex];
					positionKeySection->GetFloatVal("TimeStamp", positionTimestamp);
					positionKeySection->GetFloatVector("Position", position);
					keyPositons.push_back(AnimationStuff::KeyPosition(DirectX::XMFLOAT3(position[0], position[1], position[2]), positionTimestamp));
				}

			}
			
			DataContainerSection* rotationKeysSection = boneSection->GetLocalChild("RotationKeys");
			if (rotationKeysSection != nullptr)
			{
				for (int positionKeyIndex = 0; positionKeyIndex < rotationKeysSection->ChildSections.size(); positionKeyIndex++)
				{
					float positionTimestamp;
					std::vector<float> position;
					DataContainerSection* rotationKeySection = &rotationKeysSection->ChildSections[positionKeyIndex];
					rotationKeySection->GetFloatVal("TimeStamp", positionTimestamp);
					rotationKeySection->GetFloatVector("Rotation", position);
					keyRotations.push_back(AnimationStuff::KeyRotation(DirectX::XMFLOAT4(position[0], position[1], position[2], position[3]), positionTimestamp));
				}
			}
		

			DataContainerSection* scaleKeysSection = boneSection->GetLocalChild("ScaleKeys");
			if (scaleKeysSection != nullptr)
			{
				for (int positionKeyIndex = 0; positionKeyIndex < scaleKeysSection->ChildSections.size(); positionKeyIndex++)
				{
					float positionTimestamp;
					std::vector<float> position;
					DataContainerSection* positionKeySection = &scaleKeysSection->ChildSections[positionKeyIndex];
					positionKeySection->GetFloatVal("TimeStamp", positionTimestamp);
					positionKeySection->GetFloatVector("Scale", position);
					keyScales.push_back(AnimationStuff::KeyScale(DirectX::XMFLOAT3(position[0], position[1], position[2]), positionTimestamp));
				}
			}
		
			int boneId = -1;
			boneSection->GetIntVal("BoneId", boneId);
			if(boneId == -1)
			{
				continue;
			}
			DataContainerSection* boneInfoMapSection = boneSection->GetLocalChild("BoneInfoMap");
			
			
			BoneInfoMap[boneSection->SectionName].BoneId = boneId;

			std::vector<float> matrixRow;

			boneInfoMapSection->GetFloatVector("BoneOffsetMatrixRowOne", matrixRow);
			BoneInfoMap[boneSection->SectionName].BoneOffsetMatrix._11 = matrixRow[0];
			BoneInfoMap[boneSection->SectionName].BoneOffsetMatrix._12 = matrixRow[1];
			BoneInfoMap[boneSection->SectionName].BoneOffsetMatrix._13 = matrixRow[2];
			BoneInfoMap[boneSection->SectionName].BoneOffsetMatrix._14 = matrixRow[3];
			matrixRow.clear();

			boneInfoMapSection->GetFloatVector("BoneOffsetMatrixRowTwo", matrixRow);
			BoneInfoMap[boneSection->SectionName].BoneOffsetMatrix._21 = matrixRow[0];
			BoneInfoMap[boneSection->SectionName].BoneOffsetMatrix._22 = matrixRow[1];
			BoneInfoMap[boneSection->SectionName].BoneOffsetMatrix._23 = matrixRow[2];
			BoneInfoMap[boneSection->SectionName].BoneOffsetMatrix._24 = matrixRow[3];
			matrixRow.clear();

			boneInfoMapSection->GetFloatVector("BoneOffsetMatrixRowThree", matrixRow);
			BoneInfoMap[boneSection->SectionName].BoneOffsetMatrix._31 = matrixRow[0];
			BoneInfoMap[boneSection->SectionName].BoneOffsetMatrix._32 = matrixRow[1];
			BoneInfoMap[boneSection->SectionName].BoneOffsetMatrix._33 = matrixRow[2];
			BoneInfoMap[boneSection->SectionName].BoneOffsetMatrix._34 = matrixRow[3];
			matrixRow.clear();

			boneInfoMapSection->GetFloatVector("BoneOffsetMatrixRowFour", matrixRow);
			BoneInfoMap[boneSection->SectionName].BoneOffsetMatrix._41 = matrixRow[0];
			BoneInfoMap[boneSection->SectionName].BoneOffsetMatrix._42 = matrixRow[1];
			BoneInfoMap[boneSection->SectionName].BoneOffsetMatrix._43 = matrixRow[2];
			BoneInfoMap[boneSection->SectionName].BoneOffsetMatrix._44 = matrixRow[3];
			matrixRow.clear();
			BoneKeyframes.push_back(AnimationStuff::BoneAnimationKeyframes(boneSection->SectionName, boneId, keyPositons, keyRotations, keyScales));
		}
		m_Animations.push_back(AnimationStuff::Animation(AnimationName, AnimationDuration, AnimationTicksPerSecond, BoneKeyframes, BoneInfoMap, m_Meshes[0]));
	}
}

AnimatedModelEntityComponent::~AnimatedModelEntityComponent()
{
	RemoveStaticModel();
}
