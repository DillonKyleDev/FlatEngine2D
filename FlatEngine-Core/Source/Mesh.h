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
		Model& GetModel();
		void CreateModelResources(VkCommandPool commandPool, PhysicalDevice& physicalDevice, LogicalDevice& logicalDevice);
		void SetMaterial(std::shared_ptr<Material> material);
		void SetMaterial(std::string materialName);
		std::shared_ptr<Material> GetMaterial();
		std::string GetMaterialName();
		void CreateResources();
		void AddTexture(std::string path, uint32_t index);
		void AddTexture(Texture texture, uint32_t index);
		//std::vector<Texture>& GetTextures();
		std::map<uint32_t, Texture>& GetTextures();
		void CreateTextureResources();
		std::vector<VkDescriptorSet>& GetDescriptorSets();
		std::vector<VkDescriptorSet>& GetEmptyDescriptorSets();
		std::map<std::string, glm::vec4>& GetUBOVec4s();
		void SetUBOVec4(std::string name, Vector4 value);

	private:
		Model m_model;
		std::string m_materialName;
		std::shared_ptr<Material> m_material;		
		std::map<uint32_t, Texture> m_texturesByIndex;
		std::vector<VkDescriptorSet> m_descriptorSets;
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