#pragma once

#include "Graphics/Graphics.hpp"
#include "Crunch/ClassUtility.hpp"

#include <spirv_reflect.h>

namespace Cr::Graphics
{

class SPIRVReflection : public NoCopy
{
    public:
        SPIRVReflection(std::span<const U8> spirv_binary);
        ~SPIRVReflection();

        std::vector<SpvReflectDescriptorSet*>     get_descriptor_sets() const;
        std::vector<SpvReflectInterfaceVariable*> get_inputs() const;

    private:
        Unique<spv_reflect::ShaderModule> m_reflection;
};

}
