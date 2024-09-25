#include "Graphics/SPIRVReflection.hpp"

namespace Cr::Graphics
{

static constexpr const char* to_string(SpvReflectResult value);

#define SPV_ASSERT_THROW(COND, ...) CR_ASSERT_THROW((COND) == SPV_REFLECT_RESULT_SUCCESS, __VA_ARGS__)

SPIRVReflection::SPIRVReflection(std::span<const U8> spirv_binary)
    : m_reflection(create_unique<spv_reflect::ShaderModule>(spirv_binary.size(), spirv_binary.data()))
{
    auto result = m_reflection->GetResult();
    SPV_ASSERT_THROW(result, "SPIRV Reflection failure: {}", to_string(result));
}


SPIRVReflection::~SPIRVReflection() = default; // Workaround for forward declaration of type in std::unique_ptr 

std::vector<SpvReflectInterfaceVariable*> SPIRVReflection::get_inputs() const
{
    U32 count;
    auto result = m_reflection->EnumerateInputVariables(&count, nullptr);
    SPV_ASSERT_THROW(result, "Failed to get SPIRV input variable count: {}", to_string(result));

    std::vector<SpvReflectInterfaceVariable*> variables(count);
    result = m_reflection->EnumerateInputVariables(&count, variables.data());
    SPV_ASSERT_THROW(result, "Failed to enumerate SPIRV input variables: {}", to_string(result));

    return variables;
}

std::vector<SpvReflectDescriptorSet*> SPIRVReflection::get_descriptor_sets() const
{
    U32 count;
    auto result = m_reflection->EnumerateDescriptorSets(&count, nullptr);
    SPV_ASSERT_THROW(result, "Failed to get SPIRV descriptor set count: {}", to_string(result));

    std::vector<SpvReflectDescriptorSet*> sets(count);
    result = m_reflection->EnumerateDescriptorSets(&count, sets.data());
    SPV_ASSERT_THROW(result, "Failed to enumerate SPIRV descriptor sets: {}", to_string(result));

    return sets;
}

static constexpr const char* to_string(SpvReflectResult result)
{
    switch(result)
    {
        case SPV_REFLECT_RESULT_ERROR_PARSE_FAILED                         : return "PARSE_FAILED";
        case SPV_REFLECT_RESULT_ERROR_ALLOC_FAILED                         : return "ALLOC_FAILED";
        case SPV_REFLECT_RESULT_ERROR_RANGE_EXCEEDED                       : return "RANGE_EXCEEDED";
        case SPV_REFLECT_RESULT_ERROR_NULL_POINTER                         : return "NULL_POINTER";
        case SPV_REFLECT_RESULT_ERROR_INTERNAL_ERROR                       : return "INTERNAL_ERROR";
        case SPV_REFLECT_RESULT_ERROR_COUNT_MISMATCH                       : return "COUNT_MISMATCH";
        case SPV_REFLECT_RESULT_ERROR_ELEMENT_NOT_FOUND                    : return "ELEMENT_NOT_FOUND";
        case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_CODE_SIZE              : return "SPIRV_INVALID_CODE_SIZE";
        case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_MAGIC_NUMBER           : return "SPIRV_INVALID_MAGIC_NUMBER";
        case SPV_REFLECT_RESULT_ERROR_SPIRV_UNEXPECTED_EOF                 : return "SPIRV_UNEXPECTED_EOF";
        case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_ID_REFERENCE           : return "SPIRV_INVALID_ID_REFERENCE";
        case SPV_REFLECT_RESULT_ERROR_SPIRV_SET_NUMBER_OVERFLOW            : return "SPIRV_SET_NUMBER_OVERFLOW";
        case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_STORAGE_CLASS          : return "SPIRV_INVALID_STORAGE_CLASS";
        case SPV_REFLECT_RESULT_ERROR_SPIRV_RECURSION                      : return "SPIRV_RECURSION";
        case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_INSTRUCTION            : return "SPIRV_INVALID_INSTRUCTION";
        case SPV_REFLECT_RESULT_ERROR_SPIRV_UNEXPECTED_BLOCK_DATA          : return "SPIRV_UNEXPECTED_BLOCK_DATA";
        case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_BLOCK_MEMBER_REFERENCE : return "SPIRV_INVALID_BLOCK_MEMBER_REFERENCE";
        case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_ENTRY_POINT            : return "SPIRV_INVALID_ENTRY_POINT";
        case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_EXECUTION_MODE         : return "SPIRV_INVALID_EXECUTION_MODE";
        case SPV_REFLECT_RESULT_ERROR_SPIRV_MAX_RECURSIVE_EXCEEDED         : return "SPIRV_MAX_RECURSIVE_EXCEEDED";
        default: return "UNKNOWN";
    }
};

}
