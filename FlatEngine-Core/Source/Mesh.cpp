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

		m_sceneViewModel = Model();
		m_gameViewModel = Model();
		m_sceneViewMaterial = F_VulkanManager->GetMaterial("fl_empty");
		m_gameViewMaterial = F_VulkanManager->GetMaterial("fl_empty");
		m_sceneViewDescriptorSets = std::vector<VkDescriptorSet>(VM_MAX_FRAMES_IN_FLIGHT, {});
		m_gameViewDescriptorSets = std::vector<VkDescriptorSet>(VM_MAX_FRAMES_IN_FLIGHT, {});
		m_emptySceneViewDescriptorSets = std::vector<VkDescriptorSet>(VM_MAX_FRAMES_IN_FLIGHT, {});
		m_emptyGameViewDescriptorSets = std::vector<VkDescriptorSet>(VM_MAX_FRAMES_IN_FLIGHT, {});
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

		if (m_sceneViewMaterial != nullptr)
		{	
			std::map<uint32_t, std::string> vec4Names = m_sceneViewMaterial->GetUBOVec4Names();
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
			{ "materialName", m_materialName },
			{ "modelPath", m_sceneViewModel.GetModelPath() },
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
			m_sceneViewMaterial->GetAllocator().SetFreed(texture->second.GetAllocationIndex());
			m_gameViewMaterial->GetAllocator().SetFreed(texture->second.GetAllocationIndex());
		}
	}

	void Mesh::Cleanup()
	{				
		CleanupTextures();		
		m_sceneViewModel.Cleanup(*m_logicalDevice);
		m_gameViewModel.Cleanup(*m_logicalDevice);
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

	void Mesh::SetModel(std::string modelPath)
	{
		if (m_sceneViewModel.GetModelPath() != "")
		{
			m_sceneViewModel.Cleanup(*m_logicalDevice);
			m_gameViewModel.Cleanup(*m_logicalDevice);
		}

		m_sceneViewModel.SetModelPath(modelPath);
		m_gameViewModel.SetModelPath(modelPath);

		if (modelPath != "")
		{
			CreateModelResources(FlatEngine::F_VulkanManager->GetCommandPool(), FlatEngine::F_VulkanManager->GetPhysicalDevice(), FlatEngine::F_VulkanManager->GetLogicalDevice());
		}
	}

	Model& Mesh::GetSceneViewModel()
	{
		return m_sceneViewModel;
	}

	Model& Mesh::GetGameViewModel()
	{
		return m_gameViewModel;
	}

	void Mesh::CreateModelResources(VkCommandPool commandPool, PhysicalDevice& physicalDevice, LogicalDevice& logicalDevice)
	{
		std::shared_ptr<Model> loadedModel = F_VulkanManager->GetModel(m_sceneViewModel.GetModelPath());

		if (loadedModel == nullptr)
		{
			loadedModel = F_VulkanManager->LoadModel(m_sceneViewModel.GetModelPath());
		}

		m_sceneViewModel.SetVertices(loadedModel->GetVertices());
		m_sceneViewModel.SetIndices(loadedModel->GetIndices());
		m_sceneViewModel.CreateVertexBuffer(commandPool, physicalDevice, logicalDevice);
		m_sceneViewModel.CreateIndexBuffer(commandPool, physicalDevice, logicalDevice);
		m_sceneViewModel.CreateUniformBuffers(physicalDevice, logicalDevice);

		m_gameViewModel.SetVertices(loadedModel->GetVertices());
		m_gameViewModel.SetIndices(loadedModel->GetIndices());
		m_gameViewModel.CreateVertexBuffer(commandPool, physicalDevice, logicalDevice);
		m_gameViewModel.CreateIndexBuffer(commandPool, physicalDevice, logicalDevice);
		m_gameViewModel.CreateUniformBuffers(physicalDevice, logicalDevice);
	}

	void Mesh::SetMaterial(std::string materialName)
	{
		m_sceneViewMaterial = F_VulkanManager->GetMaterial(materialName, ViewportType::SceneView);
		m_gameViewMaterial = F_VulkanManager->GetMaterial(materialName, ViewportType::GameView);

		if (m_materialName != "")
		{			
			F_VulkanManager->RemoveSceneViewMaterialMesh(m_materialName, GetID(), this);
			F_VulkanManager->RemoveGameViewMaterialMesh(m_materialName, GetID(), this);
		}
		m_materialName = materialName;

		F_VulkanManager->AddSceneViewMaterialMesh(m_materialName, GetID(), this);
		F_VulkanManager->AddGameViewMaterialMesh(m_materialName, GetID(), this);

		if (m_sceneViewMaterial != nullptr)
		{
			std::map<uint32_t, std::string> uboVec4Names = m_sceneViewMaterial->GetUBOVec4Names();

			for (std::map<uint32_t, std::string>::iterator iter = uboVec4Names.begin(); iter != uboVec4Names.end(); iter++)
			{
				SetUBOVec4(iter->second, Vector4());
			}
		}
		else
		{
			LogError("Material not found: " + materialName);
		}
	}

	std::shared_ptr<Material> Mesh::GetSceneViewMaterial()
	{
		return m_sceneViewMaterial;
	}

	std::shared_ptr<Material> Mesh::GetGameViewMaterial()
	{
		return m_gameViewMaterial;
	}

	std::string Mesh::GetMaterialName()
	{
		return m_materialName;
	}

	void Mesh::CreateResources()
	{
		m_b_missingTextures = false;
		m_b_initialized = true;

		if (m_sceneViewMaterial != nullptr && m_sceneViewModel.GetModelPath() != "")
		{
			for (std::map<uint32_t, VkShaderStageFlags>::iterator iter = m_sceneViewMaterial->GetTexturesShaderStages()->begin(); iter != m_sceneViewMaterial->GetTexturesShaderStages()->end(); iter++)
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

			if (m_sceneViewModel.GetModelPath() != "")
			{
				CreateModelResources(F_VulkanManager->GetCommandPool(), F_VulkanManager->GetPhysicalDevice(), F_VulkanManager->GetLogicalDevice());
			}

			if (m_sceneViewMaterial != nullptr)
			{
				m_sceneViewMaterial->CreateDescriptorSets(m_sceneViewDescriptorSets, m_sceneViewModel, m_texturesByIndex);
			}	
			if (m_gameViewMaterial != nullptr)
			{
				m_gameViewMaterial->CreateDescriptorSets(m_gameViewDescriptorSets, m_gameViewModel, m_texturesByIndex);
			}

			// Create empty material descriptor sets for Scene View and Game View
			std::map<uint32_t, Texture> emptyTextures = std::map<uint32_t, Texture>();
			F_VulkanManager->GetMaterial("fl_empty", ViewportType::SceneView)->CreateDescriptorSets(m_emptySceneViewDescriptorSets, m_sceneViewModel, emptyTextures);
			F_VulkanManager->GetMaterial("fl_empty", ViewportType::GameView)->CreateDescriptorSets(m_emptyGameViewDescriptorSets, m_gameViewModel, emptyTextures);
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
		}
		else
		{
			m_texturesByIndex.emplace(index, newTexture);			
		}

		CreateTextureResources();
	}

	void Mesh::AddTexture(Texture texture, uint32_t index)
	{
		if (m_texturesByIndex.count(index))
		{
			m_texturesByIndex.at(index) = texture;			
		}
		else
		{
			m_texturesByIndex.emplace(index, texture);
		}

		CreateTextureResources();
	}

	std::map<uint32_t, Texture>& Mesh::GetTextures()
	{
		return m_texturesByIndex;
	}

	void Mesh::CreateTextureResources() // To be called once actual Textures with paths have been assigned to the Mesh
	{
		for (std::map<uint32_t, Texture>::iterator iter = m_texturesByIndex.begin(); iter != m_texturesByIndex.end(); iter++)
		{
			if (iter->second.GetTexturePath() == "")
			{				
				iter->second.LoadFromFile(GetTextureObject("resourceNotPresent")->GetTexturePath());
			}
			
			iter->second.CreateTextureImage();
		}
	}

	std::vector<VkDescriptorSet>& Mesh::GetSceneViewDescriptorSets()
	{
		return m_sceneViewDescriptorSets;
	}

	std::vector<VkDescriptorSet>& Mesh::GetGameViewDescriptorSets()
	{
		return m_gameViewDescriptorSets;
	}

	std::vector<VkDescriptorSet>& Mesh::GetEmptySceneViewDescriptorSets()
	{
		return m_emptySceneViewDescriptorSets;
	}

	std::vector<VkDescriptorSet>& Mesh::GetEmptyGameViewDescriptorSets()
	{
		return m_emptyGameViewDescriptorSets;
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