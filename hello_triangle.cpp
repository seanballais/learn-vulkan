#include <algorithm>
#include <bits/stdint-uintn.h>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <optional>
#include <set>
#include <stdexcept>
#include <vector>
#include <vulkan/vk_platform.h>
#include <vulkan/vulkan_core.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

constexpr uint32_t WINDOW_HEIGHT = 800;
constexpr uint32_t WINDOW_WIDTH = 600;

const std::vector<const char*> g_validationLayers {
  "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> g_deviceExtensions {
  VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#ifdef NDEBUG
const bool g_areValidationLayersEnabled = false;
#else
const bool g_areValidationLayersEnabled = true;
#endif

VkResult CreateDebugUtilsMessengerEXT(
  VkInstance instance,
  const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
  const VkAllocationCallbacks* pAllocator,
  VkDebugUtilsMessengerEXT* pDebugMessenger)
{
  auto func = (PFN_vkCreateDebugUtilsMessengerEXT)
    vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
  if (func != nullptr) {
    return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
  } else {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}

void DestroyDebugUtilsMessengerEXT(
  VkInstance instance,
  VkDebugUtilsMessengerEXT debugMessenger,
  const VkAllocationCallbacks* pAllocator)
{
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)
    vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
  if (func != nullptr) {
    func(instance, debugMessenger, pAllocator);
  }
}

struct QueueFamilyIndices
{
  std::optional<uint32_t> m_graphicsFamily;
  std::optional<uint32_t> m_presentFamily;

  bool isComplete()
  {
    return m_graphicsFamily.has_value()
      && m_presentFamily.has_value();
  }
};

struct SwapChainSupportDetails
{
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> presentModes;
};

class HelloTriangle
{
public:
  void run() {
#ifndef NDEBUG
    std::cout << "Running debug build.\n";
#endif

    initWindow();
    initVulkan();
    mainLoop();
    performCleanup();
  }

private:
  void initWindow()
  {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    m_window = glfwCreateWindow(WINDOW_HEIGHT, WINDOW_WIDTH, "Vulkan!",
                                nullptr, nullptr);
  }

  void initVulkan()
  {
    createVkInstance();
    setupDebugMessenger();
    createSurface();
    selectPhysicalDevice();
    createLogicalDevice();
    createSwapChain();
  }

  void mainLoop()
  {
    while (!glfwWindowShouldClose(m_window)) {
      glfwPollEvents();
    }
  }

  void performCleanup()
  {
    for (auto imageView : m_swapChainImageViews) {
      vkDestroyImageView(m_device, imageView, nullptr);
    }

    vkDestroySwapchainKHR(m_device, m_swapChain, nullptr);

    vkDestroyDevice(m_device, nullptr);

    vkDestroySurfaceKHR(m_vkInstance, m_surface, nullptr);

    if (g_areValidationLayersEnabled) {
      DestroyDebugUtilsMessengerEXT(m_vkInstance, m_debugMessenger, nullptr);
    }

    vkDestroyInstance(m_vkInstance, nullptr);

    glfwDestroyWindow(m_window);

    glfwTerminate();
  }

  static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
  {
    std::cerr << "Validation Layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
  }

  void populateDebugMessengerCreateInfo(
    VkDebugUtilsMessengerCreateInfoEXT& createInfo)
  {
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity =
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
      | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
      | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType =
      VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
      | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
      | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
  }

  void createVkInstance()
  {
    if (g_areValidationLayersEnabled && !checkValidationLayerSupport()) {
      throw std::runtime_error("Validation layers requested, but unavailable!");
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0 , 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    uint32_t vkExtensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &vkExtensionCount, nullptr);

    std::vector<VkExtensionProperties> vkExtensions(vkExtensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr,
                                           &vkExtensionCount,
                                           vkExtensions.data());

    std::cout << "Available Vulkan Extensions: \n";

    for (const auto& extension : vkExtensions) {
      std::cout << "\t" << extension.extensionName << "\n";
    }

    auto extensions = getRequiredExtensions();    
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    if (g_areValidationLayersEnabled) {
      createInfo.enabledLayerCount = static_cast<uint32_t>(
        g_validationLayers.size());
      createInfo.ppEnabledLayerNames = g_validationLayers.data();

      populateDebugMessengerCreateInfo(debugCreateInfo);
      createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
    } else {
      createInfo.enabledLayerCount = 0;
    }

    if (vkCreateInstance(&createInfo, nullptr, &m_vkInstance) != VK_SUCCESS) {
      throw std::runtime_error("Failed to create instance!");
    }
  }
  
  bool checkValidationLayerSupport()
  {
    uint32_t numLayers;
    vkEnumerateInstanceLayerProperties(&numLayers, nullptr);

    std::vector<VkLayerProperties> availableLayers(numLayers);
    vkEnumerateInstanceLayerProperties(&numLayers, availableLayers.data());

    for (const char* layerName : g_validationLayers) {
      bool isLayerFound = false;

      for (const auto& layerProperties : availableLayers) {
        if (strcmp(layerName, layerProperties.layerName) == 0) {
          isLayerFound = true;
          break;
        }
      }

      if (!isLayerFound) {
        return false;
      }
    }

    return true;
  }

  std::vector<const char*> getRequiredExtensions()
  {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions,
                                        glfwExtensions + glfwExtensionCount);

    if (g_areValidationLayersEnabled) {
      extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
  }

  void setupDebugMessenger()
  {
    if (!g_areValidationLayersEnabled) {
      return;
    }

    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    populateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(m_vkInstance,
                                     &createInfo,
                                     nullptr,
                                     &m_debugMessenger) != VK_SUCCESS) {
      throw std::runtime_error("Failed to setup debug messenger.");
    }
  }

  void createSurface()
  {
    if (glfwCreateWindowSurface(m_vkInstance, m_window, nullptr, &m_surface)
        != VK_SUCCESS) {
      throw std::runtime_error("Failed to create window surface!");
    }
  }

  QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device)
  {
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
                                             nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
                                             queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
      if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
        indices.m_graphicsFamily = i;
      }

      VkBool32 isPresentSupportAvailable = false;
      vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface,
                                           &isPresentSupportAvailable);

      if (isPresentSupportAvailable) {
        indices.m_presentFamily = i;
      }

      i++;

      if (indices.isComplete()) {
        break;
      }
    }

    return indices;
  }

  SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device)
  {
    SwapChainSupportDetails details;
    
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface,
                                              &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount,
                                         nullptr);
    if (formatCount != 0) {
      details.formats.resize(formatCount);
      vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount,
                                           details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface,
                                              &presentModeCount,
                                              nullptr);
    if (presentModeCount != 0) {
      details.presentModes.resize(presentModeCount);
      vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface,
                                                &presentModeCount,
                                                details.presentModes.data());
    }

    return details;
  }

  bool isPhysicalDeviceSuitable(VkPhysicalDevice device)
  {
    QueueFamilyIndices indices = findQueueFamilies(device);

    bool areExtensionsSupported = checkDeviceExtensionSupport(device);

    bool isSwapChainAdequate = false;
    if (areExtensionsSupported) {
      SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
      isSwapChainAdequate = !swapChainSupport.formats.empty()
                            && !swapChainSupport.presentModes.empty();
    }

    return indices.isComplete()
           && areExtensionsSupported
           && isSwapChainAdequate;
  }

  VkSurfaceFormatKHR chooseSwapSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR>& availableFormats)
  {
    for (const auto& availableFormat : availableFormats) {
      if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB
          && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
        return availableFormat;
      }
    }

    return availableFormats[0];
  }

  VkPresentModeKHR chooseSwapPresentMode(
    const std::vector<VkPresentModeKHR>& availablePresentModes)
  {
    for (const auto& availablePresentMode : availablePresentModes) {
      if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
        return availablePresentMode;
      }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
  }

  VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
  {
    if (capabilities.currentExtent.width != UINT32_MAX) {
      return capabilities.currentExtent;
    } else {
      VkExtent2D actualExtent = { WINDOW_WIDTH, WINDOW_HEIGHT };
      actualExtent.width = std::max(
        capabilities.minImageExtent.width,
        std::min(capabilities.maxImageExtent.width, actualExtent.width));
      actualExtent.height = std::max(
        capabilities.minImageExtent.height,
        std::min(capabilities.maxImageExtent.height, actualExtent.height));

      return actualExtent;
    }
  }  

  void selectPhysicalDevice()
  {
    uint32_t numDevices = 0;
    vkEnumeratePhysicalDevices(m_vkInstance, &numDevices, nullptr);

    if (numDevices == 0) {
      throw std::runtime_error("Failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(numDevices);
    vkEnumeratePhysicalDevices(m_vkInstance, &numDevices, devices.data());

    for (const auto& device : devices) {
      if (isPhysicalDeviceSuitable(device)) {
        m_physicalDevice = device;
        break;
      }
    }

    if (m_physicalDevice == VK_NULL_HANDLE) {
      throw std::runtime_error("Failed to find a suitable GPU!");
    }
  }

  void createLogicalDevice()
  {
    QueueFamilyIndices indices = findQueueFamilies(m_physicalDevice);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {
      indices.m_graphicsFamily.value(),
      indices.m_presentFamily.value()
    };

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
      VkDeviceQueueCreateInfo queueCreateInfo{};
      queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      queueCreateInfo.queueFamilyIndex = indices.m_graphicsFamily.value();
      queueCreateInfo.queueCount = 1;
      queueCreateInfo.pQueuePriorities = &queuePriority;

      queueCreateInfos.push_back(queueCreateInfo);
    }    

    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = 
      static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(
      g_deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = g_deviceExtensions.data();

    if (g_areValidationLayersEnabled) {
      createInfo.enabledLayerCount = static_cast<uint32_t>(
        g_validationLayers.size());
      createInfo.ppEnabledLayerNames = g_validationLayers.data();
    } else {
      createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device)
        != VK_SUCCESS) {
      throw std::runtime_error("Failed to create logical device!");
    }

    vkGetDeviceQueue(m_device, indices.m_graphicsFamily.value(), 0,
                     &m_graphicsQueue);
    vkGetDeviceQueue(m_device, indices.m_presentFamily.value(), 0,
                     &m_presentQueue);
  }

  void createSwapChain()
  {
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(
      m_physicalDevice);
    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(
      swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(
      swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imgCount = swapChainSupport.capabilities.minImageCount + 1;

    if (swapChainSupport.capabilities.maxImageCount > 0
        && imgCount > swapChainSupport.capabilities.maxImageCount) {
      imgCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_surface;
    createInfo.minImageCount = imgCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = findQueueFamilies(m_physicalDevice);
    uint32_t queueFamilyIndices[] = {
      indices.m_graphicsFamily.value(),
      indices.m_presentFamily.value()
    };
    if (indices.m_graphicsFamily != indices.m_presentFamily) {
      createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
      createInfo.queueFamilyIndexCount = 2;
      createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
      createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
      createInfo.queueFamilyIndexCount = 0;
      createInfo.pQueueFamilyIndices = nullptr;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &m_swapChain)
        != VK_SUCCESS) {
      throw std::runtime_error("Failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(m_device, m_swapChain, &imgCount, nullptr);
    m_swapChainImages.resize(imgCount);
    vkGetSwapchainImagesKHR(m_device, m_swapChain, &imgCount,
                            m_swapChainImages.data());

    m_swapChainImageFormat = surfaceFormat.format;
    m_swapChainExtent = extent;
  }

  void createImageViews()
  {
    m_swapChainImageViews.resize(m_swapChainImages.size());

    for (size_t i = 0; i < m_swapChainImages.size(); i++) {
      VkImageViewCreateInfo createInfo{};
      createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
      createInfo.image = m_swapChainImages[i];
      createInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
      createInfo.format = m_swapChainImageFormat;
      createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      createInfo.subresourceRange.baseMipLevel = 0;
      createInfo.subresourceRange.levelCount = 1;
      createInfo.subresourceRange.baseArrayLayer = 0;
      createInfo.subresourceRange.layerCount = 1;

      if (vkCreateImageView(m_device, &createInfo, nullptr,
                            &m_swapChainImageViews[i])
          != VK_SUCCESS) {
        throw std::runtime_error("Failed to create image views.");
      }
    }
  }

  bool checkDeviceExtensionSupport(VkPhysicalDevice device)
  {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
                                         nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
                                         availableExtensions.data());

    std::set<std::string> requiredExtensions(g_deviceExtensions.begin(),
                                             g_deviceExtensions.end());
    for (const auto& extension : availableExtensions) {
      requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
  }

  GLFWwindow* m_window;
  VkInstance m_vkInstance;
  VkDebugUtilsMessengerEXT m_debugMessenger;
  VkSurfaceKHR m_surface;
  VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
  VkDevice m_device;
  VkQueue m_graphicsQueue;
  VkQueue m_presentQueue;
  VkSwapchainKHR m_swapChain;
  std::vector<VkImage> m_swapChainImages;
  VkFormat m_swapChainImageFormat;
  VkExtent2D m_swapChainExtent;
  std::vector<VkImageView> m_swapChainImageViews;
};

int main()
{
  HelloTriangle app;

  try {
    app.run();
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;

    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
