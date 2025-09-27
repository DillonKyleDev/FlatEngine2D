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
		m_textures = std::vector<Texture>();
		m_allocationPoolIndex = -1;
		m_b_initialized = false;
		
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
		std::string materialName = "";

		if (m_material != nullptr)
		{
			materialName = m_material->GetName();
		}

		json texturesArray = json::array();

		for (Texture texture : m_textures)
		{
			json textureData = {
				{ "path", texture.GetTexturePath() }
			};

			texturesArray.push_back(textureData);
		}

		std::vector<std::string> vec4Names = m_material->GetUBOVec4Names();
		json uboVec4s = json::object();
		for (std::string uboVec4Name : vec4Names)
		{
			if (m_uboVec4s.count(uboVec4Name))
			{
				glm::vec4 data = m_uboVec4s.at(uboVec4Name);

				json vec4Data = {
					{ "x", data.x },
					{ "y", data.y },
					{ "z", data.z },
					{ "w", data.w }
				};		

				uboVec4s.emplace(uboVec4Name, vec4Data);
			}
		}

		json jsonData = {
			{ "type", "Mesh"},
			{ "id", GetID() },
			{ "_isCollapsed", IsCollapsed() },
			{ "_isActive", IsActive() },
			{ "textures", texturesArray },
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
		for (Texture& texture : m_textures)
		{
			texture.Cleanup(*m_logicalDevice);
			m_material->GetAllocator().SetFreed(texture.GetAllocationIndex());
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
		m_model.LoadModel();
		m_model.CreateVertexBuffer(commandPool, physicalDevice, logicalDevice);
		m_model.CreateIndexBuffer(commandPool, physicalDevice, logicalDevice);
		m_model.CreateUniformBuffers(physicalDevice, logicalDevice);
	}

	void Mesh::SetMaterial(std::shared_ptr<Material> material)
	{		
		m_material = material;

		if (m_material != nullptr)
		{
			m_materialName = m_material->GetName();
			m_textures.resize(m_material->GetTextureCount());

			for (std::string uboVec4Name : material->GetUBOVec4Names())
			{
				SetUBOVec4(uboVec4Name, Vector4());
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
		if (m_material != nullptr && m_model.GetModelPath() != "" && m_textures.size() == m_material->GetTextureCount())
		{
			// make sure there are the necessary number of actual textures assigned to Mesh before creating resources
			bool b_texturesAssigned = true;

			if (m_textures.size() != m_material->GetTextureCount())
			{
				b_texturesAssigned = false;
			}
			else
			{
				for (uint32_t i = 0; i < m_material->GetTextureCount(); i++)
				{
					if (m_textures[i].GetTexturePath() == "")
					{
						b_texturesAssigned = false;
					}
				}
			}

			if (b_texturesAssigned)
			{
				CreateTextureResources();

				if (m_model.GetModelPath() != "")
				{
					CreateModelResources(FlatEngine::F_VulkanManager->GetCommandPool(), FlatEngine::F_VulkanManager->GetPhysicalDevice(), FlatEngine::F_VulkanManager->GetLogicalDevice());

					m_b_initialized = true;
				}

				if (m_material != nullptr)
				{
					m_material->CreateDescriptorSets(m_descriptorSets, m_model, m_textures);
				}
			}
		}
	}

	void Mesh::AddTexture(std::string path, uint32_t index)
	{
		if (m_textures.size() >= index + 1)
		{
			Texture newTexture = Texture();
			newTexture.SetTexturePath(path);
			m_textures[index] = newTexture;
			CreateTextureResources();
		}
	}

	void Mesh::AddTexture(Texture texture, uint32_t index)
	{
		if (m_textures.size() >= index)
		{
			m_textures[index] = texture;
			CreateTextureResources();
		}
	}

	std::vector<Texture>& Mesh::GetTextures()
	{
		return m_textures;
	}

	void Mesh::CreateTextureResources() // To be called once actual Textures with paths have been assigned to the Mesh
	{
		for (Texture& texture : m_textures)
		{
			if (texture.GetTexturePath() != "")
			{
				texture.CreateTextureImage();
			}
		}
	}

	std::vector<VkDescriptorSet>& Mesh::GetDescriptorSets()
	{
		return m_descriptorSets;
	}

	//std::map<std::string, glm::float32>& Mesh::GetUBOFloats()
	//{
	//	return m_uboFloats;
	//}
	//std::map<std::string, glm::vec2>& Mesh::GetUBOVec2s()
	//{
	//	return m_uboVec2s;
	//}
	//std::map<std::string, glm::vec3>& Mesh::GetUBOVec3s()
	//{
	//	return m_uboVec3s;
	//}
	std::map<std::string, glm::vec4>& Mesh::GetUBOVec4s()
	{
		return m_uboVec4s;
	}
	//void Mesh::SetUBOVec4(std::string name, glm::vec4 value)
	//{
	//	if (m_uboVec4s.count(name))
	//	{
	//		m_uboVec4s.at(name) = value;
	//	}
	//	else
	//	{
	//		m_uboVec4s.emplace(name, value);
	//	}
	//}
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
	//std::map<std::string, glm::mat4>& Mesh::GetUBOMat4s()
	//{
	//	return m_uboMat4s;
	//}
}