#include <bits/stdint-uintn.h>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <optional>
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
  std::optional<uint32_t> graphicsFamily;
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
    selectPhysicalDevice();
  }

  void mainLoop()
  {
    while (!glfwWindowShouldClose(m_window)) {
      glfwPollEvents();
    }
  }

  void performCleanup()
  {
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

    VkApplicationInfo appInfo;
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0 , 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo;
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

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(m_vkInstance,
                                     &createInfo,
                                     nullptr,
                                     &m_debugMessenger) != VK_SUCCESS) {
      throw std::runtime_error("Failed to setup debug messenger.");
    }
  }

  bool isPhysicalDeviceSuitable(VkPhysicalDevice device)
  {
    return true;
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

  QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device)
  {
    QueueFamilyIndices indices;

    return indices;
  }

  GLFWwindow* m_window;
  VkInstance m_vkInstance;
  VkDebugUtilsMessengerEXT m_debugMessenger;
  VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
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
