#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <iostream>
#include <vector>

#include "Utilities.h"

class VulkanRenderer
{
public:
	VulkanRenderer();

	int init(GLFWwindow* newWindow);
	void cleanup();

	~VulkanRenderer();
private:
	GLFWwindow* m_Window;

	// Vulkan Comnponents
	VkInstance m_Instance;
	struct {
		VkPhysicalDevice physicalDevice;
		VkDevice logicalDevice;
	} m_MainDevice;

	VkQueue m_GraphicsQueue;

	bool enableValidationLayers;

	VkDebugUtilsMessengerEXT m_DebugMessenger;

	// |--- Vulkan Functions ---|

	// Create Functions
	void createInstance();
	void createLogicalDevice();
	void CreateDebugMessenger();

	// Get Functions
	void getPhysicalDevice();

	// |- Support Functions -|
	// Checker Functions
	bool checkInstanceExtensionSupport(std::vector<const char*>* checkExtensions);
	bool checkDeviceSuitable(VkPhysicalDevice device);
	bool checkValidationLayerSupport(const std::vector<const char*>* validationLayers);

	// Getter Functions
	QueueFamilyIndices getQueueFamilies(VkPhysicalDevice device);

	// Callback Functions
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackdata,
		void* pUserData);

	// Others
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	
	// |-	 ^	  -|

	// |---		 ^		 ---|
};

