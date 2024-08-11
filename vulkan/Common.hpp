#pragma once

#include <vulkan/vulkan.h>

#include <exception>

inline constexpr float STAGING_PRIORITY = 0.0f;
inline constexpr float LOW_PRIORITY = 0.25f;
inline constexpr float NORMAL_PRIORITY = 0.5f;
inline constexpr float HIGH_PRIORITY = 0.75f;
inline constexpr float RENDER_TARGET_PRIORITY = 1.0f;

class BadVkResult final : public std::exception {
public:
    explicit BadVkResult(VkResult result);

    const char *what() const noexcept override;
private:
    VkResult _result;
};

void check_success(VkResult result);
