//Material Manager Class
//Class to manage the Material data of 3D Objects.
#ifndef _MATERIALMANAGER_H_
#define _MATERIALMANAGER_H_

//Includes
#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>
#include "DataFileContainer.h"
#include "Singleton.h"
#include "Dynamic Descriptor Heap/DefaultTextures.h"


struct SurfaceMaterial
{
	std::string MaterialName;
	std::vector<TextureRef>Textures;
	int AlbedoMapIndex;
	int RoughnessMapIndex;
	int MetalnessMapIndex;
	int NormalMapIndex;
	DirectX::XMFLOAT3 Albedo;
	float Roughness;
	float Metalness;
	float F0;
	float Transparency;
	bool RenderForward;
	bool IsTransparent;
	bool UseAlphaChannelTransparency;
	bool HasAlbedoMap;
	bool HasRoughnessMap;
	bool HasMetalnessMap;
	bool HasNormalMap;
};


class MaterialManager : public Singleton<MaterialManager>
{
	friend class Singleton<MaterialManager>;
public:
	
	MaterialManager();
	~MaterialManager();

	bool Initialize(Direct3D* direct_3d);
	bool AddMaterialFile(std::string fileName);
	SurfaceMaterial GetMaterial(std::string materialName);

	bool SetMaterialAlbedoColor(std::string materialName, DirectX::FXMVECTOR albedoColor);
	bool SetMaterialRoughness(std::string materialName, float roughness);
	bool SetMaterialMetalness(std::string materialName, float metalness);
	/*bool SetMaterialAlbedoMap(std::string materialName, std::string albedoMapFilename);
	bool SetMaterialRoughnessMap(std::string materialName, std::string roughnessMapFilename);
	bool SetMaterialMetalnessMap(std::string materialName, std::string metalnessMapFilename);
	bool SetMaterialNormalMap( std::string materialName, std::string normalMapFilename);*/

private:
	Direct3D* m_Direct_3d;
	DataFileContainer m_MaterialFile;
	std::vector<SurfaceMaterial> m_Materials;
};
#endif