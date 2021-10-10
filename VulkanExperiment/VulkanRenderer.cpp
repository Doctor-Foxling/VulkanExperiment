#include "VulkanRenderer.h"

VulkanRenderer::VulkanRenderer()
{
#ifdef NDEBUG
    enableValidationLayers = false;
#else
    enableValidationLayers = true;
#endif
}

int VulkanRenderer::init(GLFWwindow* newWindow)
{
    m_Window = newWindow;

    try {
        createInstance();
        getPhysicalDevice();
    }
    catch (const std::runtime_error& e)
    {
        printf("Error: %s\n", e.what());
        return EXIT_FAILURE;
    }
    return 0;
}

void VulkanRenderer::cleanup()
{
    vkDestroyDevice(m_MainDevice.logicalDevice, nullptr);
    vkDestroyInstance(m_Instance, nullptr);
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

    if (enableValidationLayers)
    {
        instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME); // The macro used is equivalent to the string "VK_EXT_debug_utils"
    }

    // Check Instance Extensions supported...
    //(HINT: Other things also need extensions (e.g., devices), we are interested in instance extensions here)
    if (!checkInstanceExtensionSupport(&instanceExtensions))
    {
        throw std::runtime_error("VkInstance does not support required extensions!");
    }

    createInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
    createInfo.ppEnabledExtensionNames = instanceExtensions.data();

    // Setting up the validation layers that the instance would use
    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation",
    };

    if (enableValidationLayers && !checkValidationLayerSupport(&validationLayers))
    {
        throw std::runtime_error("VkInstance does not support required validation Layers!");
    }

    if (enableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else{
    createInfo.enabledLayerCount = 0;
    createInfo.ppEnabledLayerNames = nullptr;
    }

    VkResult result = vkCreateInstance(&createInfo, nullptr, &m_Instance);

    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create a Vulkan Instance");
    }
}

void VulkanRenderer::createLogicalDevice()
{
    //GEt the queue family indices for the chosen Physical Device
    QueueFamilyIndices indices = getQueueFamilies(m_MainDevice.physicalDevice);

    // Queue the logical devic needs to create and info to do so (Only 1 for now, will add more later!)
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = indices.graphicsFamily;          // The index of the family to create a queue from
    queueCreateInfo.queueCount = 1;                                     // Number of queues to create
    float priority = 1.0f;
    queueCreateInfo.pQueuePriorities = &priority;    // Vulkan needs to know how to handle multiple queues, so decide priority (1 = highest priority)

    // Information to create logical device (Sometimes called 'device')
    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = 1;                      // Number of Queue Create Infos
    deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;          // List of QueueCreateInfos so device can create reequired queues
    deviceCreateInfo.enabledExtensionCount = 0;                     //So this is different to the instance extensions, the device doesn't care aboutGLFW extensions
    deviceCreateInfo.ppEnabledExtensionNames = nullptr;             // List of enabled logical device extensions

    // Physical device features the logical device will be using
    VkPhysicalDeviceFeatures deviceFeatures = {};

    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

    //Create the logical device for the given physical device
    VkResult result = vkCreateDevice(m_MainDevice.physicalDevice, &deviceCreateInfo, nullptr, &m_MainDevice.logicalDevice);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create a Logical Device");
    }

    // Queues are created at the same time as the device
    // So we want a handle to our queues
    // From given logical device, of given Queue Family, of Given Queue Index (0, since only 1 queue), place reference in given VkQueue
    vkGetDeviceQueue(m_MainDevice.logicalDevice, indices.graphicsFamily, 0, &m_GraphicsQueue);
}

void VulkanRenderer::CreateDebugMessenger()
{

}

void VulkanRenderer::getPhysicalDevice()
{
    // Enumerate the Physical devices the vkInstance can access
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);

    // If no devices available, then none suport Vulkan!
    if (deviceCount == 0)
    {
        throw std::runtime_error("Can't find GPUs that support Vulkan Instance!");
    }

    // Get List of physical devices
    std::vector<VkPhysicalDevice> deviceList(deviceCount);
    vkEnumeratePhysicalDevices(m_Instance, &deviceCount, deviceList.data());

    for (const auto &device : deviceList)
    {
        if (checkDeviceSuitable(device))
        {
            m_MainDevice.physicalDevice = device;
            break;
        }
    }
}

bool VulkanRenderer::checkInstanceExtensionSupport(std::vector<const char*>* checkExtensions)
{
    // Need to get the number of extensions to create array of correct size to hold extensions
    uint32_t  extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    // Create a list of VkExtensionProperties using count
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

    // Check if given extensions are in list of available extensions
    for (const auto& checkExtension : *checkExtensions)
    {
        bool hasExtension = false;
        for (const auto& extension : extensions)
        {
            if (strcmp(checkExtension, extension.extensionName))
            {
                hasExtension = true;
                break;
            }
        }
        
        if (!hasExtension)
        {
            return true;
        }
    }

    return true;
}

bool VulkanRenderer::checkDeviceSuitable(VkPhysicalDevice device)
{
    /*
    // Inforation about the device itself (ID, name, type, vendor, etc.)
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);

    // Information about what the device can do (geo, shader, tess shader, wide lines, etc.)
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
    */

    QueueFamilyIndices indices = getQueueFamilies(device);

    return indices.isValid();
}

bool VulkanRenderer::checkValidationLayerSupport(const std::vector<const char*>* validationLayers)
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : *validationLayers) {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers)
        {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound)
        {
            return false;
        }
    }

    return true;
}

QueueFamilyIndices VulkanRenderer::getQueueFamilies(VkPhysicalDevice device)
{
    QueueFamilyIndices indices;

    //Get all Queue Family Property info for the given device
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilyList(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilyList.data());

    //Go through each queue family and check ifit has at least 1 of the required types of queue
    int i = 0;
    for (const auto& queueFamily : queueFamilyList)
    {
        // First check if queue family has at least 1 queue in that family (could have no queues)
        // Queue can be multiple types defined through bitfield. Need to bitwise AND with VK_QUEUE_*_BIT to check if has requried type
        if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphicsFamily = i;     // If queue family is valid then get index
        }

        if (indices.isValid())
        {
            break;
        }

        i++;
    }

    return indices;

}

// VKAPI_ATTR and VKAPI_CALL ensure that the function has the right signature for Vulkan to call it
VKAPI_ATTR VkBool32 VKAPI_CALL VulkanRenderer::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}
