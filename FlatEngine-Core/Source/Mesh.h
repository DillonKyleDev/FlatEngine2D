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
		std::vector<Texture>& GetTextures();
		void CreateTextureResources();
		std::vector<VkDescriptorSet>& GetDescriptorSets();

		//std::map<std::string, glm::float32>& GetUBOFloats();
		//std::map<std::string, glm::vec2>& GetUBOVec2s();
		//std::map<std::string, glm::vec3>& GetUBOVec3s();
		std::map<std::string, glm::vec4>& GetUBOVec4s();
		//std::map<std::string, glm::mat4>& GetUBOMat4s();

		//void SetUBOVec4(std::string name, glm::vec4 value);
		void SetUBOVec4(std::string name, Vector4 value);


	private:
		Model m_model;
		std::string m_materialName;
		std::shared_ptr<Material> m_material;
		std::vector<Texture> m_textures;
		std::vector<VkDescriptorSet> m_descriptorSets;
		int m_allocationPoolIndex;
		bool m_b_initialized;

		// Uniform Buffer Data
		//std::map<std::string, glm::float32> m_uboFloats;
		//std::map<std::string, glm::vec2> m_uboVec2s;
		//std::map<std::string, glm::vec3> m_uboVec3s;
		std::map<std::string, glm::vec4> m_uboVec4s;
		//std::map<std::string, glm::mat4> m_uboMat4s;

		// handles
		GameObject* m_parent;
		LogicalDevice* m_logicalDevice;
	};
}