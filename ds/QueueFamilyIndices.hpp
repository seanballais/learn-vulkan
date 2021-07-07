#ifndef QUEUE_FAMILY_INDICES_HPP
#define QUEUE_FAMILY_INDICES_HPP

#include <cstdint>
#include <optional>

struct QueueFamilyIndices
{
  std::optional<uint32_t> m_graphicsFamily;
  std::optional<uint32_t> m_presentFamily;

  bool isComplete()
  {
    return m_graphicsFamily.has_value() && m_presentFamily.has_value();
  }
};

#endif