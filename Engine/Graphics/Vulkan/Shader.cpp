#include "Graphics/Vulkan/Shader.hpp"
#include "Graphics/Vulkan/ShaderModule.hpp"

namespace Cr::Graphics::Vulkan
{

Shader::Shader(VkDevice device, VkFormat swap_format, VkPipelineBindPoint bindpoint, std::span<const Vulkan::ShaderModule* const> modules)
    : m_bindpoint(bindpoint)
    , m_device(device)
{
    VkResult result {};

    std::vector<VkPipelineShaderStageCreateInfo> stage_infos;

    VkShaderStageFlags stage_flags = {};
    for (const auto* module : modules)
    {
        CR_ASSERT((module->get_stage() & stage_flags) == 0, "Multiple Vulkan shader modules with same stages provided");
        stage_flags |= module->get_stage();

        stage_infos.push_back({
            .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage  = static_cast<VkShaderStageFlagBits>(module->get_stage()),
            .module = module->get_native(),
            .pName  = "main"
        });
    }

    // TODO Define from a provided Vertex layout but hardcode for now
    const VkVertexInputBindingDescription bind_descriptor{
        .binding   = 0,
        .stride    = sizeof(float) * 8,
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
    };

    const std::array attribute_descriptors {
        VkVertexInputAttributeDescription {
            .location = 0,
            .binding  = 0,
            .format   = VK_FORMAT_R32G32B32_SFLOAT,
            .offset   = 0,
        },
        VkVertexInputAttributeDescription {
            .location = 1,
            .binding  = 0,
            .format   = VK_FORMAT_R32G32_SFLOAT,
            .offset   = sizeof(float) * 3,
        },
        VkVertexInputAttributeDescription {
            .location = 2,
            .binding  = 0,
            .format   = VK_FORMAT_R32G32B32_SFLOAT,
            .offset   = sizeof(float) * 5,
        }
    };

    const VkPipelineVertexInputStateCreateInfo vertex_input_state_info {
        .sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount   = 1,
        .pVertexBindingDescriptions      = &bind_descriptor,
        .vertexAttributeDescriptionCount = attribute_descriptors.size(),
        .pVertexAttributeDescriptions    = attribute_descriptors.data(),
    };

    const VkPipelineInputAssemblyStateCreateInfo input_assembly_info{
        .sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE,
    };

    // Pipeline viewport, will be set dynamically
    const VkViewport viewport{};
    const VkRect2D   scissor {};
    const VkPipelineViewportStateCreateInfo viewport_info {
        .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .pViewports    = &viewport,
        .scissorCount  = 1,
        .pScissors     = &scissor,
    };

    const VkPipelineRasterizationStateCreateInfo rasterization_info {
        .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .pNext                   = nullptr,
        .flags                   = {},
        .depthClampEnable        = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode             = VK_POLYGON_MODE_FILL,
        .cullMode                = VK_CULL_MODE_BACK_BIT,
        .frontFace               = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable         = VK_FALSE,
        .depthBiasConstantFactor = 0.0f,
        .depthBiasClamp          = 0.0f,
        .depthBiasSlopeFactor    = 0.0f,
        .lineWidth               = 1.0f,
    };

    const VkPipelineMultisampleStateCreateInfo multisampling_info {
        .sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable   = VK_TRUE,
        .minSampleShading      = 1.0f,
        .pSampleMask           = nullptr,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable      = VK_FALSE,
    };

    // NOT USED YET
    //VkPipelineDepthStencilStateCreateInfo depth_stencil_info{};

    const VkPipelineColorBlendAttachmentState color_blend_attachment {
        .blendEnable         = VK_FALSE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
        .colorBlendOp        = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp        = VK_BLEND_OP_ADD,
        .colorWriteMask      = VK_COLOR_COMPONENT_R_BIT |
                               VK_COLOR_COMPONENT_G_BIT |
                               VK_COLOR_COMPONENT_B_BIT |
                               VK_COLOR_COMPONENT_A_BIT,
    };

    const VkPipelineColorBlendStateCreateInfo color_blend_info {
        .sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable   = VK_FALSE,
        .logicOp         = VK_LOGIC_OP_COPY,
        .attachmentCount = 1,
        .pAttachments    = &color_blend_attachment,
        //.blendConstants = {},
    };

    const std::array dynamic_states {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
    };

    const VkPipelineDynamicStateCreateInfo dynamic_state_info {
        .sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = static_cast<uint32_t>(dynamic_states.size()),
        .pDynamicStates    = dynamic_states.data(),
    };
    
    // TODO Reflect, hardcode for now
    const std::array descriptor_set_layout_bindings {
        VkDescriptorSetLayoutBinding{
            .binding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
            .pImmutableSamplers = nullptr,
        },
        VkDescriptorSetLayoutBinding{
            .binding = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = nullptr,
        },
    };

    const VkDescriptorSetLayoutCreateInfo descriptor_set_layout_info {
        .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = descriptor_set_layout_bindings.size(),
        .pBindings    = descriptor_set_layout_bindings.data(),
    };

    result = vkCreateDescriptorSetLayout(m_device, &descriptor_set_layout_info, nullptr, &m_descriptor_set_layout);
    VK_ASSERT_THROW(result, "Failed to create Vulkan Descriptor Set Layout: {}", to_string(result));

    // TODO Reflect, hardcode for now
    const VkPushConstantRange push_constant_range {
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .offset     = 0,
        .size       = sizeof(PushConstantObject),
    };

    const VkPipelineLayoutCreateInfo layout_info {
        .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount         = 1,
        .pSetLayouts            = &m_descriptor_set_layout,
        .pushConstantRangeCount = 1,
        .pPushConstantRanges    = &push_constant_range,
    };

    result = vkCreatePipelineLayout(m_device, &layout_info, nullptr, &m_pipeline_layout);
    VK_ASSERT_THROW(result, "Failed to create Vulkan Pipeline Layout: {}", to_string(result));

    const VkPipelineRenderingCreateInfoKHR pipeline_rendering_info {
        .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
        .colorAttachmentCount    = 1,
        .pColorAttachmentFormats = &swap_format,
    };

    const VkGraphicsPipelineCreateInfo pipeline_info {
        .sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext               = &pipeline_rendering_info,
        .stageCount          = static_cast<uint32_t>(stage_infos.size()),
        .pStages             = stage_infos.data(),
        .pVertexInputState   = &vertex_input_state_info,
        .pInputAssemblyState = &input_assembly_info,
//        .pTessellationState  =
        .pViewportState      = &viewport_info,
        .pRasterizationState = &rasterization_info,
        .pMultisampleState   = &multisampling_info,
//        .pDepthStencilState  =
        .pColorBlendState    = &color_blend_info,
        .pDynamicState       = &dynamic_state_info,
        .layout              = m_pipeline_layout,
//        .renderPass          =
//        .subpass             =
//        .basePipelineHandle  =
        .basePipelineIndex   = -1,
    };

    result = vkCreateGraphicsPipelines(m_device, nullptr, 1, &pipeline_info, nullptr, &m_handle);
    VK_ASSERT_THROW(result, "Failed to create Vulkan Pipeline: {}", to_string(result));
}

Shader::~Shader()
{
    if (m_handle)
    {
        vkDestroyPipeline(m_device, m_handle, nullptr);
        vkDestroyPipelineLayout(m_device, m_pipeline_layout, nullptr);
        vkDestroyDescriptorSetLayout(m_device, m_descriptor_set_layout, nullptr);
        m_handle = {};
        m_pipeline_layout = {};
        m_descriptor_set_layout = {};
        m_device = {};
    }
}

}
