#include "Model.h"
#include "VulkanManager.h"
#include "FlatEngine.h"

#define TINYOBJLOADER_IMPLEMENTATION // Loading obj files
#include "tiny_obj_loader.h"

#include <stdexcept>
#include <chrono>


namespace FlatEngine
{
    Model::Model()
    {
        m_winSystem = VK_NULL_HANDLE;
        m_physicalDevice = nullptr;
        m_logicalDevice = nullptr;
        m_commandPool = nullptr;
        m_modelPath = "";
        m_vertices = std::vector<Vertex>();
        m_indices = std::vector<uint32_t>();
        m_vertexBuffer = VK_NULL_HANDLE;
        m_indexBuffer = VK_NULL_HANDLE;
        m_indexBufferMemory = VK_NULL_HANDLE;
        m_vertexBufferMemory = VK_NULL_HANDLE;
    }

    Model::~Model()
    {
    }

    void Model::CleanupIndexBuffers()
    {
        F_VulkanManager->QueueBufferDeletion(m_indexBuffer);
        F_VulkanManager->QueueDeviceMemoryDeletion(m_indexBufferMemory);
    }

    void Model::CleanupVertexBuffers()
    {
        F_VulkanManager->QueueBufferDeletion(m_vertexBuffer);
        F_VulkanManager->QueueDeviceMemoryDeletion(m_vertexBufferMemory);
    }

    void Model::Cleanup()
    {
        CleanupIndexBuffers();
        CleanupVertexBuffers();
    }


    void Model::SetModelPath(std::string path)
    {
        m_modelPath = path;
    }

    std::string Model::GetModelPath()
    {
        return m_modelPath;
    }

    void Model::Init(WinSys* winSystem, PhysicalDevice* physicalDevice, LogicalDevice* logicalDevice, VkCommandPool* commandPool)
    {
        m_winSystem = winSystem;
        m_physicalDevice = physicalDevice;
        m_logicalDevice = logicalDevice;
        m_commandPool = commandPool;
    }

    void Model::LoadModel(std::string path)
    {
        // Refer to - https://vulkan-tutorial.com/en/Loading_models

        if (path != "")
        {
            m_modelPath = path;

            tinyobj::attrib_t attrib;
            std::vector<tinyobj::shape_t> shapes;
            std::vector<tinyobj::material_t> materials;
            std::string warn, err;

            std::unordered_map<Vertex, uint32_t> uniqueVertices{};

            if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, m_modelPath.c_str()))
            {
                throw std::runtime_error(warn + err);
            }

            m_vertices.clear();
            m_indices.clear();

            for (const auto& shape : shapes)
            {
                // For mesh inputs (triangles)
                if (shape.mesh.indices.size())
                {
                    for (const auto& index : shape.mesh.indices)
                    {
                        Vertex vertex{};

                        vertex.pos =
                        {
                            attrib.vertices[3 * index.vertex_index + 0],
                            attrib.vertices[3 * index.vertex_index + 1],
                            attrib.vertices[3 * index.vertex_index + 2]
                        };

                        vertex.texCoord =
                        {
                            attrib.texcoords[2 * index.texcoord_index + 0],
                            1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                        };

                        vertex.color = { 1.0f, 1.0f, 1.0f };

                        vertex.normal =
                        {
                            attrib.normals[3 * index.normal_index + 0],
                            attrib.normals[3 * index.normal_index + 1],
                            attrib.normals[3 * index.normal_index + 2]
                        };

                        // Keep only unique vertices
                        if (uniqueVertices.count(vertex) == 0)
                        {
                            uniqueVertices[vertex] = static_cast<uint32_t>(m_vertices.size());
                            m_vertices.push_back(vertex);
                        }

                        m_indices.push_back(uniqueVertices[vertex]);

                        //m_indices.push_back(static_cast<uint32_t>(m_vertices.size()));
                        //m_vertices.push_back(vertex);
                    }
                }
                // For line inputs.. TODO: Should query topology member in PipelineManager and switch
                else
                {
                    for (const auto& index : shape.lines.indices)
                    {
                        Vertex vertex{};

                        vertex.pos =
                        {
                            attrib.vertices[3 * index.vertex_index + 0],
                            attrib.vertices[3 * index.vertex_index + 1],
                            attrib.vertices[3 * index.vertex_index + 2]
                        };

                        if (attrib.texcoords.size())
                        {
                            vertex.texCoord =
                            {
                                attrib.texcoords[2 * index.texcoord_index + 0],
                                1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                            };
                        }

                        vertex.color = { 1.0f, 1.0f, 1.0f };

                        if (attrib.normals.size())
                        {
                            vertex.normal =
                            {
                                attrib.normals[3 * index.normal_index + 0],
                                attrib.normals[3 * index.normal_index + 1],
                                attrib.normals[3 * index.normal_index + 2]
                            };
                        }

                        // Keep only unique vertices
                        if (uniqueVertices.count(vertex) == 0)
                        {
                            uniqueVertices[vertex] = static_cast<uint32_t>(m_vertices.size());
                            m_vertices.push_back(vertex);
                        }

                        m_indices.push_back(uniqueVertices[vertex]);
                    }
                }
            }

            CreateResources();
        }
    }

    void Model::CreateResources()
    {
        CreateVertexBuffer();
        CreateIndexBuffer();
    }

    void Model::CreateVertexBuffer()
    {
        // Create staging buffer for control from the cpu
        // Refer to - https://vulkan-tutorial.com/en/Vertex_buffers/Staging_buffer

        VkDeviceSize bufferSize = sizeof(m_vertices[0]) * m_vertices.size();

        VkBuffer stagingBuffer{};
        VkDeviceMemory stagingBufferMemory{};
        m_winSystem->CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(m_logicalDevice->GetDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, m_vertices.data(), (size_t)bufferSize);
        vkUnmapMemory(m_logicalDevice->GetDevice(), stagingBufferMemory);

        // Create device local vertex buffer for actual buffer
        m_winSystem->CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_vertexBuffer, m_vertexBufferMemory);

        // We can now call copyBuffer function to move the vertex data to the device local buffer:
        m_winSystem->CopyBuffer(stagingBuffer, m_vertexBuffer, bufferSize, *m_commandPool);

        // After copying the data from the staging buffer to the device buffer, we should clean it up:
        vkDestroyBuffer(m_logicalDevice->GetDevice(), stagingBuffer, nullptr);
        vkFreeMemory(m_logicalDevice->GetDevice(), stagingBufferMemory, nullptr);
    }

    void Model::CreateIndexBuffer()
    {
        // Refer to - https://vulkan-tutorial.com/en/Vertex_buffers/Index_buffer

        VkDeviceSize bufferSize = sizeof(m_indices[0]) * m_indices.size();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        m_winSystem->CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(m_logicalDevice->GetDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, m_indices.data(), (size_t)bufferSize);
        vkUnmapMemory(m_logicalDevice->GetDevice(), stagingBufferMemory);

        m_winSystem->CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_indexBuffer, m_indexBufferMemory);

        m_winSystem->CopyBuffer(stagingBuffer, m_indexBuffer, bufferSize, *m_commandPool);

        vkDestroyBuffer(m_logicalDevice->GetDevice(), stagingBuffer, nullptr);
        vkFreeMemory(m_logicalDevice->GetDevice(), stagingBufferMemory, nullptr);
    }

    VkBuffer& Model::GetVertexBuffer()
    {
        return m_vertexBuffer;
    }

    VkBuffer& Model::GetIndexBuffer()
    {
        return m_indexBuffer;
    }

    std::vector<uint32_t> Model::GetIndices()
    {
        return m_indices;
    }
}