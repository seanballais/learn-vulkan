#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

constexpr uint32_t WINDOW_HEIGHT = 800;
constexpr uint32_t WINDOW_WIDTH = 600;

class HelloTriangle
{
public:
  void run() {
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
  }

  void mainLoop()
  {
    while (!glfwWindowShouldClose(m_window)) {
      glfwPollEvents();
    }
  }

  void performCleanup()
  {
    vkDestroyInstance(m_vkInstance, nullptr);

    glfwDestroyWindow(m_window);

    glfwTerminate();
  }

  void createVkInstance()
  {
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

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;

    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

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
    
    createInfo.enabledExtensionCount = glfwExtensionCount;
    createInfo.ppEnabledExtensionNames = glfwExtensions;
    createInfo.enabledLayerCount = 0;

    if (vkCreateInstance(&createInfo, nullptr, &m_vkInstance) != VK_SUCCESS) {
      throw std::runtime_error("Failed to create instance!");
    }
  }

  GLFWwindow* m_window;
  VkInstance m_vkInstance;
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
