#include "Crunch/Crunch.hpp"
#include "Crunch/Math.hpp"

#include "Core/Window.hpp"
#include "Core/Input.hpp"

#include "Graphics/Vulkan/API.hpp"
#include "Graphics/Mesh.hpp"

#include "Crunch/Filesystem.hpp"

// Temp headers and values
#include <glm/gtx/rotate_vector.hpp>
#include <ktx.h>

#include <iostream>

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    using namespace Cr;
    using namespace Cr::Graphics;

    try
    {
        constexpr U32 WINDOW_WIDTH      = 1280;
        constexpr U32 WINDOW_HEIGHT     = 720;
        constexpr F32 FOV               = 90.0f;
        constexpr F32 ASPECT_RATIO      = F32(WINDOW_WIDTH) / F32(WINDOW_HEIGHT);
        constexpr F32 MOUSE_SENSITIVITY = 0.5f;
        constexpr F32 MOVEMENT_SPEED    = 2.0f;

        Cr::Core::Window window { WINDOW_WIDTH, WINDOW_HEIGHT, "Crunch" };
        Cr::Core::Input  input  { window.get_native() };

        Cr::Graphics::Vulkan::API vk { window };

        // MESH 

        const auto vertices = get_cube_vertices(1.0f, 0);
        const auto indices  = get_cube_indices(0);

        const VkDeviceSize vertices_size = sizeof(vertices[0]) * vertices.size();
        const VkDeviceSize indices_size  = sizeof(indices[0])  * indices.size();

        const auto vertices_buffer = vk.create_buffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, vertices_size);
        const auto indices_buffer  = vk.create_buffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT  | VK_BUFFER_USAGE_TRANSFER_DST_BIT, indices_size);

        const U32 mesh_index_count = indices.size();

        {
        
            // TODO Staging buffer reuse
            auto vertices_staging = vk.create_buffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, vertices_size);
            auto indices_staging  = vk.create_buffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, indices_size);

            vertices_staging->set_data(vertices.data(), vertices_size, 0);
            indices_staging->set_data(indices.data(),  indices_size,  0);

            auto& queue = vk.get_command_queue(VK_QUEUE_TRANSFER_BIT);
            CR_ASSERT(queue.get_family_index() == vk.get_command_queue(VK_QUEUE_GRAPHICS_BIT).get_family_index(), "Queue transfer for mesh uploads not implemented yet");

            auto cmd = queue.create_command_buffer();

            cmd->begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

            VkBufferCopy copy { .size = vertices_size };
            cmd->copy_buffer(*vertices_staging, *vertices_buffer, {&copy, 1});

            copy.size = indices_size;
            cmd->copy_buffer(*indices_staging, *indices_buffer, {&copy, 1});

            cmd->end();

            const VkSubmitInfo submission {
                .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                .commandBufferCount = 1,
                .pCommandBuffers = &cmd->get_native(),
            };

            queue.submit({&submission, 1}, nullptr);

            // TODO Handle with fence or semaphore 
            queue.wait_for_idle();
        }

        // TEXTURE

        Cr::Unique<Cr::Graphics::Vulkan::Texture> texture {};
        {
            ktxTexture2* ktx_texture;

            CR_ASSERT_THROW(ktxTexture2_CreateFromNamedFile("Assets/Textures/T_CrunchLogo_D.ktx2", KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktx_texture) == KTX_SUCCESS, "Failed to load ktx image");
            CR_DEFER { ktxTexture_Destroy(ktxTexture(ktx_texture)); };

            const ktx_uint8_t* data = ktxTexture_GetData(ktxTexture(ktx_texture));
            const ktx_size_t   size = ktxTexture_GetDataSize(ktxTexture(ktx_texture));

            texture = vk.create_texture(static_cast<VkFormat>(ktx_texture->vkFormat), {ktx_texture->baseWidth, ktx_texture->baseHeight, ktx_texture->baseDepth});

            auto texture_staging = vk.create_buffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, size);

            texture_staging->set_data(data, size, 0);

            auto& queue = vk.get_command_queue(VK_QUEUE_TRANSFER_BIT);
            CR_ASSERT(queue.get_family_index() == vk.get_command_queue(VK_QUEUE_GRAPHICS_BIT).get_family_index(), "Queue transfer for mesh uploads not implemented yet");

            auto cmd = queue.create_command_buffer();

            cmd->begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

            { 
                VkImageMemoryBarrier image_memory_barrier {
                    .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                    .pNext               = nullptr,
                    .srcAccessMask       = VK_ACCESS_NONE,                       // Wait for these accesses...
                    .dstAccessMask       = VK_ACCESS_TRANSFER_WRITE_BIT,         // ...and before these ones... 
                    .oldLayout           = VK_IMAGE_LAYOUT_UNDEFINED,            // ...change layout...
                    .newLayout           = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, // ...to this!
                    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .image               = texture->get_native(),
                    .subresourceRange
                    {
                        .aspectMask      = VK_IMAGE_ASPECT_COLOR_BIT,
                        .baseMipLevel    = 0,
                        .levelCount      = ktx_texture->numLevels,
                        .baseArrayLayer  = 0,
                        .layerCount      = ktx_texture->numLayers,
                    },
                };

                cmd->pipeline_barrier(
                    VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                    VK_PIPELINE_STAGE_TRANSFER_BIT,
                    0,
                    {}, {}, {&image_memory_barrier, 1}
                );
            }

            VkBufferImageCopy copy { 
                .bufferOffset      = 0,
                .bufferRowLength   = 0,
                .bufferImageHeight = 0,
                .imageSubresource  = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .layerCount = 1,
                },
                .imageExtent       = {
                    .width  = ktx_texture->baseWidth,
                    .height = ktx_texture->baseHeight,
                    .depth  = ktx_texture->baseDepth,
                }
            };

            cmd->copy_buffer_to_texture(*texture_staging, *texture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, {&copy, 1});

            { 
                VkImageMemoryBarrier image_memory_barrier {
                    .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                    .pNext               = nullptr,
                    .srcAccessMask       = VK_ACCESS_TRANSFER_WRITE_BIT,
                    .dstAccessMask       = VK_ACCESS_SHADER_READ_BIT,
                    .oldLayout           = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    .newLayout           = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, // TODO QUEUE FAMILIES
                    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, // TODO QUEUE FAMILIES
                    .image               = texture->get_native(),
                    .subresourceRange
                    {
                        .aspectMask      = VK_IMAGE_ASPECT_COLOR_BIT,
                        .baseMipLevel    = 0,
                        .levelCount      = ktx_texture->numLevels,
                        .baseArrayLayer  = 0,
                        .layerCount      = ktx_texture->numLayers,
                    },
                };

                cmd->pipeline_barrier(
                    VK_PIPELINE_STAGE_TRANSFER_BIT,
                    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                    0,
                    {}, {}, {&image_memory_barrier, 1}
                );
            }

            cmd->end();

            const VkSubmitInfo submission {
                .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                .commandBufferCount = 1,
                .pCommandBuffers = &cmd->get_native(),
            };

            queue.submit({&submission, 1}, nullptr);

            // TODO Handle with fence or semaphore 
            queue.wait_for_idle();
        }


        Cr::Unique<Cr::Graphics::Vulkan::Shader> shader {};

        // TODO In reality, all modules should be compiled first then assembled into pipelines under a single call
        {
            const auto vert_source = Cr::read_binary_file("Assets/Shaders/triangle.vert.spv");
            const auto frag_source = Cr::read_binary_file("Assets/Shaders/triangle.frag.spv");

            const std::array modules = {
                vk.create_shader_module(vert_source, VK_SHADER_STAGE_VERTEX_BIT),
                vk.create_shader_module(frag_source, VK_SHADER_STAGE_FRAGMENT_BIT)
            };

            const std::array refs = { modules[0].get(), modules[1].get() };
            shader = vk.create_shader(VK_PIPELINE_BIND_POINT_GRAPHICS, {refs});
        }

        // UNIFORM BUFFER FOR STATIC FRAME DATA
        auto frame_uniform_buffer = vk.create_buffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(Cr::Graphics::UniformBufferObject));

        VkDescriptorSetAllocateInfo desc_info {
            .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool     = vk.m_descriptor_pool,
            .descriptorSetCount = 1,
            .pSetLayouts        = &shader->get_descriptor_set_layout(),
        };

        VkDescriptorSet descriptor_set = nullptr;
        VK_ASSERT_THROW(vkAllocateDescriptorSets(vk.m_device, &desc_info, &descriptor_set), "Failed to allocate descriptor set"); 

        VkDescriptorBufferInfo descriptor_buffer_info {
            .buffer = frame_uniform_buffer->get_native(),
            .offset = 0,
            .range = sizeof(Cr::Graphics::UniformBufferObject),
        };

        VkDescriptorImageInfo descriptor_image_info {
            .sampler     = texture->get_sampler(),
            .imageView   = texture->get_view(),
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        };

        std::array writes {
            VkWriteDescriptorSet {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = descriptor_set,
                .dstBinding = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .pBufferInfo = &descriptor_buffer_info,
            },
            VkWriteDescriptorSet {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = descriptor_set,
                .dstBinding = 1,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .pImageInfo = &descriptor_image_info,
            },
        };

        vkUpdateDescriptorSets(vk.m_device, writes.size(), writes.data(), 0, nullptr);

        // SCENE

        Cr::Vec3f cube_position   { 1.0f, 0.0f, 0.0f };
        // Cr::Vec3f sphere_position {-1.0f, 0.0f, 0.0f };

        auto cube_matrix   = glm::translate(Cr::Mat4f{1.0f}, cube_position);
        //auto sphere_matrix = glm::translate(Cr::Mat4f{1.0f}, sphere_position);

        // CAMERA

        Cr::Vec3f camera_position  { 0.0f, 0.0f, 2.0f };
        Cr::Vec3f camera_xyz_euler { 0.0f, 0.0f,  0.0f };

        Cr::Vec3f camera_target = Cr::Quatf{camera_xyz_euler} * Cr::VEC3F_FORWARD;
        Cr::Vec3f camera_right  = glm::normalize(glm::cross(-camera_target, Cr::VEC3F_UP));
        Cr::Vec3f camera_up     = glm::normalize(glm::cross(-camera_target, camera_right));

        Cr::Vec2f mouse_last_position = input.get_mouse_position();

        // Main Loop

        F32 time = window.get_time();

        while (!window.should_close())
        {
            F32 time_delta = window.get_time() - time;
            time += time_delta;

            window.poll_events();

            if (input.is_key_down(Cr::Key::ESCAPE))
            {
                window.set_to_close();
            }

            const Cr::Vec3f axis
            {
                -F32(input.is_key_down(Cr::Key::A))            + F32(input.is_key_down(Cr::Key::D)),
                -F32(input.is_key_down(Cr::Key::LEFT_CONTROL)) + F32(input.is_key_down(Cr::Key::SPACE)),
                -F32(input.is_key_down(Cr::Key::S))            + F32(input.is_key_down(Cr::Key::W)),
            };

            const Cr::Vec3f camera_axis_offset = axis * time_delta * MOVEMENT_SPEED;

            camera_position += camera_axis_offset.x * camera_right;
            camera_position += camera_axis_offset.y * camera_up;
            camera_position += camera_axis_offset.z * -camera_target;

            const auto mouse_new_position = input.get_mouse_position();
            const auto mouse_offset = mouse_new_position - mouse_last_position;
            mouse_last_position = mouse_new_position;

            camera_xyz_euler.x -= MOUSE_SENSITIVITY * mouse_offset.y * time_delta;
            camera_xyz_euler.y -= MOUSE_SENSITIVITY * mouse_offset.x * time_delta;

            camera_xyz_euler.x = glm::clamp(camera_xyz_euler.x, -Cr::PI / 2 + 0.0001f, Cr::PI / 2 - 0.0001f);

            camera_target = Cr::Quatf{camera_xyz_euler} * Cr::VEC3F_FORWARD;
            camera_right  = glm::normalize(glm::cross(-camera_target, Cr::VEC3F_UP));
            camera_up     = glm::normalize(glm::cross(camera_target, camera_right));

            // std::cout << camera_forward.x << ' ' << camera_forward.y << ' ' << camera_forward.z << '\n';

            // std::cout << camera_right.x << ' ' << camera_right.y << ' ' << camera_right.z << '\n';
            // std::cout << camera_up.x << ' ' << camera_up.y << ' ' << camera_up.z << '\n' << '\n';

            const Cr::Mat4f perspective_matrix = glm::perspective(glm::radians(FOV), ASPECT_RATIO, 0.1f, 100.0f);
            const Cr::Mat4f view_matrix        = glm::lookAt(camera_position, camera_position - camera_target, camera_up);
            
            Cr::Graphics::UniformBufferObject frame_data {
                .projected_view = perspective_matrix * view_matrix
            };

            const F32 rotation_velocity = 50.0f * time_delta;

            cube_matrix   = glm::rotate(cube_matrix,   glm::radians( rotation_velocity), Cr::VEC3F_UP);
            //sphere_matrix = glm::rotate(sphere_matrix, glm::radians(-rotation_velocity), Cr::VEC3F_UP);

            Cr::Graphics::PushConstantObject instance_data {
                .model = cube_matrix
            };
            
            // RENDER PIPELINE

            frame_uniform_buffer->set_data(&frame_data, sizeof(frame_data), 0);

            auto& cmd = vk.begin_frame();

            cmd.bind_shader(*shader);

            cmd.bind_vertex_buffer(*vertices_buffer);
            cmd.bind_index_buffer(*indices_buffer, VK_INDEX_TYPE_UINT32);

            cmd.bind_descriptor_set(*shader, descriptor_set);

            cmd.push_constants(*shader, instance_data);

            cmd.draw_indexed(mesh_index_count, 1, 0, 0, 0);

            vk.end_frame();
        }

        vkDeviceWaitIdle(vk.m_device);
        vkFreeDescriptorSets(vk.m_device, vk.m_descriptor_pool, 1, &descriptor_set);
    }
    catch (std::exception& e)
    {
        std::cout << e.what();
    }

    return (0);
}
