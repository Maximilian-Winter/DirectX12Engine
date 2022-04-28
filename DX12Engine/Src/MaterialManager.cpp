#include "MaterialManager.h"

MaterialManager::MaterialManager()
{
	m_Materials.clear();
}

MaterialManager::~MaterialManager()
{

}

bool MaterialManager::Initialize(Direct3D* direct_3d)
{
	m_Direct_3d = direct_3d;
	AddMaterialFile("default.matlib");
	return true;
}

bool MaterialManager::AddMaterialFile(std::string MaterialFilename)
{
	bool result;
	
	//Clear the config data object and use it to parse the material file
	m_MaterialFile.ClearDataContainer();
	result = m_MaterialFile.LoadDataContainerFile(MaterialFilename);
	if (result)
	{
		int NumberOfMaterials = m_MaterialFile.GetNumberOfRootSections();

		for (int i = 0; i < NumberOfMaterials; i++)
		{
			SurfaceMaterial tempMaterial;
			std::string tempFilename;

			std::vector<int> RootIndicies;

			result = m_MaterialFile.GetSectionName(m_MaterialFile.GetIndiciePathToRootSecton(i), tempMaterial.MaterialName);
			if (!result)
			{
				return false;
			}
			
			for (int k = 0; k < m_Materials.size(); k++)
			{
				if (m_Materials[k].MaterialName == tempMaterial.MaterialName)
				{
					return true;
				}
			}

			//Add color value to the material
			std::vector<float> tempAlbedo;
			result = m_MaterialFile.GetFloatVectorByName(tempMaterial.MaterialName, "albedo", tempAlbedo);
			if (!result || tempAlbedo.size() != 3)
			{
				tempAlbedo.clear();
				tempAlbedo.push_back(1.0f);
				tempAlbedo.push_back(0.0f);
				tempAlbedo.push_back(0.0f);
			}
			tempMaterial.Albedo.x = tempAlbedo[0];
			tempMaterial.Albedo.y = tempAlbedo[1];
			tempMaterial.Albedo.z = tempAlbedo[2];

			float tempRoughness;
			result = m_MaterialFile.GetFloatValueByName(tempMaterial.MaterialName, "roughness", tempRoughness);
			if (!result)
			{
				tempRoughness = 0.5f;
			}
			tempMaterial.Roughness = tempRoughness;

			float tempMetalness;
			result = m_MaterialFile.GetFloatValueByName(tempMaterial.MaterialName, "metalness", tempMetalness);
			if (!result)
			{
				tempMetalness = 0.0f;
			}
			tempMaterial.Metalness = tempMetalness;

			float tempF0;
			result = m_MaterialFile.GetFloatValueByName(tempMaterial.MaterialName, "f0", tempF0);
			if (!result)
			{
				tempF0 = 0.06;
			}
			tempMaterial.F0 = tempF0;

			float tempTransparency;
			result = m_MaterialFile.GetFloatValueByName(tempMaterial.MaterialName, "transparency", tempTransparency);
			if (!result)
			{
				tempTransparency = 1.0;
			}
			tempMaterial.Transparency = tempTransparency;


			//Add the albedoMap to the texture manager
			result = m_MaterialFile.GetStringValueByName(tempMaterial.MaterialName, "albedo_map", tempFilename);
			if (result)
			{
				tempMaterial.Textures.push_back(TextureManager::LoadDDSFromFile(m_Direct_3d, tempFilename));
				if (!tempMaterial.Textures[tempMaterial.Textures.size() - 1].IsValid())
				{
					tempMaterial.HasAlbedoMap = false;
					tempMaterial.AlbedoMapIndex = -1;
				}
				else
				{
					tempMaterial.HasAlbedoMap = true;
					tempMaterial.AlbedoMapIndex = tempMaterial.Textures.size() - 1;
				}
			}
			else
			{
				tempMaterial.HasAlbedoMap = false;
				tempMaterial.AlbedoMapIndex = -1;
			}
			

			//Add the roughnessMap to the texture manager
			result = m_MaterialFile.GetStringValueByName(tempMaterial.MaterialName, "roughness_map", tempFilename);
			if (result)
			{
				tempMaterial.Textures.push_back(TextureManager::LoadDDSFromFile(m_Direct_3d, tempFilename));
				if (!tempMaterial.Textures[tempMaterial.Textures.size() - 1].IsValid())
				{
					tempMaterial.HasRoughnessMap = false;
					tempMaterial.RoughnessMapIndex = -1;
				}
				else
				{
					tempMaterial.HasRoughnessMap = true;
					tempMaterial.RoughnessMapIndex = tempMaterial.Textures.size() - 1;
				}
			}
			else
			{
				tempMaterial.HasRoughnessMap = false;
				tempMaterial.RoughnessMapIndex = -1;
			}
			

			//Add the metalnessMap to the texture manager
			result = m_MaterialFile.GetStringValueByName(tempMaterial.MaterialName, "metalness_map", tempFilename);
			if (result)
			{
				tempMaterial.Textures.push_back(TextureManager::LoadDDSFromFile(m_Direct_3d, tempFilename));
				if (!tempMaterial.Textures[tempMaterial.Textures.size() - 1].IsValid())
				{
					tempMaterial.HasMetalnessMap = false;
					tempMaterial.MetalnessMapIndex = -1;
				}
				else
				{
					tempMaterial.HasMetalnessMap = true;
					tempMaterial.MetalnessMapIndex = tempMaterial.Textures.size() - 1;
				}
			}
			else
			{
				tempMaterial.HasMetalnessMap = false;
				tempMaterial.MetalnessMapIndex = -1;
			}

			//Add the normalMap to the texture manager
			result = m_MaterialFile.GetStringValueByName(tempMaterial.MaterialName, "normal_map", tempFilename);
			if (result)
			{
				tempMaterial.Textures.push_back(TextureManager::LoadDDSFromFile(m_Direct_3d, tempFilename));
				if (!tempMaterial.Textures[tempMaterial.Textures.size()-1].IsValid())
				{
					tempMaterial.HasNormalMap = false;
					tempMaterial.NormalMapIndex = -1;
				}
				else
				{
					tempMaterial.HasNormalMap = true;
					tempMaterial.NormalMapIndex = tempMaterial.Textures.size() - 1;
				}
			}
			else
			{
				tempMaterial.HasNormalMap = false;
				tempMaterial.NormalMapIndex = -1;
			}

			bool renderForward;
			result = m_MaterialFile.GetBoolValueByName(tempMaterial.MaterialName, "render_forward", renderForward);
			if (!result)
			{
				renderForward = false;
			}
			tempMaterial.RenderForward = renderForward;

			bool IsTransparent = false;
			result = m_MaterialFile.GetBoolValueByName(tempMaterial.MaterialName, "is_transparent", IsTransparent);
			if (!result)
			{
				IsTransparent = false;
			}
			tempMaterial.IsTransparent = IsTransparent;

			bool UseAlphaChannelTransparency = false;
			result = m_MaterialFile.GetBoolValueByName(tempMaterial.MaterialName, "use_alphachannel_transparency", UseAlphaChannelTransparency);
			if (!result)
			{
				UseAlphaChannelTransparency = false;
			}
			tempMaterial.UseAlphaChannelTransparency = UseAlphaChannelTransparency;

			m_Materials.push_back(tempMaterial);
		}
	}
	else
	{
		return false;
	}

	return true;
}

SurfaceMaterial MaterialManager::GetMaterial(std::string MaterialName)
{
	auto it = std::find_if(m_Materials.begin(), m_Materials.end(), [&MaterialName](const SurfaceMaterial& obj) {return obj.MaterialName == MaterialName; });

	if (it != m_Materials.end())
	{
		return *it;
	}

	return GetMaterial("Default");
}

bool MaterialManager::SetMaterialAlbedoColor(std::string materialName, DirectX::FXMVECTOR albedoColor)
{
	for (auto it = m_Materials.begin(); it < m_Materials.end(); it++)
	{
		if (materialName == it->MaterialName)
		{
			DirectX::XMStoreFloat3(&it->Albedo, albedoColor);
			it->HasAlbedoMap = false;
			return true;
		}
	}
	return false;
}

bool MaterialManager::SetMaterialRoughness(std::string materialName, float roughness)
{
	for (auto it = m_Materials.begin(); it < m_Materials.end(); it++)
	{
		if (materialName == it->MaterialName)
		{
			it->Roughness = roughness;
			it->HasRoughnessMap = false;
			return true;
		}
	}
	return false;
}

bool MaterialManager::SetMaterialMetalness(std::string materialName, float metalness)
{
	for (auto it = m_Materials.begin(); it < m_Materials.end(); it++)
	{
		if (materialName == it->MaterialName)
		{
			it->Metalness = metalness;
			it->HasMetalnessMap = false;
			return true;
		}
	}
	return false;
}
/*
bool MaterialManager::SetMaterialAlbedoMap(std::string materialName, std::string albedoMapFilename)
{
	bool result;
	for (auto it = m_Materials.begin(); it < m_Materials.end(); it++)
	{
		if (materialName == it->MaterialName)
		{
			//Add the albedoMap to the texture manager
			it->AlbedoMapIndex = DefaultTextures::LoadDDSFromFile(albedoMapFilename);
			if (!it->AlbedoMapIndex.IsValid())
			{
				it->HasAlbedoMap = false;
			}
			else
			{
				it->HasAlbedoMap = true;
				return true;
			}
		}
	}
	return false;
}

bool MaterialManager::SetMaterialRoughnessMap(std::string materialName, std::string roughnessMapFilename)
{
	bool result;
	for (auto it = m_Materials.begin(); it < m_Materials.end(); it++)
	{
		if (materialName == it->MaterialName)
		{
			it->RoughnessMapIndex = DefaultTextures::LoadDDSFromFile(roughnessMapFilename);
			if (!it->RoughnessMapIndex.IsValid())
			{
				it->HasRoughnessMap = false;
			}
			else
			{
				it->HasRoughnessMap = true;
				return true;
			}
		}
	}
	return false;
}

bool MaterialManager::SetMaterialMetalnessMap(std::string materialName, std::string metalnessMapFilename)
{
	bool result;
	for (auto it = m_Materials.begin(); it < m_Materials.end(); it++)
	{
		if (materialName == it->MaterialName)
		{
			it->MetalnessMapIndex = DefaultTextures::LoadDDSFromFile(metalnessMapFilename);
			if (!it->MetalnessMapIndex.IsValid())
			{
				it->HasMetalnessMap = false;
			}
			else
			{
				it->HasMetalnessMap = true;
				return true;
			}
		}
	}
	return false;
}

bool MaterialManager::SetMaterialNormalMap(std::string materialName, std::string normalMapFilename)
{
	bool result;
	for (auto it = m_Materials.begin(); it < m_Materials.end(); it++)
	{
		if (materialName == it->MaterialName)
		{
			it->NormalMap = DefaultTextures::LoadDDSFromFile(normalMapFilename);
			if (!it->NormalMap.IsValid())
			{
				it->HasNormalMap = false;
			}
			else
			{
				it->HasNormalMap = true;
				return true;
			}
		}
	}
	return false;
}
*/