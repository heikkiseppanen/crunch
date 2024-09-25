#include "Graphics/Vulkan/ShaderModule.hpp"

namespace Cr::Graphics::Vulkan
{

ShaderModule::ShaderModule(VkDevice device, std::span<const U8> spirv, VkShaderStageFlags stage)
    //: m_reflection(spirv)
    : m_handle()
    , m_stage(stage)
    , m_device(device)
{
    VkShaderModuleCreateInfo info {
        .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = spirv.size(),
        .pCode    = reinterpret_cast<const U32*>(spirv.data()),
    };
        
    auto result = vkCreateShaderModule(m_device, &info, nullptr, &m_handle);
    VK_ASSERT_THROW(result, "Failed to create Vulkan shader module: {}", to_string(result));
}

ShaderModule::~ShaderModule()
{
    if (m_handle)
    {
        vkDestroyShaderModule(m_device, m_handle, nullptr);
        m_handle = {};
        m_device = {};
    }
}

// [[nodiscard]] std::vector<VkDescriptorSetLayoutBinding> ShaderModule::get_descriptor_set_layout_bindings() const
// {
//     std::vector<VkDescriptorSetLayoutBinding> vulkan_bindings;
//     for (const auto* descriptor_set : m_reflection.get_descriptor_sets())
//     {
//         for (const auto it = descriptor_set->bindings, end = it + descriptor_set->binding_count; it != end; ++it)
//         {
//             const auto* binding = *it;
//             vulkan_bindings.push_back({
//                 .binding         = layout->binding,
//                 .descriptorCount = layout->
//                 .descriptorType  = layout->count.
//                 .stageFlags      = m_stage,
//             });
//         }
//     }
// }

//[[nodiscard]] std::vector<VkPushConstantRange> ShaderModule::get_push_constant_ranges() const
//{

} // Cr::Graphics::Vulkan
