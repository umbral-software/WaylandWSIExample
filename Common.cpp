#include "Common.hpp"

#include <vulkan/vk_enum_string_helper.h>

BadVkResult::BadVkResult(VkResult result)
    :_result(result)
{}

const char *BadVkResult::what() const noexcept {
    return string_VkResult(_result);
}

void check_success(VkResult result) {
    if (result) {
        throw BadVkResult(result);
    }
}
