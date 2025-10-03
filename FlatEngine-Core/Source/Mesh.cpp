#include "Mesh.h"
#include "FlatEngine.h"
#include "VulkanManager.h"

#include "json.hpp"
using json = nlohmann::json;
using namespace nlohmann::literals;


namespace FlatEngine
{
	Mesh::Mesh(GameObject* parent, long myID, long parentID)
	{
		SetID(myID);
		SetParentID(parentID);
		SetType(T_Mesh);

		m_model = Model();
		m_material = F_VulkanManager->GetMaterial("engineMaterial_emptyMaterial");
		m_descriptorSets = std::vector<VkDescriptorSet>(VM_MAX_FRAMES_IN_FLIGHT, {});		
		m_texturesByIndex = std::map<uint32_t, Texture>();
		m_allocationPoolIndex = -1;
		m_b_initialized = false;
		m_b_missingTextures = false;
		
		m_uboVec4s = std::map<std::string, glm::vec4>();

		// handles		
		m_parent = parent;
		m_logicalDevice = &F_VulkanManager->GetLogicalDevice();
	}

	Mesh::~Mesh()
	{
	}

	std::string Mesh::GetData()
	{
		json texturesData = json::object();
		for (std::map<uint32_t, Texture>::iterator textureData = m_texturesByIndex.begin(); textureData != m_texturesByIndex.end(); textureData++)
		{
			texturesData.emplace(std::to_string(textureData->first), textureData->second.GetTexturePath());
		}

		std::string materialName = "";
		json uboVec4s = json::object();

		if (m_material != nullptr)
		{
			materialName = m_material->GetName();

			std::map<uint32_t, std::string> vec4Names = m_material->GetUBOVec4Names();
			for (std::map<uint32_t, std::string>::iterator iter = vec4Names.begin(); iter != vec4Names.end(); iter++)
			{
				if (m_uboVec4s.count(iter->second))
				{
					glm::vec4 data = m_uboVec4s.at(iter->second);

					json vec4Data = {
						{ "x", data.x },
						{ "y", data.y },
						{ "z", data.z },
						{ "w", data.w }
					};

					uboVec4s.emplace(iter->second, vec4Data);
				}
			}
		}

		json jsonData = {
			{ "type", "Mesh"},
			{ "id", GetID() },
			{ "_isCollapsed", IsCollapsed() },
			{ "_isActive", IsActive() },
			{ "textures", texturesData },
			{ "materialName", materialName },
			{ "modelPath", m_model.GetModelPath() },
			{ "uboVec4s", uboVec4s }
		};

		std::string data = jsonData.dump();
		// Return dumped json object with required data for saving
		return data;
	}

	void Mesh::CleanupTextures()
	{
		for (std::map<uint32_t, Texture>::iterator texture = m_texturesByIndex.begin(); texture != m_texturesByIndex.end(); texture++)
		{
			texture->second.Cleanup(*m_logicalDevice);
			m_material->GetAllocator().SetFreed(texture->second.GetAllocationIndex());
		}
	}

	void Mesh::Cleanup()
	{				
		CleanupTextures();		
		m_model.Cleanup(*m_logicalDevice);
	}


	GameObject* Mesh::GetParentPtr()
	{
		return m_parent;
	}

	bool Mesh::Initialized()
	{
		return m_b_initialized;
	}

	bool Mesh::MissingTextures()
	{
		return m_b_missingTextures;
	}

	void Mesh::SetModel(Model model)
	{
		if (m_model.GetModelPath() != "")
		{
			m_model.Cleanup(*m_logicalDevice);
		}
		m_model = model;

		if (m_model.GetModelPath() != "")
		{
			CreateModelResources(FlatEngine::F_VulkanManager->GetCommandPool(), FlatEngine::F_VulkanManager->GetPhysicalDevice(), FlatEngine::F_VulkanManager->GetLogicalDevice());
		}
	}

	void Mesh::SetModel(std::string modelPath)
	{
		if (m_model.GetModelPath() != "")
		{
			m_model.Cleanup(*m_logicalDevice);
		}

		m_model.SetModelPath(modelPath);

		if (modelPath != "")
		{
			CreateModelResources(FlatEngine::F_VulkanManager->GetCommandPool(), FlatEngine::F_VulkanManager->GetPhysicalDevice(), FlatEngine::F_VulkanManager->GetLogicalDevice());
		}
	}

	Model& Mesh::GetModel()
	{
		return m_model;
	}

	void Mesh::CreateModelResources(VkCommandPool commandPool, PhysicalDevice& physicalDevice, LogicalDevice& logicalDevice)
	{
		std::shared_ptr<Model> loadedModel = F_VulkanManager->GetModel(m_model.GetModelPath());

		if (loadedModel == nullptr)
		{
			loadedModel = F_VulkanManager->LoadModel(m_model.GetModelPath());
		}

		m_model.SetVertices(loadedModel->GetVertices());
		m_model.SetIndices(loadedModel->GetIndices());
		m_model.CreateVertexBuffer(commandPool, physicalDevice, logicalDevice);
		m_model.CreateIndexBuffer(commandPool, physicalDevice, logicalDevice);
		m_model.CreateUniformBuffers(physicalDevice, logicalDevice);
	}

	void Mesh::SetMaterial(std::shared_ptr<Material> material)
	{		
		m_material = material;

		if (m_material != nullptr)
		{
			std::map<uint32_t, std::string> uboVec4Names = material->GetUBOVec4Names();
			m_materialName = m_material->GetName();
			//m_textures.resize(m_material->GetTextureCount());

			for (std::map<uint32_t, std::string>::iterator iter = uboVec4Names.begin(); iter != uboVec4Names.end(); iter++)
			{
				SetUBOVec4(iter->second, Vector4());
			}
		}
	}

	void Mesh::SetMaterial(std::string materialName)
	{
		m_material = F_VulkanManager->GetMaterial(materialName);
		m_materialName = materialName;

		SetMaterial(m_material);
	}

	std::shared_ptr<Material> Mesh::GetMaterial()
	{
		return m_material;
	}

	std::string Mesh::GetMaterialName()
	{
		return m_materialName;
	}

	void Mesh::CreateResources()
	{
		m_b_missingTextures = false;

		if (m_material != nullptr && m_model.GetModelPath() != "")
		{
			for (std::map<uint32_t, VkShaderStageFlags>::iterator iter = m_material->GetTexturesShaderStages()->begin(); iter != m_material->GetTexturesShaderStages()->end(); iter++)
			{
				if (!m_texturesByIndex.count(iter->first) || (m_texturesByIndex.count(iter->first) && (m_texturesByIndex.at(iter->first).GetTexturePath() == "" || m_texturesByIndex.at(iter->first).GetTexturePath() == GetTextureObject("resourceNotPresent")->GetTexturePath())))
				{
					Texture emptyTexture = Texture();
					m_texturesByIndex.emplace(iter->first, emptyTexture);
					m_texturesByIndex.at(iter->first).LoadFromFile(GetTextureObject("resourceNotPresent")->GetTexturePath());
					m_b_missingTextures = true;
				}
			}

			CreateTextureResources();

			if (m_model.GetModelPath() != "")
			{
				CreateModelResources(FlatEngine::F_VulkanManager->GetCommandPool(), FlatEngine::F_VulkanManager->GetPhysicalDevice(), FlatEngine::F_VulkanManager->GetLogicalDevice());

				m_b_initialized = true;
			}

			if (m_material != nullptr)
			{
				m_material->CreateDescriptorSets(m_descriptorSets, m_model, m_texturesByIndex);
			}
		}
		else
		{
			m_b_initialized = false;
		}
	}

	void Mesh::AddTexture(std::string path, uint32_t index)
	{
		Texture newTexture = Texture();
		newTexture.SetTexturePath(path);

		if (m_texturesByIndex.count(index))
		{
			m_texturesByIndex.at(index) = newTexture;
			CreateTextureResources();
		}
		else
		{
			m_texturesByIndex.emplace(index, newTexture);
			CreateTextureResources();
		}
	}

	void Mesh::AddTexture(Texture texture, uint32_t index)
	{
		if (m_texturesByIndex.count(index))
		{
			m_texturesByIndex.at(index) = texture;
			CreateTextureResources();
		}
		else
		{
			m_texturesByIndex.emplace(index, texture);
		}
	}

	std::map<uint32_t, Texture>& Mesh::GetTextures()
	{
		return m_texturesByIndex;
	}

	void Mesh::CreateTextureResources() // To be called once actual Textures with paths have been assigned to the Mesh
	{
		for (std::map<uint32_t, Texture>::iterator iter = m_texturesByIndex.begin(); iter != m_texturesByIndex.end(); iter++)
		{
			if (iter->second.GetTexturePath() != "")
			{
				iter->second.CreateTextureImage();
			}
		}
	}

	std::vector<VkDescriptorSet>& Mesh::GetDescriptorSets()
	{
		return m_descriptorSets;
	}

	std::map<std::string, glm::vec4>& Mesh::GetUBOVec4s()
	{
		return m_uboVec4s;
	}

	void Mesh::SetUBOVec4(std::string name, Vector4 value)
	{
		if (m_uboVec4s.count(name))
		{
			m_uboVec4s.at(name) = glm::vec4(value.x, value.y, value.z, value.w);
		}
		else
		{
			m_uboVec4s.emplace(name, glm::vec4(value.x, value.y, value.z, value.w));
		}
	}
}