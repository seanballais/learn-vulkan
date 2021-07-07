#ifndef SWAP_CHAIN_SUPPORT_DETAILS_HPP
#define SWAP_CHAIN_SUPPORT_DETAILS_HPP

#include <vector>

#include <vulkan/vulkan.h>

struct SwapChainSupportDetails
{
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> presentModes;
};

#endif