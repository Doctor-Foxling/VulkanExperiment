#include "VulkanRenderer.h"

VulkanRenderer::VulkanRenderer()
{
}

int VulkanRenderer::init(GLFWwindow* newWindow)
{
    m_Window = newWindow;

    try {
        createInstance();
    }
    catch (const std::runtime_error& e)
    {
        printf("Error: %s\n", e.what());
        return EXIT_FAILURE;
    }
    return 0;
}

VulkanRenderer::~VulkanRenderer()
{
}

void VulkanRenderer::createInstance()
{
    // Information about the application itesef
    // Most data here doesn't effect the program and is for developer convinience
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;      
    appInfo.pApplicationName = "Vulkan App";                    //Custom name of the application
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);     // Custom version of the application
    appInfo.pEngineName = "No Engine";                         // Custom engine name
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);          // Custom engine version

    // This actually does affect the program
    appInfo.apiVersion = VK_API_VERSION_1_2;                   // The Vulkan Version

    // Creation information for a Vulkan Instance
    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    // Create list to hold instance extensions
    std::vector<const char*> instanceExtensions = std::vector<const char*>();

    // Set up extensions Instance will use
    uint32_t glfwExtensionCount = 0;         // GLFW may require multiple extensions
    const char** glfwExtensions;            // Extensions passed as array of cstrings, so need pointer (the array) to pointer (the cstring)

    // Get GLFW Extensions
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    // Add GLFW extensions to list of extensions
    for (size_t i = 0; i < glfwExtensionCount; i++)
    {
        instanceExtensions.push_back(glfwExtensions[i]);
    }

    createInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
    createInfo.ppEnabledExtensionNames = instanceExtensions.data();

    // TODO: Set up VAlidation Layers that Instance will use
    createInfo.enabledLayerCount = 0;
    createInfo.ppEnabledLayerNames = nullptr;

    VkResult result = vkCreateInstance(&createInfo, nullptr, &m_Instance);

    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create a Vulkan Instance");
    }
}
