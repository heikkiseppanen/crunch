#include "Graphics/API.hpp"

#include "ktx.h"

namespace Cr::Graphics
{

API::API(const Core::Window& surface_context) : driver(surface_context) {}

MeshID API::mesh_create(const std::vector<Vertex>& vertices, const std::vector<u32>& indices)
{
    // TODO Shouldn't create buffers per mesh, but lets do for now!

    const u64 vertex_buffer_size = sizeof(vertices[0]) * vertices.size();
    const u64 index_buffer_size  = sizeof(indices[0]) * indices.size();

    const MeshContext mesh {
        .vertex_buffer_id = this->driver.buffer_create(Graphics::BufferType::VERTEX, vertex_buffer_size),
        .index_buffer_id  = this->driver.buffer_create(Graphics::BufferType::INDEX, index_buffer_size),
        .index_count      = static_cast<u32>(indices.size()),
    };

    this->driver.buffer_map_range(mesh.vertex_buffer_id, vertices.data(), vertices.size(), 0);
    this->driver.buffer_map_range(mesh.index_buffer_id, indices.data(), indices.size(), 0);

    this->driver.buffer_flush(mesh.vertex_buffer_id);
    this->driver.buffer_flush(mesh.index_buffer_id);

    this->meshes.emplace_back(mesh);

    return this->meshes.size() - 1;
}

void API::mesh_destroy(MeshID mesh_id)
{
    // NOTE Should rather just free up the context from within the buffer... am I going to end up writing my own vk buffer allocator at some point?
    auto& mesh = this->meshes[mesh_id];

    this->driver.buffer_destroy(mesh.vertex_buffer_id);
    this->driver.buffer_destroy(mesh.index_buffer_id);

    // TODO Meshes don't have any sort of an invalidated state to work with, should be resolved by the ownership pool implementation honestly
}

u32 API::texture_create(const std::string& path)
{
    TextureContext texture = {};

    // TODO Move ktx import elsewhere
    ktxTexture2* ktx_handle;

    CR_ASSERT_THROW(ktxTexture2_CreateFromNamedFile(path.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktx_handle) == KTX_SUCCESS, "Failed to load ktx image");
    const Defer ktx_cleanup = [ktx_handle]{ ktxTexture_Destroy(ktxTexture(ktx_handle)); };

    const ktx_uint8_t* image_data = ktxTexture_GetData(ktxTexture(ktx_handle));
    const ktx_size_t   image_size = ktxTexture_GetDataSize(ktxTexture(ktx_handle));

    // TODO Have staging buffers cached
    const BufferID staging_buffer_id      = this->driver.buffer_create(BufferType::STAGING, image_size);
    const Defer    staging_buffer_cleanup = [&] { this->driver.buffer_destroy(staging_buffer_id); };

    this->driver.buffer_map_range(staging_buffer_id, image_data, image_size, 0);
    this->driver.buffer_flush(staging_buffer_id);

    // NOTE This could work well as a builder pattern?
    VkImageCreateInfo image_info{};
    image_info.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.pNext         = nullptr;
    image_info.flags         = 0;
    image_info.imageType     = VK_IMAGE_TYPE_2D;
    image_info.format        = static_cast<VkFormat>(ktx_handle->vkFormat);
    image_info.extent.width  = ktx_handle->baseWidth;
    image_info.extent.height = ktx_handle->baseHeight;
    image_info.extent.depth  = 1;
    image_info.mipLevels     = ktx_handle->numLevels;
    image_info.arrayLayers   = ktx_handle->numLayers;
    image_info.samples       = VK_SAMPLE_COUNT_1_BIT;
    image_info.tiling        = VK_IMAGE_TILING_OPTIMAL;
    image_info.usage         = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    image_info.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
//    image_info.queueFamilyIndexCount;
//    image_info.pQueueFamilyIndices;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VmaAllocationCreateInfo allocation_info {};
    allocation_info.usage    = VMA_MEMORY_USAGE_AUTO;
    
    Vulkan::Image image;

    VK_ASSERT_THROW(vmaCreateImage(m_allocator, &image_info, &allocation_info, &image.handle, &image.allocation, nullptr), "Failed to create texture image");

    texture.image_id = static_cast<decltype(texture.image_id)>(m_image_pool.size() - 1); // Overflow possibility, but unlikely

    // TODO: Command buffer abstraction
    // NOTE: Shouldn't have to create these for every single texture created
    //       but rather manage uploading multiple at once on renderer level

    VkCommandBuffer command_buffer;
    VkCommandBufferAllocateInfo command_buffer_info {};
    command_buffer_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_info.pNext              = nullptr;
    command_buffer_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY; // Submit to queue, cannot be called from other buffers
    command_buffer_info.commandPool        = m_command_pool;
    command_buffer_info.commandBufferCount = 1;

    VK_ASSERT_THROW(vkAllocateCommandBuffers(m_device, &command_buffer_info, &command_buffer), "Failed to allocate command buffer");

    const Defer command_buffer_cleanup = [this, command_buffer]{
        vkFreeCommandBuffers(m_device, m_command_pool, 1, &command_buffer);
    };

    VkCommandBufferBeginInfo begin_info {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VK_ASSERT_THROW(vkBeginCommandBuffer(command_buffer, &begin_info), "Failure to begin image upload command buffer");

    { 
        VkImageMemoryBarrier image_memory_barrier
        {
            .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .pNext               = nullptr,
            .srcAccessMask       = VK_ACCESS_NONE,                       // Wait for these accesses...
            .dstAccessMask       = VK_ACCESS_TRANSFER_WRITE_BIT,         // ...and before these ones... 
            .oldLayout           = VK_IMAGE_LAYOUT_UNDEFINED,            // ...change layout...
            .newLayout           = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, // ...to this!
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image               = image.handle,
            .subresourceRange
            {
                .aspectMask      = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel    = 0,
                .levelCount      = ktx_handle->numLevels,
                .baseArrayLayer  = 0,
                .layerCount      = ktx_handle->numLayers,
            },
        };
        vkCmdPipelineBarrier(
            command_buffer, 
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            0, 0, nullptr, 0, nullptr, 1, &image_memory_barrier
        );
    }

    VkBufferImageCopy region_info {}; 
    region_info.bufferOffset = 0;
    region_info.bufferRowLength = 0;
    region_info.bufferImageHeight = 0;

    region_info.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region_info.imageSubresource.mipLevel = 0;
    region_info.imageSubresource.baseArrayLayer = 0;
    region_info.imageSubresource.layerCount = 1;

    region_info.imageOffset = {0,0,0};
    region_info.imageExtent = image_info.extent;

    vkCmdCopyBufferToImage(command_buffer, staging_buffer.handle, image.handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region_info);
            
    { 
        VkImageMemoryBarrier image_memory_barrier
        {
            .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .pNext               = nullptr,
            .srcAccessMask       = VK_ACCESS_TRANSFER_WRITE_BIT, 
            .dstAccessMask       = VK_ACCESS_SHADER_READ_BIT,
            .oldLayout           = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            .newLayout           = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image               = image.handle,
            .subresourceRange
            {
                .aspectMask      = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel    = 0,
                .levelCount      = ktx_handle->numLevels,
                .baseArrayLayer  = 0,
                .layerCount      = ktx_handle->numLayers,
            },
        };
        vkCmdPipelineBarrier(command_buffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            0, 0, nullptr, 0, nullptr, 1, &image_memory_barrier
        );
    }

    vkEndCommandBuffer(command_buffer);

    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.pCommandBuffers = &command_buffer;
    submit_info.commandBufferCount = 1;

    VK_ASSERT_THROW(vkQueueSubmit(m_graphics_queue, 1, &submit_info, VK_NULL_HANDLE), "Failed to submit command buffer");
    VK_ASSERT_THROW(vkQueueWaitIdle(m_graphics_queue), "Failure to wait for queue after image upload submission");

    VkImageViewCreateInfo view_info {};
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.pNext = VK_NULL_HANDLE;
    view_info.flags = 0;
    view_info.image = image.handle;
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = static_cast<VkFormat>(ktx_handle->vkFormat);
    view_info.components = {}; // Default rgb
    view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = ktx_handle->numLevels;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = ktx_handle->numLayers;

    // Store into image for now... 
    VK_ASSERT_THROW(vkCreateImageView(m_device, &view_info, nullptr, &image.view), "Failed to create image view for texture");

    VkSamplerCreateInfo sampler_info {};
    sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_info.pNext = nullptr;
    sampler_info.flags = 0;
    sampler_info.magFilter = VkFilter::VK_FILTER_LINEAR;
    sampler_info.minFilter = VkFilter::VK_FILTER_LINEAR;
    sampler_info.addressModeU = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeV = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeW = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.mipmapMode = VkSamplerMipmapMode::VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_info.mipLodBias = 0.0f;
    sampler_info.anisotropyEnable = VK_TRUE; 
    sampler_info.maxAnisotropy = m_physical_device_properties.limits.maxSamplerAnisotropy;
    sampler_info.compareEnable = VK_FALSE;
    sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
    sampler_info.minLod = 0.0f;
    sampler_info.maxLod = 0.0f;
    sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    sampler_info.unnormalizedCoordinates = VK_FALSE;

    VK_ASSERT_THROW(vkCreateSampler(m_device, &sampler_info, nullptr, &image.sampler), "Failed to create image sampler for texture");

    m_image_pool.emplace_back(image);
    m_texture_list.emplace_back(texture);

    return static_cast<TextureID>(m_texture_list.size() - 1); // Overflow danger
}

void Driver::texture_destroy(TextureID texture_id)
{
    TextureContext& texture = m_texture_list[texture_id];

    this->image_destroy(texture.image_id);

    // No invalidation state, should be done in ownership pool
}

void Driver::image_destroy(ImageID image_id)
{
    Image& image = m_image_pool[image_id];

    if (!image.handle) return; //

    vkDestroySampler(m_device, image.sampler, nullptr);
    vkDestroyImageView(m_device, image.view, nullptr);
    vmaDestroyImage(m_allocator, image.handle, image.allocation);

    image = {};
}

}
