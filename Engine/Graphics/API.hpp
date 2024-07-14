#include "Graphics/Types.hpp"

#include "Graphics/Vulkan/Driver.hpp"

#include "Shared/ClassUtility.hpp"

#include <vector>

namespace Cr::Core { class Window; }

namespace Cr::Graphics
{

class API : public NoValueSemantics
{
    public:
        API(const Core::Window& surface_context);

        [[nodiscard]]
        ShaderID shader_create(const std::vector<u8>& vertex_spirv, const std::vector<u8>& fragment_spirv);
        void     shader_set_uniform(ShaderID id, const UniformBufferObject& uniforms);
        void     shader_destroy(ShaderID shader_id);

        // TODO Abstract vertex data layout
        [[nodiscard]]
        MeshID mesh_create(const std::vector<Vertex>& vertices, const std::vector<u32>& indices);
        void   mesh_destroy(MeshID mesh_id);

        [[nodiscard]]
        TextureID texture_create(const std::string& path);
        void      texture_destroy(TextureID texture_id);

        void begin_render();
        void draw(MeshID mesh_id, ShaderID shader_id, const PushConstantObject& push_constants);
        void end_render();

    private:
        std::vector<Mesh> meshes;
        std::vector<Texture> textures;

        Vulkan::Driver driver;
};

}
