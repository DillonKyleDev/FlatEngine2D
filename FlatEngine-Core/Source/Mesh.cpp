#include "Mesh.h"
#include "FlatEngine.h"
#include "VulkanManager.h"
#include "Vector3.h"
#include "WinSys.h"
#include "Transform.h"
#include "Camera.h"

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

		m_model = nullptr;
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
		m_winSystem = nullptr;
		m_logicalDevice = nullptr;
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

		std::string modelPath = "";
		if (m_model != nullptr)
		{
			modelPath = m_model->GetModelPath();
		}

		json jsonData = {
			{ "type", "Mesh"},
			{ "id", GetID() },
			{ "_isCollapsed", IsCollapsed() },
			{ "_isActive", IsActive() },
			{ "textures", texturesData },
			{ "materialName", m_materialName },
			{ "modelPath", modelPath },
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
			int allocationIndex = texture->second.GetAllocationIndex();

			if (allocationIndex >= 0)
			{
				m_sceneViewMaterial->GetAllocator().SetFreed(allocationIndex);
				m_gameViewMaterial->GetAllocator().SetFreed(allocationIndex);
			}
		}
	}

	void Mesh::CleanupUniformBuffers()
	{
		for (size_t i = 0; i < VM_MAX_FRAMES_IN_FLIGHT; i++)
		{
			F_VulkanManager->QueueBufferDeletion(m_sceneViewUniformBuffers[i]);
			F_VulkanManager->QueueDeviceMemoryDeletion(m_sceneViewUniformBuffersMemory[i]);
			F_VulkanManager->QueueBufferDeletion(m_gameViewUniformBuffers[i]);
			F_VulkanManager->QueueDeviceMemoryDeletion(m_gameViewUniformBuffersMemory[i]);
		}
	}

	void Mesh::Cleanup()
	{				
		CleanupTextures();	
		CleanupUniformBuffers();
	}

	void Mesh::Init(WinSys* winSystem, LogicalDevice* logicalDevice)
	{
		m_winSystem = winSystem;
		m_logicalDevice = logicalDevice;

		CreateUniformBuffers();
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
		std::shared_ptr<Model> loadedModel = F_VulkanManager->GetModel(modelPath);

		if (loadedModel == nullptr)
		{
			loadedModel = F_VulkanManager->LoadModel(modelPath);
		}

		m_model = loadedModel;

		F_VulkanManager->AddSceneViewMaterialMesh(m_materialName, GetID(), this);
		F_VulkanManager->AddGameViewMaterialMesh(m_materialName, GetID(), this);
	}

	std::shared_ptr<Model> Mesh::GetModel()
	{
		return m_model;
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

		if (m_sceneViewMaterial != nullptr && m_model != nullptr)
		{
			for (std::map<uint32_t, TexturePipelineData>::iterator iter = m_sceneViewMaterial->GetTexturesPipelineData()->begin(); iter != m_sceneViewMaterial->GetTexturesPipelineData()->end(); iter++)
			{
				if (iter->second.descriptorType != VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT)
				{
					if (!m_texturesByIndex.count(iter->first) || (m_texturesByIndex.count(iter->first) && (m_texturesByIndex.at(iter->first).GetTexturePath() == "" || m_texturesByIndex.at(iter->first).GetTexturePath() == GetTextureObject("resourceNotPresent")->GetTexturePath())))
					{
						Texture emptyTexture = Texture();
						m_texturesByIndex.emplace(iter->first, emptyTexture);
						m_texturesByIndex.at(iter->first).LoadFromFile(GetTextureObject("resourceNotPresent")->GetTexturePath());
						m_b_missingTextures = true;
					}
				}
			}

			CreateTextureResources();

			if (m_sceneViewMaterial != nullptr)
			{
				m_sceneViewMaterial->CreateDescriptorSets(m_sceneViewDescriptorSets, m_sceneViewUniformBuffers, m_texturesByIndex);
			}	
			if (m_gameViewMaterial != nullptr)
			{
				m_gameViewMaterial->CreateDescriptorSets(m_gameViewDescriptorSets, m_gameViewUniformBuffers, m_texturesByIndex);
			}

			// Create empty material descriptor sets for Scene View and Game View
			std::map<uint32_t, Texture> emptyTextures = std::map<uint32_t, Texture>();
			F_VulkanManager->GetMaterial("fl_empty", ViewportType::SceneView)->CreateDescriptorSets(m_emptySceneViewDescriptorSets, m_sceneViewUniformBuffers, emptyTextures);
			F_VulkanManager->GetMaterial("fl_empty", ViewportType::GameView)->CreateDescriptorSets(m_emptyGameViewDescriptorSets, m_gameViewUniformBuffers, emptyTextures);
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

	void Mesh::AddTextureLua(std::string path, int index)
	{
		if (index >= 0)
		{
			AddTexture(path, index);
		}
		else
		{
			LogError("Texture index must be positive.");
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

	void Mesh::CreateUniformBuffers()
	{
		// Refer to - https://vulkan-tutorial.com/en/Uniform_buffers/Descriptor_layout_and_buffer

		VkDeviceSize bufferSize = sizeof(CustomUBO);

		m_sceneViewUniformBuffers.resize(VM_MAX_FRAMES_IN_FLIGHT);
		m_sceneViewUniformBuffersMemory.resize(VM_MAX_FRAMES_IN_FLIGHT);
		m_sceneViewUniformBuffersMapped.resize(VM_MAX_FRAMES_IN_FLIGHT);

		m_gameViewUniformBuffers.resize(VM_MAX_FRAMES_IN_FLIGHT);
		m_gameViewUniformBuffersMemory.resize(VM_MAX_FRAMES_IN_FLIGHT);
		m_gameViewUniformBuffersMapped.resize(VM_MAX_FRAMES_IN_FLIGHT);

		for (size_t i = 0; i < VM_MAX_FRAMES_IN_FLIGHT; i++)
		{
			m_winSystem->CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_sceneViewUniformBuffers[i], m_sceneViewUniformBuffersMemory[i]);
			vkMapMemory(m_logicalDevice->GetDevice(), m_sceneViewUniformBuffersMemory[i], 0, bufferSize, 0, &m_sceneViewUniformBuffersMapped[i]);

			m_winSystem->CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_gameViewUniformBuffers[i], m_gameViewUniformBuffersMemory[i]);
			vkMapMemory(m_logicalDevice->GetDevice(), m_gameViewUniformBuffersMemory[i], 0, bufferSize, 0, &m_gameViewUniformBuffersMapped[i]);
		}
	}

	void Mesh::UpdateUniformBuffer(ViewportType viewportType, bool b_orthographic)
	{
		Transform* transform = m_parent->GetTransform();
		Vector3 meshPosition = transform->GetPosition();
		glm::mat4 meshScale = transform->GetScaleMatrix();
		glm::mat4 meshRotation = transform->GetRotationMatrix();
		Camera* primaryCamera = nullptr;
		Vector3 cameraPosition = Vector3();		
		std::map<uint32_t, std::string> materialVec4s;

		switch (viewportType)
		{
		case ViewportType::SceneView:
		{
			primaryCamera = F_sceneViewCameraObject->GetCamera();
			if (primaryCamera != nullptr)
			{
				cameraPosition = F_sceneViewCameraObject->GetTransform()->GetPosition();
			}

			materialVec4s = m_sceneViewMaterial->GetUBOVec4Names();
			break;
		}
		case ViewportType::GameView:
		{
			primaryCamera = GetPrimaryCamera();
			if (primaryCamera != nullptr)
			{
				cameraPosition = primaryCamera->GetParent()->GetTransform()->GetPosition();
			}
			else
			{
				primaryCamera = F_sceneViewCameraObject->GetCamera();
				cameraPosition = F_sceneViewCameraObject->GetTransform()->GetPosition();
			}

			materialVec4s = m_gameViewMaterial->GetUBOVec4Names();
			break;
		}
		default:
			break;
		}


		if (primaryCamera != nullptr)
		{
			//glm::mat4 cameraRotation = primaryCamera->GetParentPtr()->GetTransform()->GetRotationMatrix();
			bool b_forceZUp = primaryCamera->ForceZUp();
			//glm::vec4 lookDir = cameraRotation * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
			glm::vec4 lookDir = primaryCamera->GetLookDirection();
			glm::vec4 up;

			if (b_forceZUp)
			{
				up = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
			}
			else
			{
				up = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
			}

			glm::vec4 meshPos = glm::vec4(meshPosition.x, meshPosition.y, meshPosition.z, 0);
			glm::vec4 viewportCameraPos = glm::vec4(cameraPosition.x, cameraPosition.y, cameraPosition.z, 0);
			glm::mat4 model = meshRotation * meshScale;
			glm::vec4 cameraLookDir = glm::vec4(lookDir.x, lookDir.y, lookDir.z, 0);
			glm::mat4 view = glm::lookAt(cameraPosition.GetGLMVec3(), glm::vec3(cameraPosition.x + cameraLookDir.x, cameraPosition.y + cameraLookDir.y, cameraPosition.z + cameraLookDir.z), glm::vec3(up));

			glm::mat4 projection;
			if (b_orthographic)
			{
				projection = glm::ortho(0.0f, 1.0f, 1.0f, 0.0f, 10.0f, 100000.0f);
			}
			else
			{
				float nearClip = primaryCamera->GetNearClippingDistance();
				float farClip = primaryCamera->GetFarClippingDistance();
				float perspectiveAngle = primaryCamera->GetPerspectiveAngle();
				float aspectRatio = (float)(m_winSystem->GetExtent().width / m_winSystem->GetExtent().height);
				projection = glm::perspective(glm::radians(perspectiveAngle), aspectRatio, nearClip, farClip);
				projection[1][1] *= -1;
			}

			CustomUBO ubo{};

			BaseUBO base{};
			base.meshPosition = meshPos;
			base.cameraPosition = viewportCameraPos;
			base.model = model;
			base.view = view;
			base.projection = projection;
			ubo.BaseUBO = base;

			int vec4Counter = 0;
			for (std::map<uint32_t, std::string>::iterator materialVec4 = materialVec4s.begin(); materialVec4 != materialVec4s.end(); materialVec4++)
			{
				if (materialVec4->first <= 32 && m_uboVec4s.count(materialVec4->second)) // FIX ME: 32 is the size of the m_uboVec4s array passed to the shaders
				{
					ubo.vec4s[materialVec4->first] = m_uboVec4s.at(materialVec4->second);
					vec4Counter++;
				}
			}

			switch (viewportType)
			{
			case ViewportType::SceneView:
			{
				memcpy(m_sceneViewUniformBuffersMapped[VM_currentFrame], &ubo, sizeof(ubo));
				break;
			}
			case ViewportType::GameView:
			{
				memcpy(m_gameViewUniformBuffersMapped[VM_currentFrame], &ubo, sizeof(ubo));
				break;
			}
			default:
				break;
			}
		}
	}

	std::vector<VkBuffer>& Mesh::GetSceneViewUniformBuffers()
	{
		return m_sceneViewUniformBuffers;
	}

	std::vector<VkBuffer>& Mesh::GetGameViewUniformBuffers()
	{
		return m_gameViewUniformBuffers;
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