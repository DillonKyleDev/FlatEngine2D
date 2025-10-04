#pragma once
#include "Component.h"
#include "Model.h"
#include "Material.h"
#include "PhysicalDevice.h"
#include "LogicalDevice.h"
#include "RenderPass.h"
#include "Vector4.h"

#include <memory>
#include <string>
#include <map>


namespace FlatEngine
{
	class ViewportManager;

	class Mesh : public Component
	{
	public:
		Mesh(GameObject* parent, long myID = -1, long parentID = -1);
		~Mesh();
		std::string GetData();
		void CleanupTextures();
		void Cleanup();

		GameObject* GetParentPtr();
		bool Initialized();
		bool MissingTextures();
		void SetModel(Model model);
		void SetModel(std::string modelPath);
		Model& GetSceneViewModel();
		Model& GetGameViewModel();
		void CreateModelResources(VkCommandPool commandPool, PhysicalDevice& physicalDevice, LogicalDevice& logicalDevice);
		void SetMaterial(std::shared_ptr<Material> material);
		void SetMaterial(std::string materialName);
		std::shared_ptr<Material> GetSceneViewMaterial();
		std::shared_ptr<Material> GetGameViewMaterial();
		std::string GetMaterialName();
		void CreateResources();
		void AddTexture(std::string path, uint32_t index);
		void AddTexture(Texture texture, uint32_t index);
		//std::vector<Texture>& GetTextures();
		std::map<uint32_t, Texture>& GetTextures();
		void CreateTextureResources();
		std::vector<VkDescriptorSet>& GetSceneViewDescriptorSets();
		std::vector<VkDescriptorSet>& GetGameViewDescriptorSets();
		std::vector<VkDescriptorSet>& GetEmptyDescriptorSets();
		std::map<std::string, glm::vec4>& GetUBOVec4s();
		void SetUBOVec4(std::string name, Vector4 value);

	private:
		Model m_sceneViewModel; // Maybe just have individual m_uniformBuffersMapped instead of 2 separate Models
		Model m_gameViewModel;
		std::string m_materialName;
		std::shared_ptr<Material> m_sceneViewMaterial;		
		std::shared_ptr<Material> m_gameViewMaterial;
		std::map<uint32_t, Texture> m_texturesByIndex;
		std::vector<VkDescriptorSet> m_sceneViewDescriptorSets;
		std::vector<VkDescriptorSet> m_gameViewDescriptorSets;
		std::vector<VkDescriptorSet> m_emptyDescriptorSets;
		int m_allocationPoolIndex;
		bool m_b_initialized;
		bool m_b_missingTextures;

		// Uniform Buffer Data
		std::map<std::string, glm::vec4> m_uboVec4s;

		// handles
		GameObject* m_parent;
		LogicalDevice* m_logicalDevice;
	};
}