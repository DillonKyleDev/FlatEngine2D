#pragma once
#include "VulkanManager.h"
#include "Helper.h"
#include "Material.h"
#include "Project.h"
#include "FlatEngine.h"
#include "ThreadPool.h"

#include "SDL_vulkan.h"
#include <glm.hpp>

// Refer to - https://vulkan-tutorial.com/en/Uniform_buffers/Descriptor_layout_and_buffer
#include <gtc/matrix_transform.hpp> // Not used currently but might need it later

#include <chrono> // Time keeping
#include <memory>
#include <array>
#include <fstream>
#include <thread>
#include <list>

// TODO: Remove PipelineManager class and move RenderPasses into Materials alongside GraphicsPipeline to be controlled by each Material. 

namespace FlatEngine
{
    GameObject* F_sceneViewCameraObject = nullptr;
    std::string F_selectedMaterialName = "";
    ValidationLayers VM_validationLayers = ValidationLayers();
    uint32_t VM_currentFrame = 0;
    uint32_t VM_imageCount = 0;
    Scene F_sceneViewGridObjects = Scene();

    void VulkanManager::check_vk_result(VkResult err)
    {
        if (err == 0)
        {
            return;
        }
        fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
        if (err < 0)
        {
            abort();
        }
    }

    VulkanManager::VulkanManager()
    {
        m_instance = VK_NULL_HANDLE;
        m_winSystem = WinSys();
        m_physicalDevice = PhysicalDevice();
        m_logicalDevice = LogicalDevice();       

        m_renderToTextureSceneViewRenderPass = RenderPass();
        m_renderToTextureGameViewRenderPass = RenderPass();
        m_imGuiRenderPass = RenderPass();

        // gpu communication
        m_systemCommandPool = VK_NULL_HANDLE;
        m_imGuiCommandPool = VK_NULL_HANDLE;
        m_sceneViewCommandPool = VK_NULL_HANDLE;
        m_gameViewCommandPool = VK_NULL_HANDLE;
        m_imageAvailableSemaphores = std::vector<VkSemaphore>();
        m_renderFinishedSemaphores = std::vector<VkSemaphore>();
        m_inFlightFences = std::vector<VkFence>();
        m_b_framebufferResized = false; 

        m_imGuiMaterial = std::shared_ptr<Material>();
        m_sceneViewMaterials = std::map<std::string, std::shared_ptr<Material>>();
        m_gameViewMaterials = std::map<std::string, std::shared_ptr<Material>>();
        m_sceneViewMaterialMeshes = std::map<std::string, std::map<long, Mesh*>>();
        m_gameViewMaterialMeshes = std::map<std::string, std::map<long, Mesh*>>();
        m_sceneViewTexture = Texture();
        m_gameViewTexture = Texture();
        m_models = std::map<std::string, std::shared_ptr<Model>>();

        m_bufferDeleteQueue= std::vector<VkBuffer>();
        m_deviceMemoryDeleteQueue = std::vector<VkDeviceMemory>();
        m_descriptorPoolDeleteQueue = std::vector<VkDescriptorPool>();

        m_b_showGridObjects = true;
        m_b_orthographic = false;
    }

    VulkanManager::~VulkanManager()
    {
    }

    void VulkanManager::Cleanup()
    {
        QuitImGui();

        vkDeviceWaitIdle(m_logicalDevice.GetDevice()); // This may need to be moved elsewhere potentially

        // Semaphores and Fences
        for (size_t i = 0; i < VM_MAX_FRAMES_IN_FLIGHT; i++)
        {
            vkDestroySemaphore(m_logicalDevice.GetDevice(), m_imageAvailableSemaphores[i], nullptr);
            vkDestroySemaphore(m_logicalDevice.GetDevice(), m_renderFinishedSemaphores[i], nullptr);
            vkDestroyFence(m_logicalDevice.GetDevice(), m_inFlightFences[i], nullptr);
        }

        m_winSystem.CleanupDrawingResources();

        for (std::map<std::string, std::shared_ptr<Material>>::iterator material = m_sceneViewMaterials.begin(); material != m_sceneViewMaterials.end(); material++)
        {
            material->second->Cleanup();
        }
        for (std::map<std::string, std::shared_ptr<Material>>::iterator material = m_gameViewMaterials.begin(); material != m_gameViewMaterials.end(); material++)
        {
            material->second->Cleanup();
        }

        vkDestroyCommandPool(m_logicalDevice.GetDevice(), m_systemCommandPool, nullptr);
        vkDestroyCommandPool(m_logicalDevice.GetDevice(), m_imGuiCommandPool, nullptr);
        vkDestroyCommandPool(m_logicalDevice.GetDevice(), m_sceneViewCommandPool, nullptr);
        vkDestroyCommandPool(m_logicalDevice.GetDevice(), m_gameViewCommandPool, nullptr);

        m_logicalDevice.Cleanup();
        m_physicalDevice.Cleanup();
        VM_validationLayers.Cleanup(m_instance);
        m_winSystem.CleanupSystem();

        m_renderToTextureSceneViewRenderPass.Cleanup(m_logicalDevice);
        m_renderToTextureGameViewRenderPass.Cleanup(m_logicalDevice);
        m_imGuiRenderPass.Cleanup(m_logicalDevice);

        // Destroy Vulkan instance
        vkDestroyInstance(m_instance, nullptr);
    }

    bool VulkanManager::Init(int width, int height)
    {
        bool b_success = true;

        if (!InitVulkan(width, height))
        {
            printf("Vulkan initialization failed!\n");
            b_success = false;
        }
        else
        {
            printf("Vulkan initialized...\n");
        }

        return b_success;
    }

    void VulkanManager::CreateTextureImage(VkImage& image, std::string path, uint32_t mipLevels, VkDeviceMemory& imageMemory)
    {
        image = m_winSystem.CreateTextureImage(path, mipLevels, imageMemory, m_systemCommandPool);
    }

    void VulkanManager::CreateImageView(VkImageView& imageView, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels)
    {
        m_winSystem.CreateImageView(imageView, image, format, aspectFlags, mipLevels);
    }

    void VulkanManager::CreateTextureSampler(VkSampler& textureSampler, uint32_t mipLevels)
    {
        m_winSystem.CreateTextureSampler(textureSampler, mipLevels);
    }

    bool VulkanManager::InitVulkan(int windowWidth, int windowHeight)
    {
        bool b_success = true;

        if (!m_winSystem.CreateSDLWindow("FlatEngine", windowWidth, windowHeight))
        {
            printf("SDL window creation failed!\n");
            b_success = false;
        }
        else
        {
            if (!CreateVulkanInstance())
            {
                printf("Failed to create Vulkan instance!\n");
                b_success = false;
            }
            else
            {
                m_winSystem.SetHandles(&m_instance, &m_physicalDevice, &m_logicalDevice, &m_systemCommandPool);
                m_winSystem.CreateSurface();
                VM_validationLayers.SetupDebugMessenger(m_instance);
                m_physicalDevice.Init(m_instance, m_winSystem.GetSurface());
                m_logicalDevice.Init(m_physicalDevice, m_winSystem.GetSurface());
                m_winSystem.CreateDrawingResources();
                QueueFamilyIndices indices = Helper::FindQueueFamilies(m_physicalDevice.GetDevice(), m_winSystem.GetSurface());
                m_logicalDevice.SetGraphicsIndex(indices.graphicsFamily.value());
                CreateCommandPool(m_systemCommandPool, m_logicalDevice, indices.graphicsFamily.value());
                CreateCommandPool(m_imGuiCommandPool, m_logicalDevice, indices.graphicsFamily.value());
                CreateCommandPool(m_sceneViewCommandPool, m_logicalDevice, indices.graphicsFamily.value());
                CreateCommandPool(m_gameViewCommandPool, m_logicalDevice, indices.graphicsFamily.value());                
                CreateSyncObjects();

                m_sceneViewTexture.CreateRenderToTextureResources(m_sceneViewCommandPool);
                m_gameViewTexture.CreateRenderToTextureResources(m_gameViewCommandPool);
                
                CreateRenderToTextureRenderPassResources(m_renderToTextureSceneViewRenderPass, m_sceneViewTexture, m_sceneViewCommandPool);
                CreateRenderToTextureRenderPassResources(m_renderToTextureGameViewRenderPass, m_gameViewTexture, m_gameViewCommandPool);
                CreateImGuiRendePassResources();

                m_imGuiMaterial = LoadMaterial("../engine/materials/fl_imgui.mat", &m_imGuiRenderPass, false);
                CreateImGuiResources();
                m_imGuiMaterial->Init();

                LoadEngineMaterials();

                CreateSceneViewGridObjects();
            }
        }

        return b_success;
    }

    bool VulkanManager::CreateVulkanInstance()
    {
        bool b_success = true;

        // Validation layer setup for debugger
        if (b_ENABLE_VALIDATION_LAYERS && !VM_validationLayers.CheckSupport())
        {
            printf("Error: Validation layers requested, but not available.\n");
            b_success = false;
        }

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "FlatEngine";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "FlatEngine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        uint32_t sdlExtensionCount = 0;
        const char** sdlExtensionNames;
        SDL_Vulkan_GetInstanceExtensions(m_winSystem.GetWindow(), &sdlExtensionCount, nullptr);
        sdlExtensionNames = new const char* [sdlExtensionCount];
        SDL_Vulkan_GetInstanceExtensions(m_winSystem.GetWindow(), &sdlExtensionCount, sdlExtensionNames);

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{}; // DebugUtilsMessenger for CreateInstance and DestroyInstance functions (automatically destroyed by Vulkan when closed)

        // Get extensions for use with debug messenger
        auto validationExtensions = VM_validationLayers.GetRequiredExtensions();
        uint32_t totalExtensionCount = (uint32_t)(sdlExtensionCount + validationExtensions.size());
        const char** extensionNames = new const char* [totalExtensionCount];
        for (uint32_t i = 0; i < sdlExtensionCount; i++)
        {
            extensionNames[i] = sdlExtensionNames[i];
        }
        for (uint32_t i = 0; i < validationExtensions.size(); i++)
        {
            extensionNames[sdlExtensionCount + i] = validationExtensions[i];
        }
        createInfo.enabledExtensionCount = totalExtensionCount;
        createInfo.ppEnabledExtensionNames = &extensionNames[0];

        if (b_ENABLE_VALIDATION_LAYERS)
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(VM_validationLayers.Size());
            createInfo.ppEnabledLayerNames = VM_validationLayers.Data();
            VM_validationLayers.PopulateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
        }
        else
        {
            createInfo.enabledLayerCount = 0;
            createInfo.pNext = nullptr;
        }

        if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS)
        {
            printf("Failed to create Vulkan instance...\n");
            b_success = false;
        }

        return b_success;
    }

    void VulkanManager::CreateCommandPool(VkCommandPool& commandPool, LogicalDevice& logicalDevice, uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags)
    {
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = flags;
        poolInfo.queueFamilyIndex = queueFamilyIndex;

        if (vkCreateCommandPool(logicalDevice.GetDevice(), &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create command pool!");
        }
    }

    void VulkanManager::CreateRenderToTextureRenderPassResources(RenderPass& renderPass, Texture& renderToTexture, VkCommandPool& commandPool)
    {
        renderPass.SetHandles(&m_instance, &m_winSystem, &m_physicalDevice, &m_logicalDevice, &commandPool);

        renderPass.EnableDepthBuffering();
        renderPass.EnableMsaa();
        VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT; // F_VulkanManager->GetMaxSamples();
        VkFormat colorFormat = VK_FORMAT_R32G32B32A32_SFLOAT;
        renderPass.SetImageColorFormat(colorFormat);
        renderPass.SetMSAASampleCount(msaaSamples);

        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = colorFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        renderPass.AddRenderPassAttachment(colorAttachment, colorAttachmentRef);

        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = Helper::FindDepthFormat(m_physicalDevice.GetDevice());
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        renderPass.AddRenderPassAttachment(depthAttachment, depthAttachmentRef);

        VkAttachmentDescription colorAttachmentResolve{};
        colorAttachmentResolve.format = colorFormat;
        colorAttachmentResolve.samples = msaaSamples;
        colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        VkAttachmentReference colorAttachmentResolveRef{};
        colorAttachmentResolveRef.attachment = 2;
        colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        renderPass.AddRenderPassAttachment(colorAttachmentResolve, colorAttachmentResolveRef);
        
        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        renderPass.AddSubpassDependency(dependency);

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &renderPass.GetAttachmentRefs()[0];
        subpass.pDepthStencilAttachment = &renderPass.GetAttachmentRefs()[1];
        subpass.pResolveAttachments = &renderPass.GetAttachmentRefs()[2];
        renderPass.AddSubpass(subpass);

        renderPass.ConfigureFrameBufferImageViews(renderToTexture.GetImageViews()); // Give m_renderPass the VkImageViews to write to their VkImages (to be used later by ImGui material)		

        renderPass.Init();
    }

    void VulkanManager::CreateImGuiRendePassResources()
    {
        m_imGuiRenderPass.SetHandles(&m_instance, &m_winSystem, &m_physicalDevice, &m_logicalDevice, &m_imGuiCommandPool);

        m_imGuiRenderPass.EnableMsaa();
        VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT; // F_VulkanManager->GetMaxSamples();
        VkFormat colorFormat = VK_FORMAT_B8G8R8A8_UNORM;
        m_imGuiRenderPass.SetImageColorFormat(colorFormat);
        m_imGuiRenderPass.SetMSAASampleCount(msaaSamples);

        VkAttachmentDescription colorAttachment = {};
        colorAttachment.format = m_winSystem.GetImageFormat();
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        VkAttachmentReference colorAttachmentRef = {};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        m_imGuiRenderPass.AddRenderPassAttachment(colorAttachment, colorAttachmentRef);

        VkAttachmentDescription colorAttachmentResolve{};
        colorAttachmentResolve.format = colorFormat;
        colorAttachmentResolve.samples = msaaSamples;
        colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        VkAttachmentReference colorAttachmentResolveRef{};
        colorAttachmentResolveRef.attachment = 1;
        colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        m_imGuiRenderPass.AddRenderPassAttachment(colorAttachmentResolve, colorAttachmentResolveRef);
        
        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        m_imGuiRenderPass.AddSubpassDependency(dependency);

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
        subpass.pResolveAttachments = &colorAttachmentResolveRef;
        m_imGuiRenderPass.AddSubpass(subpass);

        m_imGuiRenderPass.ConfigureFrameBufferImageViews(m_winSystem.GetSwapChainImageViews());

        m_imGuiRenderPass.Init();
    }

    void VulkanManager::GetImGuiDescriptorSetLayoutInfo(std::vector<VkDescriptorSetLayoutBinding>& bindings, VkDescriptorSetLayoutCreateInfo& layoutInfo)
    {
        bindings.resize(1);
        bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        bindings[0].descriptorCount = 1;
        bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        bindings[0].binding = 0;
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();
    }

    void VulkanManager::GetImGuiDescriptorPoolInfo(std::vector<VkDescriptorPoolSize>& poolSizes, VkDescriptorPoolCreateInfo& poolInfo)
    {
        poolSizes =
        {
            { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
        };
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        poolInfo.maxSets = 1000;
        poolInfo.poolSizeCount = (uint32_t)poolSizes.size();
        poolInfo.pPoolSizes = poolSizes.data();
    }

    void VulkanManager::CreateImGuiResources()
    {
        // https://frguthmann.github.io/posts/vulkan_imgui/       

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;// | ImGuiConfigFlags_ViewportsEnable;
        //io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports | ImGuiBackendFlags_RendererHasViewports;

        VulkanManager::CreateCommandPool(m_imGuiCommandPool, m_logicalDevice, m_logicalDevice.GetGraphicsIndex(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

        // Set up Descriptor Material Allocator
        std::vector<VkDescriptorSetLayoutBinding> bindings{};
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        GetImGuiDescriptorSetLayoutInfo(bindings, layoutInfo);
        std::vector<VkDescriptorPoolSize> poolSizes{};
        VkDescriptorPoolCreateInfo poolInfo{};
        GetImGuiDescriptorPoolInfo(poolSizes, poolInfo);

        m_imGuiMaterial->GetAllocator().ConfigureDescriptorSetLayout(bindings, layoutInfo);
        m_imGuiMaterial->GetAllocator().ConfigureDescriptorPools(poolSizes, poolInfo);
        m_imGuiMaterial->AddTexture(0, VK_SHADER_STAGE_FRAGMENT_BIT);
        m_imGuiMaterial->GetAllocator().Init(AllocatorType::DescriptorPool, m_imGuiMaterial->GetTexturesShaderStages(), m_logicalDevice);

        ImGui_ImplSDL2_InitForVulkan(m_winSystem.GetWindow());

        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = m_instance;
        init_info.PhysicalDevice = m_physicalDevice.GetDevice();
        init_info.Device = m_logicalDevice.GetDevice();
        init_info.QueueFamily = ImGui_ImplVulkanH_SelectQueueFamilyIndex(m_physicalDevice.GetDevice());
        init_info.Queue = m_logicalDevice.GetGraphicsQueue();
        init_info.PipelineCache = VK_NULL_HANDLE;
        init_info.DescriptorPool = m_imGuiMaterial->CreateDescriptorPool();
        init_info.RenderPass = m_imGuiRenderPass.GetRenderPass();
        init_info.Subpass = 0;
        init_info.MinImageCount = VM_imageCount;
        init_info.ImageCount = VM_imageCount;
        init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        init_info.Allocator = nullptr;
        init_info.CheckVkResultFn = VulkanManager::check_vk_result;

        if (!ImGui_ImplVulkan_Init(&init_info))
        {
            FlatEngine::LogError("ImGui backends setup failed!");
        }
    }

    void VulkanManager::CreateImGuiTexture(Texture& texture, std::vector<VkDescriptorSet>& descriptorSets)
    {
        texture.CreateTextureImage();
        Model emptyModel = Model();
        std::map<uint32_t, Texture> textures = std::map<uint32_t, Texture>();
        textures.emplace(0, texture);
        m_imGuiMaterial->CreateDescriptorSets(descriptorSets, emptyModel, textures);
    }

    void VulkanManager::FreeImGuiTexture(uint32_t allocatedFrom)
    {
        m_imGuiMaterial->GetAllocator().SetFreed(allocatedFrom);
    }

    void VulkanManager::QuitImGui()
    {
        vkDestroyCommandPool(m_logicalDevice.GetDevice(), m_imGuiCommandPool, nullptr);

        VkResult err = vkDeviceWaitIdle(m_logicalDevice.GetDevice());
        VulkanManager::check_vk_result(err);
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
    }

    void VulkanManager::LoadEngineMaterials()
    {
        // TODO: Remove m_renderToTexture reference
        AddSceneViewMaterial(LoadMaterial("../engine/materials/fl_empty.mat", &m_renderToTextureSceneViewRenderPass));
        AddGameViewMaterial(LoadMaterial("../engine/materials/fl_empty.mat", &m_renderToTextureGameViewRenderPass));
        AddSceneViewMaterial(LoadMaterial("../engine/materials/fl_unlit.mat", &m_renderToTextureSceneViewRenderPass));
        AddGameViewMaterial(LoadMaterial("../engine/materials/fl_unlit.mat", &m_renderToTextureGameViewRenderPass));
        AddSceneViewMaterial(LoadMaterial("../engine/materials/fl_verticesOnly.mat", &m_renderToTextureSceneViewRenderPass));
        AddGameViewMaterial(LoadMaterial("../engine/materials/fl_verticesOnly.mat", &m_renderToTextureGameViewRenderPass));
        AddSceneViewMaterial(LoadMaterial("../engine/materials/fl_xAxis.mat", &m_renderToTextureSceneViewRenderPass));
        AddGameViewMaterial(LoadMaterial("../engine/materials/fl_xAxis.mat", &m_renderToTextureGameViewRenderPass));
        AddSceneViewMaterial(LoadMaterial("../engine/materials/fl_yAxis.mat", &m_renderToTextureSceneViewRenderPass));
        AddGameViewMaterial(LoadMaterial("../engine/materials/fl_yAxis.mat", &m_renderToTextureGameViewRenderPass));
        AddSceneViewMaterial(LoadMaterial("../engine/materials/fl_zAxis.mat", &m_renderToTextureSceneViewRenderPass));
        AddGameViewMaterial(LoadMaterial("../engine/materials/fl_zAxis.mat", &m_renderToTextureGameViewRenderPass));
        AddSceneViewMaterial(LoadMaterial("../engine/materials/fl_uv.mat", &m_renderToTextureSceneViewRenderPass));
        AddGameViewMaterial(LoadMaterial("../engine/materials/fl_uv.mat", &m_renderToTextureGameViewRenderPass));
    }

    void VulkanManager::InitializeMaterials()
    {
        m_sceneViewMaterials.clear();
        m_gameViewMaterials.clear();

        LoadEngineMaterials();

        std::vector<std::string> materialFiles = std::vector<std::string>();
        materialFiles = FindAllFilesWithExtension(GetDir("projectDir"), ".mat");

        for (std::string path : materialFiles)
        {
            AddSceneViewMaterial(LoadMaterial(path, &m_renderToTextureSceneViewRenderPass));
            AddGameViewMaterial(LoadMaterial(path, &m_renderToTextureGameViewRenderPass));
        }
    }

    void VulkanManager::SaveMaterial(std::shared_ptr<Material> material)
    {
        std::string filepath = material->GetPath();

        std::ofstream fileObject;
        std::ifstream ifstream(filepath);

        // Delete old contents of the file
        fileObject.open(filepath, std::ofstream::out | std::ofstream::trunc);
        fileObject.close();

        // Opening file in append mode
        fileObject.open(filepath, std::ios::app);

        std::string data = material->GetData();

        fileObject << data.c_str() << std::endl;
        fileObject.close();

        std::map<long, Mesh>& meshes = GetMeshes();
        for (std::map<long, Mesh>::iterator iter = meshes.begin(); iter != meshes.end(); iter++)
        {
            if (iter->second.GetMaterialName() == material->GetName())
            {
                iter->second.CreateResources();
            }
        }
    }

    std::shared_ptr<Material> VulkanManager::LoadMaterial(std::string path, RenderPass* renderPass, bool b_init)
    {
        std::shared_ptr<Material> newMaterial = std::make_shared<Material>();

        json materialData = LoadFileData(path);
        if (materialData != nullptr)
        {
            std::string name = CheckJsonString(materialData, "name", "Material");
            newMaterial->SetName(name);
            newMaterial->SetPath(path);

            if (name == "")
            {
                newMaterial->SetName(GetFilenameFromPath(path));
            }
         
            std::string vertexShaderPath = CheckJsonString(materialData, "vertexShaderPath", name);
            if (vertexShaderPath != "")
            {
                newMaterial->SetVertexPath(vertexShaderPath);
            }
            std::string fragmentShaderPath = CheckJsonString(materialData, "fragmentShaderPath", name);
            if (fragmentShaderPath != "")
            {
                newMaterial->SetFragmentPath(fragmentShaderPath);
            }

            if (JsonContains(materialData, "uboVec4Names", name))
            {
                json uboVec4Data = materialData["uboVec4Names"];

                if (uboVec4Data.size())
                {
                    for (auto item = uboVec4Data.begin(); item != uboVec4Data.end(); ++item)
                    {
                        try
                        {
                            newMaterial->AddUBOVec4(item.value(), (uint32_t)std::stoi(item.key()));
                        }
                        catch (const json::out_of_range& e)
                        {
                            LogError(e.what());
                        }
                    }
                }
            }

            if (JsonContains(materialData, "texturesShaderStageData", name))
            {
                json texturesShaderData = materialData["texturesShaderStageData"];

                if (texturesShaderData.size())
                {
                    for (auto item = texturesShaderData.begin(); item != texturesShaderData.end(); ++item)
                    {
                        try
                        {
                            newMaterial->AddTexture((uint32_t)std::stoi(item.key()), (VkShaderStageFlags)item.value());
                        }
                        catch (const json::out_of_range& e)
                        {
                            LogError(e.what());
                        }
                    }
                }
            }

            // Graphics Pipeline configuration
            if (JsonContains(materialData, "inputAssemblyData", name))
            {
                json inputAssemblyData = materialData["inputAssemblyData"];

                VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfos = newMaterial->GetInputAssemblyCreateInfos(); // Maybe just use initialized VkPipelineInputAssemblyStateCreateInfo instead
                inputAssemblyInfos.topology = (VkPrimitiveTopology)CheckJsonInt(inputAssemblyData, "topology", name);
                inputAssemblyInfos.primitiveRestartEnable = (bool)CheckJsonBool(inputAssemblyData, "_primitiveRestartEnable", name);
                newMaterial->SetInputAssemblyCreateInfos(inputAssemblyInfos);
            }
            if (JsonContains(materialData, "rasterizerData", name))
            {                
                json rasterizerData = materialData["rasterizerData"];

                VkPipelineRasterizationStateCreateInfo rasterizerInfos = newMaterial->GetRasterizerCreateInfos();
                rasterizerInfos.polygonMode = (VkPolygonMode)CheckJsonInt(rasterizerData, "polygonMode", name);
                rasterizerInfos.cullMode = (VkCullModeFlagBits)CheckJsonInt(rasterizerData, "cullMode", name);
                rasterizerInfos.lineWidth = CheckJsonFloat(rasterizerData, "lineWidth", name);
                newMaterial->SetRasterizerCreateInfos(rasterizerInfos);
            }
            if (JsonContains(materialData, "colorBlendAttachmentData", name))
            {
                json colorBlendAttachmentData = materialData["colorBlendAttachmentData"];

                VkPipelineColorBlendAttachmentState colorBlendAttachmentInfos = newMaterial->GetColorBlendAttachmentCreateInfos();
                colorBlendAttachmentInfos.alphaBlendOp = (VkBlendOp)CheckJsonInt(colorBlendAttachmentData, "alphaBlendOp", name);
                newMaterial->SetColorBlendAttachmentCreateInfos(colorBlendAttachmentInfos);                
            }

            newMaterial->SetHandles(&m_instance, &m_winSystem, &m_physicalDevice, &m_logicalDevice, renderPass);

            if (b_init)
            {
                newMaterial->Init();
            }            
        }

        return newMaterial;
    }

    std::shared_ptr<Material> VulkanManager::CreateNewMaterialFile(std::string fileName, std::string path)
    {
        std::string filePath = "";
        std::shared_ptr<Material> newMaterial = std::make_shared<Material>();

        if (path == "")
        {
            filePath = GetDir("projectDir") + "/materials/" + fileName + ".mat";
        }
        else
        {
            filePath = path + "/" + fileName + ".mat";
        }

        // TODO: SHOULD ADD THE MATERIAL TO BOTH SCENE VIEW AND GAME VIEW MATERIALS
        newMaterial->SetPath(filePath);
        newMaterial->SetName(fileName);
        newMaterial->SetHandles(&m_instance, &m_winSystem, &m_physicalDevice, &m_logicalDevice, &m_renderToTextureSceneViewRenderPass);
        SaveMaterial(newMaterial);        

        return newMaterial;
    }

    // Only call add material after all material members have been filled
    void VulkanManager::AddSceneViewMaterial(std::shared_ptr<Material> material)
    {        
        m_sceneViewMaterials.emplace(material->GetName(), material);
    }

    void VulkanManager::AddGameViewMaterial(std::shared_ptr<Material> material)
    {        
        m_gameViewMaterials.emplace(material->GetName(), material);
    }

    std::shared_ptr<Material> VulkanManager::GetMaterial(std::string materialName, ViewportType viewportType)
    {                
        switch (viewportType)
        {
        case ViewportType::SceneView:
            if (m_sceneViewMaterials.count(materialName))
            {
                return m_sceneViewMaterials.at(materialName);
            }
            break;
        case ViewportType::GameView:
            if (m_gameViewMaterials.count(materialName))
            {
                return m_gameViewMaterials.at(materialName);
            }
            break;
        default:
            break;
        }

        if (materialName == "imgui")
        {
            return m_imGuiMaterial;
        }

        return nullptr;
    }

    // Assume Scene View Materials for now
    std::map<std::string, std::shared_ptr<Material>>& VulkanManager::GetMaterials()
    {
        return m_sceneViewMaterials;
    }

    void VulkanManager::ReloadShaders()
    {
        for (std::map<std::string, std::shared_ptr<Material>>::iterator material = m_sceneViewMaterials.begin(); material != m_sceneViewMaterials.end(); material++)
        {
            if (material->second->Initialized())
            {
                material->second->RecreateGraphicsPipeline();
            }
        }
        for (std::map<std::string, std::shared_ptr<Material>>::iterator material = m_gameViewMaterials.begin(); material != m_gameViewMaterials.end(); material++)
        {
            if (material->second->Initialized())
            {
                material->second->RecreateGraphicsPipeline();
            }
        }
    }

    void VulkanManager::AddSceneViewMaterialMesh(std::string materialName, long ID, Mesh* mesh)
    {
        if (m_sceneViewMaterialMeshes.count(materialName) && !m_sceneViewMaterialMeshes.at(materialName).count(ID))
        {
            m_sceneViewMaterialMeshes.at(materialName).emplace(ID, mesh);
        }
        else
        {
            std::map<long, Mesh*> newMap = std::map<long, Mesh*>();
            newMap.emplace(ID, mesh);
            m_sceneViewMaterialMeshes.emplace(materialName, newMap);
        }
    }

    void VulkanManager::AddGameViewMaterialMesh(std::string materialName, long ID, Mesh* mesh)
    {
        if (m_gameViewMaterialMeshes.count(materialName) && !m_gameViewMaterialMeshes.at(materialName).count(ID))
        {
            m_gameViewMaterialMeshes.at(materialName).emplace(ID, mesh);
        }
        else
        {
            std::map<long, Mesh*> newMap = std::map<long, Mesh*>();
            newMap.emplace(ID, mesh);
            m_gameViewMaterialMeshes.emplace(materialName, newMap);
        }
    }

    void VulkanManager::RemoveSceneViewMaterialMesh(std::string materialName, long ID, Mesh* mesh)
    {
        if (m_sceneViewMaterialMeshes.count(materialName) && m_sceneViewMaterialMeshes.at(materialName).count(ID))
        {
            m_sceneViewMaterialMeshes.at(materialName).erase(ID);
        }
    }

    void VulkanManager::RemoveGameViewMaterialMesh(std::string materialName, long ID, Mesh* mesh)
    {
        if (m_gameViewMaterialMeshes.count(materialName) && m_gameViewMaterialMeshes.at(materialName).count(ID))
        {
            m_gameViewMaterialMeshes.at(materialName).erase(ID);
        }
    }

    void VulkanManager::ClearGroupedByMaterialMeshes()
    {
        m_sceneViewMaterialMeshes.clear();
        m_gameViewMaterialMeshes.clear();
    }

    std::shared_ptr<Model> VulkanManager::GetModel(std::string modelPath)
    {
        if (m_models.count(modelPath))
        {
            return m_models.at(modelPath);
        }
        else
        {
            return nullptr;
        }
    }

    std::shared_ptr<Model> VulkanManager::LoadModel(std::string modelPath)
    {
        std::shared_ptr<Model> newModel = std::make_shared<Model>();
        newModel->SetModelPath(modelPath);
        newModel->LoadModel();
        m_models.emplace(modelPath, newModel);

        return newModel;
    }

    void VulkanManager::QueueBufferDeletion(VkBuffer bufferToDelete)
    {
        m_bufferDeleteQueue.push_back(bufferToDelete);
    }

    void VulkanManager::QueueDeviceMemoryDeletion(VkDeviceMemory deviceMemoryToDelete)
    {
        m_deviceMemoryDeleteQueue.push_back(deviceMemoryToDelete);
    }

    void VulkanManager::QueueDescriptorPoolDeletion(VkDescriptorPool descriptorPoolToDelete)
    {
        m_descriptorPoolDeleteQueue.push_back(descriptorPoolToDelete);;
    }

    void VulkanManager::DeleteQueuedVKObjects()
    {
        if (m_bufferDeleteQueue.size())
        {
            for (VkBuffer buffer : m_bufferDeleteQueue)
            {
                vkDestroyBuffer(m_logicalDevice.GetDevice(), buffer, nullptr);
            }
            m_bufferDeleteQueue.clear();
        }

        if (m_deviceMemoryDeleteQueue.size())
        {
            for (VkDeviceMemory deviceMemory : m_deviceMemoryDeleteQueue)
            {
                vkFreeMemory(m_logicalDevice.GetDevice(), deviceMemory, nullptr);
            }
            m_deviceMemoryDeleteQueue.clear();
        }

        if (m_descriptorPoolDeleteQueue.size())
        {
            for (VkDescriptorPool descriptorPool : m_descriptorPoolDeleteQueue)
            {
                vkDestroyDescriptorPool(m_logicalDevice.GetDevice(), descriptorPool, nullptr);
            }
            m_descriptorPoolDeleteQueue.clear();
        }
    }

    void VulkanManager::CreateSyncObjects()
    {
        // More info here - https://vulkan-tutorial.com/en/Drawing_a_triangle/Drawing/Rendering_and_presentation

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // Start it signaled so the very first frame doesn't block indefinitely

        m_imageAvailableSemaphores.resize(VM_MAX_FRAMES_IN_FLIGHT);
        m_renderFinishedSemaphores.resize(VM_MAX_FRAMES_IN_FLIGHT);
        m_inFlightFences.resize(VM_MAX_FRAMES_IN_FLIGHT);

        for (size_t i = 0; i < VM_MAX_FRAMES_IN_FLIGHT; i++)
        {
            if (vkCreateSemaphore(m_logicalDevice.GetDevice(), &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(m_logicalDevice.GetDevice(), &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(m_logicalDevice.GetDevice(), &fenceInfo, nullptr, &m_inFlightFences[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create semaphores!");
            }
        }
    }

    void VulkanManager::CreateSceneViewGridObjects()
    {
        F_sceneViewGridObjects.SetIsSceneViewGridScene(true);

        F_sceneViewCameraObject = F_sceneViewGridObjects.CreateGameObject();
        F_sceneViewCameraObject->SetIsSceneViewGridObject(true);
        F_sceneViewCameraObject->SetName("SceneViewCamera");
        F_sceneViewCameraObject->AddCamera();

        GameObject* grid = F_sceneViewGridObjects.CreateGameObject();
        grid->SetIsSceneViewGridObject(true);
        grid->SetName("Grid");
        Mesh* gridMesh = grid->AddMesh();
        gridMesh->SetMaterial("fl_verticesOnly");
        gridMesh->SetModel("../engine/models/largeGrid.obj");
        gridMesh->AddTexture("../engine/images/colors/cave.png", 0);
        gridMesh->CreateResources();

        GameObject* xAxis = F_sceneViewGridObjects.CreateGameObject();
        xAxis->SetIsSceneViewGridObject(true);
        xAxis->SetName("xAxis");
        Mesh* xAxisMesh = xAxis->AddMesh();
        xAxisMesh->SetMaterial("fl_xAxis");
        xAxisMesh->SetModel("../engine/models/xAxis.obj");
        xAxisMesh->AddTexture("../engine/images/colors/green.png", 0);
        xAxisMesh->CreateResources();

        GameObject* yAxis = F_sceneViewGridObjects.CreateGameObject();
        yAxis->SetIsSceneViewGridObject(true);
        yAxis->SetName("yAxis");
        Mesh* yAxisMesh = yAxis->AddMesh();
        yAxisMesh->SetMaterial("fl_yAxis");
        yAxisMesh->SetModel("../engine/models/yAxis.obj");
        yAxisMesh->AddTexture("../engine/images/colors/yellow.png", 0);
        yAxisMesh->CreateResources();

        GameObject* zAxis = F_sceneViewGridObjects.CreateGameObject();
        zAxis->SetIsSceneViewGridObject(true);
        zAxis->SetName("zAxis");
        Mesh* zAxisMesh = zAxis->AddMesh();
        zAxisMesh->SetMaterial("fl_zAxis");
        zAxisMesh->SetModel("../engine/models/zAxis.obj");
        zAxisMesh->AddTexture("../engine/images/colors/rose.png", 0);    
        zAxisMesh->CreateResources();
    }

    std::vector<VkDescriptorSet>& VulkanManager::GetSceneViewDescriptorSets()
    {
        return m_sceneViewTexture.GetDescriptorSets();
    }

    bool VulkanManager::ShowSceneViewGridObjects()
    {
        return m_b_showGridObjects;
    }

    void VulkanManager::SetShowSceneViewGridObjects(bool b_showGridObjects)
    {
        m_b_showGridObjects = b_showGridObjects;
    }

    void VulkanManager::ToggleShowSceneViewGridObjects()
    {
        m_b_showGridObjects = !m_b_showGridObjects;
    }

    void VulkanManager::ToggleOrthographic()
    {
        m_b_orthographic = !m_b_orthographic;
    }

    std::vector<VkDescriptorSet>& VulkanManager::GetGameViewDescriptorSets()
    {
        return m_gameViewTexture.GetDescriptorSets();
    }

    void VulkanManager::DrawFrame()
    {
        // More info here - https://vulkan-tutorial.com/en/Drawing_a_triangle/Drawing/Rendering_and_presentation

        if (m_winSystem.m_b_framebufferResized)
        {
            RecreateSwapChainAndFrameBuffers();
            m_winSystem.m_b_framebufferResized = false;
        }

        // At the start of the frame, we want to wait until the previous frame has finished, so that the command buffer and semaphores are available to use. To do that, we call vkWaitForFences:
        vkWaitForFences(m_logicalDevice.GetDevice(), 1, &m_inFlightFences[VM_currentFrame], VK_TRUE, UINT64_MAX);

        DeleteQueuedVKObjects();

        uint32_t imageIndex;        
        VkResult aquireImageResult = vkAcquireNextImageKHR(m_logicalDevice.GetDevice(), m_winSystem.GetSwapChain(), UINT64_MAX, m_imageAvailableSemaphores[VM_currentFrame], VK_NULL_HANDLE, &imageIndex);
        if (!CheckSwapChainIntegrity(aquireImageResult, "Failed to acquire swap chain image."))
        {
            return;
        }

        // manually reset the fence to the unsignaled state with the vkResetFences call:
        vkResetFences(m_logicalDevice.GetDevice(), 1, &m_inFlightFences[VM_currentFrame]);
        


        std::list<VkCommandBuffer> commandBuffers;       
        std::vector<std::thread> threads = std::vector<std::thread>();

        threads.emplace_back([this, &commandBuffers, imageIndex]
        {
            // Scene View
            if (m_renderToTextureSceneViewRenderPass.Initialized())
            {                                  
                m_renderToTextureSceneViewRenderPass.BeginRenderPass(imageIndex);

                if (m_b_showGridObjects)
                {
                    for (std::map<long, Mesh>::iterator mesh = F_sceneViewGridObjects.GetMeshes().begin(); mesh != F_sceneViewGridObjects.GetMeshes().end(); mesh++)
                    {
                        std::shared_ptr<Material> material = mesh->second.GetSceneViewMaterial();
                        if (mesh->second.Initialized() && material != nullptr)
                        {                            
                            mesh->second.GetSceneViewModel().UpdateUniformBuffer(m_winSystem, &mesh->second, ViewportType::SceneView, m_b_orthographic);
                            m_renderToTextureSceneViewRenderPass.RecordCommandBuffer(material->GetGraphicsPipeline());
                            m_renderToTextureSceneViewRenderPass.DrawIndexed(mesh->second, material, ViewportType::SceneView); // Create final VkImage on m_sceneViewTexture's m_images member variable                                                       
                        }
                    }
                }          

                std::vector<Mesh*> meshesMissingTextures = std::vector<Mesh*>();

                for (std::map<std::string, std::map<long, Mesh*>>::iterator materials = m_sceneViewMaterialMeshes.begin(); materials != m_sceneViewMaterialMeshes.end(); materials++)
                {
                    if (m_sceneViewMaterials.count(materials->first))
                    {
                        std::shared_ptr<Material> material = m_sceneViewMaterials.at(materials->first);
                    
                        for (std::map<long, Mesh*>::iterator meshes = materials->second.begin(); meshes != materials->second.end(); meshes++)
                        {
                            Mesh* mesh = meshes->second;

                            m_renderToTextureSceneViewRenderPass.RecordCommandBuffer(material->GetGraphicsPipeline());

                            if (meshes->second->Initialized() && material != nullptr && !meshes->second->MissingTextures())
                            {
                                mesh->GetSceneViewModel().UpdateUniformBuffer(m_winSystem, meshes->second, ViewportType::SceneView, m_b_orthographic);
                                m_renderToTextureSceneViewRenderPass.DrawIndexed(*meshes->second, material, ViewportType::SceneView); // Create final VkImage on m_sceneViewTexture's m_images member variable                                       
                            }
                            else if (meshes->second->MissingTextures())
                            {
                                meshesMissingTextures.push_back(mesh);                        
                            }
                        }
                    }
                }

                // Render the Mesh but using the fl_empty material
                if (meshesMissingTextures.size())
                {
                    m_renderToTextureSceneViewRenderPass.RecordCommandBuffer(GetMaterial("fl_empty")->GetGraphicsPipeline());

                    for (Mesh* mesh : meshesMissingTextures)
                    {
                        mesh->GetSceneViewModel().UpdateUniformBuffer(m_winSystem, mesh, ViewportType::SceneView, m_b_orthographic);
                        m_renderToTextureSceneViewRenderPass.DrawIndexed(*mesh, GetMaterial("fl_empty"), ViewportType::SceneView); // Create final VkImage on m_sceneViewTexture's m_images member variable   
                    }
                }

                m_renderToTextureSceneViewRenderPass.EndRenderPass();

                commandBuffers.push_back(m_renderToTextureSceneViewRenderPass.GetCommandBuffers()[VM_currentFrame]);  
            }
        });


        threads.emplace_back([this, &commandBuffers, imageIndex]
        {
            // Game View
            if (m_renderToTextureGameViewRenderPass.Initialized())
            {            
                m_renderToTextureGameViewRenderPass.BeginRenderPass(imageIndex);

                std::vector<Mesh*> meshesMissingTextures = std::vector<Mesh*>();

                for (std::map<std::string, std::map<long, Mesh*>>::iterator materials = m_gameViewMaterialMeshes.begin(); materials != m_gameViewMaterialMeshes.end(); materials++)
                {
                    if (m_gameViewMaterials.count(materials->first))
                    {
                        std::shared_ptr<Material> material = m_gameViewMaterials.at(materials->first);

                        for (std::map<long, Mesh*>::iterator meshes = materials->second.begin(); meshes != materials->second.end(); meshes++)
                        {
                            Mesh* mesh = meshes->second;

                            m_renderToTextureGameViewRenderPass.RecordCommandBuffer(material->GetGraphicsPipeline());

                            if (mesh->Initialized() && material != nullptr && !mesh->MissingTextures())
                            {
                                mesh->GetGameViewModel().UpdateUniformBuffer(m_winSystem, mesh, ViewportType::GameView, m_b_orthographic);
                                m_renderToTextureGameViewRenderPass.DrawIndexed(*mesh, material, ViewportType::GameView); // Create final VkImage on m_sceneViewTexture's m_images member variable                                       
                            }
                            else if (mesh->MissingTextures())
                            {
                                meshesMissingTextures.push_back(mesh);
                            }
                        }
                    }
                }

                // Render the Mesh but using the fl_empty material (empty meshes
                if (meshesMissingTextures.size())
                {
                    m_renderToTextureGameViewRenderPass.RecordCommandBuffer(GetMaterial("fl_empty")->GetGraphicsPipeline());

                    for (Mesh* mesh : meshesMissingTextures)
                    {
                        mesh->GetGameViewModel().UpdateUniformBuffer(m_winSystem, mesh, ViewportType::GameView, m_b_orthographic);
                        m_renderToTextureGameViewRenderPass.DrawIndexed(*mesh, GetMaterial("fl_empty"), ViewportType::GameView); // Create final VkImage on m_gameViewTexture's m_images member variable   
                    }
                }

                m_renderToTextureGameViewRenderPass.EndRenderPass();

                commandBuffers.push_back(m_renderToTextureGameViewRenderPass.GetCommandBuffers()[VM_currentFrame]);
            }
        });
        
        for (auto& thread : threads)
        {
            if (thread.joinable())
            {
                thread.join();
            }
        }

        // Once VkImages have been written to in each viewport RenderPass, those VkImages can be used as textures and sampled by a different material and desired descriptorSets, (ie. ImGui!), so we need to create descriptor sets for them using the ImGui Material's configuration           
        // This process can't be done inside the threads because the same Allocator (VkDescriptorPool) is being used to allocate the VkDescriptorSets of each RenderToTexture, and accessing the same VkDescriptorPool from separate threads is prohibited, so we just wait for them to be joined back to the main thread.
        Model emptyModel = Model();
        std::map<uint32_t, Texture> sceneViewTextures = std::map<uint32_t, Texture>(); 
        sceneViewTextures.emplace(0, m_sceneViewTexture); // m_sceneViewTexture was given to m_renderToTextureSceneViewRenderPass in each material and it was written to in m_renderToTextureSceneViewRenderPass.DrawIndexed().. Likewise with Game View but with Game View specific member variables
        m_imGuiMaterial->GetAllocator().AllocateDescriptorSets(m_sceneViewTexture.GetDescriptorSets(), emptyModel, *m_imGuiMaterial->GetTexturesShaderStages(), sceneViewTextures);

        std::map<uint32_t, Texture> gameViewTextures = std::map<uint32_t, Texture>();
        gameViewTextures.emplace(0, m_gameViewTexture);
        m_imGuiMaterial->GetAllocator().AllocateDescriptorSets(m_gameViewTexture.GetDescriptorSets(), emptyModel, *m_imGuiMaterial->GetTexturesShaderStages(), gameViewTextures);

        if (m_imGuiMaterial != nullptr)
        {
            m_imGuiRenderPass.BeginRenderPass(imageIndex);
            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), m_imGuiRenderPass.GetCommandBuffers()[VM_currentFrame]);
            m_imGuiRenderPass.EndRenderPass();
        }
        commandBuffers.push_back(m_imGuiRenderPass.GetCommandBuffers()[VM_currentFrame]);
   
        std::vector<VkCommandBuffer> buffers = std::vector<VkCommandBuffer>();
        
        for (VkCommandBuffer& buffer : commandBuffers)
        {
            buffers.push_back(buffer);
        }

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { m_imageAvailableSemaphores[VM_currentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = (uint32_t)buffers.size();
        submitInfo.pCommandBuffers = buffers.data();

        VkSemaphore signalSemaphores[] = { m_renderFinishedSemaphores[VM_currentFrame] }; // { m_renderFinishedSemaphores[imageIndex] }; Seemed to fix the semaphore issue, but didn't fix the frame flickering
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        VkResult submitResult = vkQueueSubmit(m_logicalDevice.GetGraphicsQueue(), 1, &submitInfo, m_inFlightFences[VM_currentFrame]);
        CheckSwapChainIntegrity(submitResult, "Failed to submit draw command buffer.");

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = { m_winSystem.GetSwapChain() };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr;

        // Present!
        VkResult presentResult = vkQueuePresentKHR(m_logicalDevice.GetPresentQueue(), &presentInfo);

        VM_currentFrame = (VM_currentFrame + 1) % VM_MAX_FRAMES_IN_FLIGHT;
    }

    VkSampleCountFlagBits VulkanManager::GetMaxSamples()
    {
        return m_maxSamples;
    }

    void VulkanManager::SetMaxSamples(VkSampleCountFlagBits maxSamples)
    {
        m_maxSamples = maxSamples;
    }

    void VulkanManager::RecreateSwapChainAndFrameBuffers()
    {
        m_winSystem.RecreateSwapChain();   

        m_sceneViewTexture.CreateRenderToTextureResources(m_sceneViewCommandPool);
        m_gameViewTexture.CreateRenderToTextureResources(m_gameViewCommandPool);

        m_renderToTextureSceneViewRenderPass.ConfigureFrameBufferImageViews(m_sceneViewTexture.GetImageViews());        
        m_renderToTextureSceneViewRenderPass.RecreateFrameBuffers();

        m_renderToTextureGameViewRenderPass.ConfigureFrameBufferImageViews(m_gameViewTexture.GetImageViews());
        m_renderToTextureGameViewRenderPass.RecreateFrameBuffers();

        m_imGuiRenderPass.ConfigureFrameBufferImageViews(m_winSystem.GetSwapChainImageViews());
        m_imGuiRenderPass.RecreateFrameBuffers();

        ImGui_ImplVulkan_SetMinImageCount(static_cast<uint32_t>(m_winSystem.GetSwapChainImageViews().size()));
    }

    bool VulkanManager::CheckSwapChainIntegrity(VkResult result, std::string errorMessage)
    {
        // More details here - https://vulkan-tutorial.com/en/Drawing_a_triangle/Swap_chain_recreation
        // Check on swap chain integrity after image access and after present

        bool b_swapChainGood = true;

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
        {
            b_swapChainGood = false;
            RecreateSwapChainAndFrameBuffers();
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            RecreateSwapChainAndFrameBuffers();
            throw std::runtime_error(errorMessage);
        }

        return b_swapChainGood;
    }

    WinSys& VulkanManager::GetWinSystem()
    {
        return m_winSystem;
    }

    VkInstance& VulkanManager::GetInstance()
    {
        return m_instance;
    }

    VkQueue& VulkanManager::GetGraphicsQueue()
    {
        return m_logicalDevice.GetGraphicsQueue();
    }

    PhysicalDevice& VulkanManager::GetPhysicalDevice()
    {
        return m_physicalDevice;
    }

    LogicalDevice& VulkanManager::GetLogicalDevice()
    {
        return m_logicalDevice;
    }

    VkCommandPool& VulkanManager::GetCommandPool()
    {
        return m_systemCommandPool;
    }
}