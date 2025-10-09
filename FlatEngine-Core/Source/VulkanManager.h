#pragma once
#include "ImGuiManager.h"
#include "ViewportManager.h"
#include "Structs.h"
#include "ValidationLayers.h"
#include "PhysicalDevice.h"
#include "LogicalDevice.h"
#include "WinSys.h"
#include "RenderPass.h"
#include "Mesh.h"
#include "Material.h"
#include "Vector2.h"

#include <memory>
#include <vector>
#include <map>


namespace FlatEngine
{
    // For device extensions required to present images to the window system (swap chain usage)
    const std::vector<const char*> DEVICE_EXTENSIONS =
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

#ifdef NDEBUG
    const bool b_ENABLE_VALIDATION_LAYERS = false;
#else
    const bool b_ENABLE_VALIDATION_LAYERS = true;
#endif

    extern ValidationLayers VM_validationLayers;
    const int VM_MAX_FRAMES_IN_FLIGHT = 2; // Max number of frames that are not currently being presented, but are being prepared
    extern uint32_t VM_currentFrame;
    extern uint32_t VM_imageCount;

    class Scene;

    class VulkanManager
    {
        friend class Mesh;
        friend class Application;            

    public:
        VulkanManager();
        ~VulkanManager();
        void Cleanup();

        bool Init(int windowWidth, int windowHeight);
        WinSys& GetWinSystem();
        VkInstance& GetInstance();
        VkQueue& GetGraphicsQueue();
        void DrawFrame();
        VkSampleCountFlagBits GetMaxSamples();
        void SetMaxSamples(VkSampleCountFlagBits maxSamples);

        VkCommandPool& GetCommandPool();
        PhysicalDevice& GetPhysicalDevice();
        LogicalDevice& GetLogicalDevice();        

        static void check_vk_result(VkResult err);
        static void CreateCommandPool(VkCommandPool& commandPool, LogicalDevice& logicalDevice, uint32_t queueFamilyIndices, VkCommandPoolCreateFlags flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);       

        // Materials
        void LoadEngineMaterials();
        void InitializeMaterials();        
        void SaveMaterial(std::shared_ptr<Material> material);
        std::shared_ptr<Material> LoadMaterial(std::string path, RenderPass* renderPass, bool b_init = true);
        std::shared_ptr<Material> CreateNewMaterialFile(std::string fileName, std::string path = "");        
        void AddSceneViewMaterial(std::shared_ptr<Material> material);
        void AddGameViewMaterial(std::shared_ptr<Material> material);
        void AddTextureToMaterial(std::string materialName, uint32_t index, TexturePipelineData textureData);
        void RemoveTextureFromMaterial(std::string materialName, uint32_t index = -1);
        void AddUBOVec4ToMaterial(std::string materialName, std::string uboVec4Name, int index = -1);
        void RemoveUBOVec4FromMaterial(std::string materialName, int index = -1);
        void SetMaterialVertexPath(std::string materialName, std::string vertexPath);
        void SetMaterialFragmentPath(std::string materialName, std::string fragmentPath);
        std::shared_ptr<Material> GetMaterial(std::string materialName, ViewportType viewportType = ViewportType::SceneView);
        std::map<std::string, std::shared_ptr<Material>>& GetMaterials();
        void ReloadShaders();
        void AddSceneViewMaterialMesh(std::string materialName, long ID, Mesh* mesh);
        void AddGameViewMaterialMesh(std::string materialName, long ID, Mesh* mesh);
        void RemoveSceneViewMaterialMesh(std::string materialName, long ID, Mesh* mesh);
        void RemoveGameViewMaterialMesh(std::string materialName, long ID, Mesh* mesh);
        void ClearGroupedByMaterialMeshes();
        std::shared_ptr<Model> GetModel(std::string modelPath);
        std::shared_ptr<Model> LoadModel(std::string modelPath);

        // Memory management
        void QueueBufferDeletion(VkBuffer bufferToDelete);
        void QueueDeviceMemoryDeletion(VkDeviceMemory deviceMemoryToDelete);
        void QueueDescriptorPoolDeletion(VkDescriptorPool descriptorPoolToDelete);
        void DeleteQueuedVKObjects();

        // ImGui
        void CreateImGuiRendePassResources();
        void GetImGuiDescriptorSetLayoutInfo(std::vector<VkDescriptorSetLayoutBinding>& bindings, VkDescriptorSetLayoutCreateInfo& layoutInfo);
        void GetImGuiDescriptorPoolInfo(std::vector<VkDescriptorPoolSize>& poolSizes, VkDescriptorPoolCreateInfo& poolInfo);
        void CreateImGuiResources();
        void QuitImGui();
        void CreateImGuiTexture(Texture& texture, std::vector<VkDescriptorSet>& descriptorSets);
        void FreeImGuiTexture(uint32_t allocatedFrom);        
        // Scene View
        void CreateRenderToTextureRenderPassResources(RenderPass& renderPass, Texture& renderToTexture, VkCommandPool& commandPool);
        std::vector<VkDescriptorSet>& GetSceneViewDescriptorSets();  
        void CreateSceneViewGridObjects();
        bool ShowSceneViewGridObjects();
        void SetShowSceneViewGridObjects(bool b_showGridObjects);
        void ToggleShowSceneViewGridObjects();
        void ToggleOrthographic();
        // Game View
        std::vector<VkDescriptorSet>& GetGameViewDescriptorSets();
        // Post Processing
        void CreatePostProcessingRenderPassResources();

        // WinSystem wrappers
        void CreateTextureImage(VkImage& image, std::string path, uint32_t mipLevels, VkDeviceMemory& imageMemory);
        void CreateImageView(VkImageView& imageView, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
        void CreateTextureSampler(VkSampler& textureSampler, uint32_t mipLevels);

    private:
        bool InitVulkan(int windowWidth, int windowHeight);
        bool CheckSwapChainIntegrity(VkResult result, std::string errorMessage);
        void RecreateSwapChainAndFrameBuffers();
        bool CreateVulkanInstance();
        void CreateSyncObjects();        
        
        RenderPass m_renderToTextureSceneViewRenderPass;
        RenderPass m_renderToTextureGameViewRenderPass;
        RenderPass m_postProcessingRenderPass;
        RenderPass m_imGuiRenderPass;
        std::shared_ptr<Material> m_imGuiMaterial;
        std::map<std::string, std::shared_ptr<Material>> m_sceneViewMaterials;
        std::map<std::string, std::shared_ptr<Material>> m_gameViewMaterials;
        std::map<std::string, std::map<std::string, std::map<long, Mesh*>>> m_sceneViewMaterialMeshes;
        std::map<std::string, std::map<std::string, std::map<long, Mesh*>>> m_gameViewMaterialMeshes;
        std::map<std::string, std::shared_ptr<Model>> m_models;
        Texture m_sceneViewTexture;
        Texture m_gameViewTexture;      
        Texture m_postProcessingTexture;
        std::vector<VkDescriptorSet> m_postProcessingDescriptorSets;

        bool m_b_showGridObjects;
        bool m_b_orthographic;

        VkInstance m_instance;
        WinSys m_winSystem;
        PhysicalDevice m_physicalDevice;
        VkSampleCountFlagBits m_maxSamples;
        LogicalDevice m_logicalDevice;
        bool m_b_framebufferResized;
        VkCommandPool m_systemCommandPool;
        VkCommandPool m_imGuiCommandPool;
        VkCommandPool m_sceneViewCommandPool;
        VkCommandPool m_gameViewCommandPool;
        VkCommandPool m_postProcessingCommandPool;
        std::vector<VkSemaphore> m_imageAvailableSemaphores;
        std::vector<VkSemaphore> m_renderFinishedSemaphores;
        std::vector<VkFence> m_inFlightFences;        

        std::vector<VkBuffer> m_bufferDeleteQueue;
        std::vector<VkDeviceMemory> m_deviceMemoryDeleteQueue;
        std::vector<VkDescriptorPool> m_descriptorPoolDeleteQueue;
    };
}