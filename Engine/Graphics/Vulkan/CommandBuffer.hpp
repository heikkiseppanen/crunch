#pragma once

#include "Crunch.hpp"

#include "Shared/ClassUtility.hpp"

#include "Graphics/Vulkan/Structs.hpp"
#include "Graphics/Vulkan/Extension.hpp"

namespace Cr::Vk
{

class CommandBuffer : public NoValueSemantics
{
    public:
        CommandBuffer() = delete;
        CommandBuffer(VkDevice device, VkCommandPool pool);
        ~CommandBuffer();

        void draw_indexed();

        void begin_recording(VkCommandBufferUsageFlags flags);
        void end_recording();

        void set_image_memory_barrier(Image& image, VkImageLayout old_layout, VkImageLayout new_layout, VkImageSubresourceRange sub_resource_layout);

    private:
        VkCommandBuffer m_handle = VK_NULL_HANDLE;
};

}
