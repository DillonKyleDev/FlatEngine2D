#pragma once
#include "Structs.h"
#include "PhysicalDevice.h"
#include "LogicalDevice.h"
#include "WinSys.h"

#include <vector>


namespace FlatEngine
{
	class Mesh;

	class Model
	{
	public:
		Model();
		~Model();
		void CleanupIndexBuffers();
		void CleanupVertexBuffers();
		void Cleanup();

		void SetModelPath(std::string path);
		std::string GetModelPath();
		void Init(WinSys* winSystem, PhysicalDevice* physicalDevice, LogicalDevice* logicalDevice, VkCommandPool* commandPool );
		void LoadModel(std::string path);
		void CreateResources();
		void CreateVertexBuffer();
		void CreateIndexBuffer();
		VkBuffer& GetVertexBuffer();
		VkBuffer& GetIndexBuffer();
		std::vector<uint32_t> GetIndices();

	private:
		std::string m_modelPath;
		std::vector<Vertex> m_vertices;
		std::vector<uint32_t> m_indices;
		VkBuffer m_vertexBuffer;
		VkBuffer m_indexBuffer;
		VkDeviceMemory m_vertexBufferMemory;
		VkDeviceMemory m_indexBufferMemory;
		WinSys* m_winSystem;
		PhysicalDevice* m_physicalDevice;
		LogicalDevice* m_logicalDevice;
		VkCommandPool* m_commandPool;
	};
}

