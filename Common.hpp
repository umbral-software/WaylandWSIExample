#pragma once

#include <vulkan/vulkan.h>

#include <exception>

#define STAGING_PRIORITY 0.0f
#define LOW_PRIORITY 0.25f
#define NORMAL_PRIORITY 0.5f
#define HIGH_PRIORITY 0.75f
#define RENDER_TARGET_PRIORITY 1.0f

class BadVkResult final : public std::exception {
public:
    explicit BadVkResult(VkResult result);

    const char *what() const noexcept override;
private:
    VkResult _result;
};

void check_success(VkResult result);
