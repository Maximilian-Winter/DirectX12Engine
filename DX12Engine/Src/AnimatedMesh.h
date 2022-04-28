#ifndef _ANIMATEDMESH_H_
#define _ANIMATEDMESH_H_
#include <d3d12.h>
#include <DirectXCollision.h>
#include <directxmath.h>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <wrl/client.h>

#include "TransformNode.h"


class AnimatedMesh;

namespace MeshUtilities
{
	struct MeshGeometry;
}

struct Subset;
class Direct3D;

namespace AnimationStuff
{
	struct BoneInfo
	{
		BoneInfo();

		BoneInfo(const BoneInfo& other);

		BoneInfo(BoneInfo&& other) noexcept;

		BoneInfo& operator=(const BoneInfo& other);

		BoneInfo& operator=(BoneInfo&& other) noexcept;

		/*id is index in finalBoneMatrices*/
		int BoneId;

		/*offset matrix transforms vertex from model space to bone space*/
		DirectX::XMFLOAT4X4 BoneOffsetMatrix;

	};


	struct KeyPosition
	{
		DirectX::XMFLOAT3 position;
		float timeStamp;
	};

	struct KeyRotation
	{
		DirectX::XMFLOAT4 orientation;
		float timeStamp;
	};

	struct KeyScale
	{
		DirectX::XMFLOAT3 scale;
		float timeStamp;
	};

	class BoneAnimationKeyframes
	{
	public:
		BoneAnimationKeyframes(const std::string& name, int ID, std::vector<KeyPosition> keyPositions, std::vector<KeyRotation> keyRotations, std::vector<KeyScale> keyScales)
			:
			m_Name(name),
			m_ID(ID)
		{
			DirectX::XMStoreFloat4x4(&m_LocalTransform, DirectX::XMMatrixIdentity());

			m_NumPositions = keyPositions.size();
			m_Positions = keyPositions;

			m_NumRotations = keyRotations.size();
			m_Rotations = keyRotations;

			m_NumScalings = keyScales.size();
			m_Scales = keyScales;
		}

		void Update(float animationTime)
		{
			DirectX::XMMATRIX translation = InterpolatePosition(animationTime);
			DirectX::XMMATRIX rotation = InterpolateRotation(animationTime);
			DirectX::XMMATRIX scale = InterpolateScaling(animationTime);
			DirectX::XMStoreFloat4x4(&m_LocalTransform, scale * rotation * translation);
		}
		DirectX::XMMATRIX GetLocalTransform() { return DirectX::XMLoadFloat4x4(&m_LocalTransform); }
		std::string GetBoneName() const { return m_Name; }
		int GetBoneID() { return m_ID; }



		int GetPositionIndex(float animationTime)
		{
			for (int index = 0; index < m_NumPositions - 1; ++index)
			{
				if (animationTime < m_Positions[index + 1].timeStamp)
					return index;
			}
			return -1;
		}

		int GetRotationIndex(float animationTime)
		{
			for (int index = 0; index < m_NumRotations - 1; ++index)
			{
				if (animationTime < m_Rotations[index + 1].timeStamp)
					return index;
			}
			return -1;
		}

		int GetScaleIndex(float animationTime)
		{
			for (int index = 0; index < m_NumScalings - 1; ++index)
			{
				if (animationTime < m_Scales[index + 1].timeStamp)
					return index;
			}
			return -1;
		}


	private:

		float GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime)
		{
			float scaleFactor = 0.0f;
			float midWayLength = animationTime - lastTimeStamp;
			float framesDiff = nextTimeStamp - lastTimeStamp;
			scaleFactor = midWayLength / framesDiff;
			return scaleFactor;
		}

		DirectX::XMMATRIX InterpolatePosition(float animationTime)
		{
			if (1 == m_NumPositions)
				return DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&m_Positions[0].position));

			int p0Index = GetPositionIndex(animationTime);
			int p1Index = p0Index + 1;
			if(p0Index == -1)
			{
				return DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f);
			}
			else
			{
				float scaleFactor = GetScaleFactor(m_Positions[p0Index].timeStamp,
					m_Positions[p1Index].timeStamp, animationTime);
				const DirectX::XMVECTOR finalPosition = DirectX::XMVectorLerp(DirectX::XMLoadFloat3(&m_Positions[p0Index].position), DirectX::XMLoadFloat3(&m_Positions[p1Index].position)
					, scaleFactor);
				return  DirectX::XMMatrixTranslationFromVector(finalPosition);
			}
		}

		DirectX::XMMATRIX InterpolateRotation(float animationTime)
		{
			if (1 == m_NumRotations)
			{
				auto rotation = DirectX::XMVector4Normalize(DirectX::XMLoadFloat4(&m_Rotations[0].orientation));
				return DirectX::XMMatrixRotationQuaternion (rotation);
			}

			int p0Index = GetRotationIndex(animationTime);
			int p1Index = p0Index + 1;

			if (p0Index == -1)
			{
				return DirectX::XMMatrixRotationQuaternion(DirectX::XMQuaternionIdentity());
			}
			else
			{
				float scaleFactor = GetScaleFactor(m_Rotations[p0Index].timeStamp,
					m_Rotations[p1Index].timeStamp, animationTime);
				DirectX::XMVECTOR finalRotation = DirectX::XMQuaternionSlerp(DirectX::XMLoadFloat4(&m_Rotations[p0Index].orientation), DirectX::XMLoadFloat4(&m_Rotations[p1Index].orientation)
					, scaleFactor);
				finalRotation = DirectX::XMQuaternionNormalize(finalRotation);
				return DirectX::XMMatrixRotationQuaternion(finalRotation);
			}

		}

		DirectX::XMMATRIX InterpolateScaling(float animationTime)
		{
			if (1 == m_NumScalings)
				return DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&m_Scales[0].scale));

			int p0Index = GetScaleIndex(animationTime);
			int p1Index = p0Index + 1;


			if (p0Index == -1)
			{
				return DirectX::XMMatrixScaling(1.0f, 1.0f, 1.0f);
			}
			else
			{
				float scaleFactor = GetScaleFactor(m_Scales[p0Index].timeStamp,
					m_Scales[p1Index].timeStamp, animationTime);
				const DirectX::XMVECTOR finalScale = DirectX::XMVectorLerp(DirectX::XMLoadFloat3(&m_Scales[p0Index].scale), DirectX::XMLoadFloat3(&m_Scales[p1Index].scale)
					, scaleFactor);
				return  DirectX::XMMatrixScalingFromVector(finalScale);
			}

		}

		std::vector<KeyPosition> m_Positions;
		std::vector<KeyRotation> m_Rotations;
		std::vector<KeyScale> m_Scales;
		int m_NumPositions;
		int m_NumRotations;
		int m_NumScalings;

		DirectX::XMFLOAT4X4 m_LocalTransform;
		std::string m_Name;
		int m_ID;
	};

	class Animation
	{
	public:
		Animation(const Animation& other)
			: m_AnimationName(other.m_AnimationName),
			m_Duration(other.m_Duration),
			m_TicksPerSecond(other.m_TicksPerSecond),
			m_BonesAnimationInformation(other.m_BonesAnimationInformation),
			m_RootNode(other.m_RootNode),
			m_BoneInfoMap(other.m_BoneInfoMap)
		{
		}

		Animation(Animation&& other) noexcept
			: m_AnimationName(std::move(other.m_AnimationName)),
			m_Duration(other.m_Duration),
			m_TicksPerSecond(other.m_TicksPerSecond),
			m_BonesAnimationInformation(std::move(other.m_BonesAnimationInformation)),
			m_RootNode(std::move(other.m_RootNode)),
			m_BoneInfoMap(other.m_BoneInfoMap)
		{
		}

		Animation& operator=(const Animation& other)
		{
			if (this == &other)
				return *this;
			m_AnimationName = other.m_AnimationName;
			m_Duration = other.m_Duration;
			m_TicksPerSecond = other.m_TicksPerSecond;
			m_BonesAnimationInformation = other.m_BonesAnimationInformation;
			m_RootNode = other.m_RootNode;
			m_BoneInfoMap = other.m_BoneInfoMap;
			return *this;
		}

		Animation& operator=(Animation&& other) noexcept
		{
			if (this == &other)
				return *this;
			m_AnimationName = std::move(other.m_AnimationName);
			m_Duration = other.m_Duration;
			m_TicksPerSecond = other.m_TicksPerSecond;
			m_BonesAnimationInformation = std::move(other.m_BonesAnimationInformation);
			m_RootNode = std::move(other.m_RootNode);
			m_BoneInfoMap = other.m_BoneInfoMap;
			return *this;
		}


		Animation(std::string animationName, float duration, int ticksPerSecond, std::vector<BoneAnimationKeyframes> bonesAnimationInformation, std::map<std::string, BoneInfo> boneInfo, TransformNode* rootNodeOfTheModel)
		{
			m_AnimationName = animationName;
			m_RootNode = rootNodeOfTheModel;
			m_BoneInfoMap = boneInfo;
			m_BonesAnimationInformation = bonesAnimationInformation;
			m_Duration = duration;
			m_TicksPerSecond = ticksPerSecond;
		}

		~Animation()
		{
		}

		BoneAnimationKeyframes* FindBone(const std::string& name)
		{
			auto iter = std::find_if(m_BonesAnimationInformation.begin(), m_BonesAnimationInformation.end(),
				[&](const BoneAnimationKeyframes& Bone)
				{
					return Bone.GetBoneName() == name;
				}
			);
			if (iter == m_BonesAnimationInformation.end()) return nullptr;
			else return &(*iter);
		}

		inline std::string GetAnimationName() { return m_AnimationName; }
		inline float GetTicksPerSecond() { return m_TicksPerSecond; }
		inline std::vector<BoneAnimationKeyframes> GetBonesAnimationInformation() { return m_BonesAnimationInformation; }
		inline float GetDuration() { return m_Duration; }
		inline TransformNode* GetRootNode() { return m_RootNode; }
		inline void SetTicksPerSecond(int ticksPerSecond) { m_TicksPerSecond = ticksPerSecond; }
		inline const std::map<std::string, BoneInfo>& GetBoneIDMap()
		{
			return m_BoneInfoMap;
		}

	private:
		std::string m_AnimationName;
		float m_Duration;
		int m_TicksPerSecond;
		std::vector<BoneAnimationKeyframes> m_BonesAnimationInformation;
		TransformNode* m_RootNode;
		std::map<std::string, BoneInfo> m_BoneInfoMap;
	};

	class Animator
	{
	public:
		Animator()
		{
			
		}

		void UpdateAnimation(float dt)
		{
			m_DeltaTime = dt;
			if (m_CurrentAnimation)
			{
				m_CurrentTime += m_CurrentAnimation->GetTicksPerSecond() * dt;
				m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->GetDuration());
				CalculateBoneTransform(m_CurrentAnimation->GetRootNode(), DirectX::XMMatrixIdentity());
			}
		}

		void PlayAnimation(Animation* pAnimation)
		{
			m_CurrentTime = 0.0;
			m_CurrentAnimation = pAnimation;
			m_FinalBoneMatrices.clear();
			m_FinalBoneMatrices.reserve(100);

			for (int i = 0; i < 100; i++)
			{
				DirectX::XMFLOAT4X4 identityMatrix;
				DirectX::XMStoreFloat4x4(&identityMatrix, DirectX::XMMatrixIdentity());
				m_FinalBoneMatrices.push_back(identityMatrix);
			}

		}

		void CalculateBoneTransform(TransformNode* node, DirectX::XMMATRIX parentTransform)
		{
			std::string nodeName = node->TransFormName();
			DirectX::XMMATRIX nodeTransform = node->GetLocalToParentSpaceTransformMatrix();

			BoneAnimationKeyframes* Bone = m_CurrentAnimation->FindBone(nodeName);

			if (Bone)
			{
				Bone->Update(m_CurrentTime);
				nodeTransform = Bone->GetLocalTransform();
			}

			DirectX::XMMATRIX globalTransformation = nodeTransform * parentTransform ;

			auto boneInfoMap = m_CurrentAnimation->GetBoneIDMap();
			if (boneInfoMap.find(nodeName) != boneInfoMap.end())
			{
				int index = boneInfoMap[nodeName].BoneId;
				DirectX::XMMATRIX offset = DirectX::XMLoadFloat4x4(&boneInfoMap[nodeName].BoneOffsetMatrix);
				DirectX::XMStoreFloat4x4(&m_FinalBoneMatrices[index], offset * globalTransformation);
			}

			for (int i = 0; i < node->GetLocalChildCount(); i++)
				CalculateBoneTransform(node->GetLocalChild(i), globalTransformation);
		}

		std::vector<DirectX::XMFLOAT4X4>& GetFinalBoneMatrices()
		{
			return m_FinalBoneMatrices;
		}

	private:
		std::vector<DirectX::XMFLOAT4X4> m_FinalBoneMatrices;
		Animation* m_CurrentAnimation;
		float m_CurrentTime;
		float m_DeltaTime;

	};
}


struct AnimatedVertex
{
	AnimatedVertex() : X(0), Y(0), Z(0), TU(0), TV(0), NX(0), NY(0), NZ(0),
					BoneIdOne(-1), BoneIdTwo(-1), BoneIdThree(-1), BoneIdFour(-1), BoneWeightOne(0.0f), BoneWeightTwo(0.0f), BoneWeightThree(0.0f), BoneWeightFour(0.0f)
	{
	}

	AnimatedVertex(float x, float y, float z, float tu, float tv, float nx, float ny, float nz, float boneIdOne, float boneWeightOne, float boneIdTwo, float boneWeightTwo, float boneIdThree, float boneWeightThree, float boneIdFour, float boneWeightFour)
		:X(x), Y(y), Z(z), TU(tu), TV(tv), NX(nx), NY(ny), NZ(nz), BoneIdOne(boneIdOne), BoneIdTwo(boneIdTwo), BoneIdThree(boneIdThree), BoneIdFour(boneIdFour), BoneWeightOne(boneWeightOne), BoneWeightTwo(boneWeightTwo), BoneWeightThree(boneWeightThree), BoneWeightFour(boneWeightFour)
	{
		if(BoneIdOne < 0)
		{
			BoneIdOne = 0;
			BoneWeightOne = 0;
		}

		if (BoneIdTwo < 0)
		{
			BoneIdTwo = 0;
			BoneWeightTwo = 0;
		}

		if (BoneIdThree < 0)
		{
			BoneIdThree = 0;
			BoneWeightThree = 0;
		}

		if (BoneIdFour < 0)
		{
			BoneIdFour = 0;
			BoneWeightFour = 0;
		}
	}

	float X, Y, Z;
	float TU, TV;
	float NX, NY, NZ;
	float BoneIdOne;
	float BoneIdTwo;
	float BoneIdThree;
	float BoneIdFour;
	float BoneWeightOne;
	float BoneWeightTwo;
	float BoneWeightThree;
	float BoneWeightFour;

};

class AnimatedMesh
{
public:
	AnimatedMesh();
	AnimatedMesh(Direct3D* d3d12Device, std::vector<AnimatedVertex> vertices, std::vector<int> indicies, std::vector<Subset> subsets);
	AnimatedMesh(const AnimatedMesh&);
	~AnimatedMesh();

	void CreateBuffers(Direct3D* d3d12Device, std::vector<AnimatedVertex> vertices, std::vector<int> indicies, std::vector<Subset> subsets);
	void Render(Direct3D* direct3d);
private:

	std::unique_ptr<MeshUtilities::MeshGeometry> m_MeshGeometry;
	DirectX::BoundingOrientedBox MeshBoundingBox;
	D3D12_INDEX_BUFFER_VIEW m_IndexBufferView;
	D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;

	UINT vbByteSize;
	UINT ibByteSize;
};
#endif
