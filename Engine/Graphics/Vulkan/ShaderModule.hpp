#pragma once

#include "Crunch/ClassUtility.hpp"
// #include "Graphics/SPIRVReflection.hpp"

#include "Graphics/Vulkan/Vulkan.hpp"

namespace Cr::Graphics::Vulkan
{

class ShaderModule : public NoCopy
{
    public:
        ShaderModule(VkDevice device, std::span<const U8> spirv, VkShaderStageFlags stage);
        ~ShaderModule();

        // [[nodiscard]] std::vector<VkDescriptorSetLayoutBinding> get_descriptor_set_layout_bindings() const;
        // [[nodiscard]] std::vector<VkPushConstantRange>          get_push_constant_ranges()  const;

        [[nodiscard]] constexpr VkShaderStageFlags    get_stage()  const { return m_stage;  }
        [[nodiscard]] constexpr const VkShaderModule& get_native() const { return m_handle; }

    private:
        // SPIRVReflection m_reflection;

        VkShaderModule     m_handle;
        VkShaderStageFlags m_stage;

        VkDevice m_device; 
};

}
