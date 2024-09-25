#pragma once

#include "Graphics/Vulkan/Vulkan.hpp"
#include "Graphics/Vulkan/CommandBuffer.hpp"

#include "Crunch/ClassUtility.hpp"

namespace Cr::Graphics::Vulkan
{

class Queue : public NoCopy
{
    public:
        Queue() = default;
        Queue(VkDevice device, U32 family_index, U32 queue_index);
        ~Queue();

        Queue(Queue&& other) noexcept;
        Queue& operator = (Queue&& other) noexcept;

        [[nodiscard]] Unique<Vulkan::CommandBuffer> create_command_buffer();

        void submit(std::span<const VkSubmitInfo> submissions, VkFence to_signal);

        void wait_for_idle();

        [[nodiscard]] constexpr U32 get_family_index() const { return m_family_index; }
        [[nodiscard]] constexpr VkQueue get_native() const { return m_handle; }

    private:
        VkQueue       m_handle {};
        VkCommandPool m_command_pool {};

        U32 m_family_index;

        VkDevice m_device {};
};

}
