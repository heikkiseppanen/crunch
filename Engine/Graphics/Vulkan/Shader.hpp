#pragma once

#include "Graphics/Vulkan/Vulkan.hpp"

#include "Crunch/ClassUtility.hpp"

namespace Cr::Graphics::Vulkan
{

class ShaderModule;

class Shader : public NoCopy
{
    public:
        Shader() = default;
        Shader(VkDevice device, VkFormat swap_format, VkPipelineBindPoint bindpoint, std::span<const Vulkan::ShaderModule* const> modules);
        ~Shader();

        [[nodiscard]] constexpr const VkPipeline&       get_native()     const { return m_handle;          }
        [[nodiscard]] constexpr const VkPipelineLayout& get_pipeline_layout()     const { return m_pipeline_layout; }
        [[nodiscard]] constexpr VkPipelineBindPoint     get_bind_point() const { return m_bindpoint;       }

        [[nodiscard]] constexpr const VkDescriptorSetLayout& get_descriptor_set_layout() const { return m_descriptor_set_layout; }
    private:
        VkPipeline          m_handle          {};
        VkPipelineLayout    m_pipeline_layout {};
        VkPipelineBindPoint m_bindpoint       {};

        VkDescriptorSetLayout m_descriptor_set_layout {};

        VkDevice m_device {};
};

}
