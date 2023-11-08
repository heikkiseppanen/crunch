#pragma once

#include "Crunch.hpp"
#include "Shared/ClassUtility.hpp"

#include <vulkan/vulkan_core.h>
#include <vk_mem_alloc.h>

#include <vector>
#include <algorithm>

namespace Cr::Vk
{

struct Buffer 
{
	public:
		VkBuffer handle;

		Buffer() = default;
		Buffer(VmaAllocator allocator, const VmaAllocationCreateInfo& allocation_info, const VkBufferCreateInfo& buffer_info);

		Buffer(Buffer&& other);
		Buffer& operator = (Buffer&& other);

		~Buffer();

		template<typename InputIt>
		inline void map(InputIt begin, InputIt end, u64 byte_offset)
		{
			typedef typename std::iterator_traits<InputIt>::value_type value_type;

			std::copy(begin, end, static_cast<value_type*>(m_properties.pMappedData) + byte_offset);
		}

		void flush();
	
	private:
		VmaAllocator m_allocator;
		VmaAllocation m_allocation;
		VmaAllocationInfo m_properties;

}; // struct Buffer

} // namespace Cr::Vk
