
#include "backend/vulkan/lib/vulkan_result.hpp"
#include "buffer.hpp"
#include "conversion.hpp"

namespace mgpu::vulkan {

bool Buffer::State::operator==(const State& other_state) const {
  return m_access == other_state.m_access && m_pipeline_stages == other_state.m_pipeline_stages;
}

Buffer::Buffer(Device* device, VkBuffer vk_buffer, VmaAllocation vma_allocation, const MGPUBufferCreateInfo& create_info)
    : BufferBase{create_info}
    , m_device{device}
    , m_vk_buffer{vk_buffer}
    , m_vma_allocation{vma_allocation} {
}

Buffer::~Buffer() {
  Unmap();

  // Defer deletion of underlying Vulkan resources until the currently recorded frame has been fully processed on the GPU.
  // TODO(fleroviux): make this a little bit less verbose.
  Device* device = m_device;
  VkBuffer vk_buffer = m_vk_buffer;
  VmaAllocation vma_allocation = m_vma_allocation;
  device->GetDeleterQueue().Schedule([device, vk_buffer, vma_allocation]() {
    vkDestroyBuffer(device->Handle(), vk_buffer, nullptr);
    vmaFreeMemory(device->GetVmaAllocator(), vma_allocation);
  });
}

Result<BufferBase*> Buffer::Create(Device* device, const MGPUBufferCreateInfo& create_info) {
  const VkBufferCreateInfo vk_buffer_create_info{
    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .size = create_info.size,
    .usage = MGPUBufferUsageToVkBufferUsage(create_info.usage),
    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    .queueFamilyIndexCount = 0,
    .pQueueFamilyIndices = nullptr
  };

  VmaAllocationCreateInfo vma_alloc_create_info{
    .flags = 0,
    .usage = VMA_MEMORY_USAGE_AUTO
  };

  if(create_info.flags & MGPU_BUFFER_FLAGS_HOST_VISIBLE) {
    vma_alloc_create_info.flags |= (create_info.flags & MGPU_BUFFER_FLAGS_HOST_RANDOM_ACCESS) ?
      VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT : VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
  }

  VmaAllocator vma_allocator = device->GetVmaAllocator();

  VkBuffer vk_buffer{};
  VmaAllocation vma_allocation{};
  MGPU_VK_FORWARD_ERROR(vmaCreateBuffer(vma_allocator, &vk_buffer_create_info, &vma_alloc_create_info, &vk_buffer, &vma_allocation, nullptr));
  return new Buffer{device, vk_buffer, vma_allocation, create_info};
}

bool Buffer::IsMapped() const {
  return m_mapped_address != nullptr;
}

Result<void*> Buffer::Map() {
  if(IsMapped()) {
    return m_mapped_address;
  }
  MGPU_VK_FORWARD_ERROR(vmaMapMemory(m_device->GetVmaAllocator(), m_vma_allocation, &m_mapped_address));
  return m_mapped_address;
}

MGPUResult Buffer::Unmap() {
  if(IsMapped()) {
    vmaUnmapMemory(m_device->GetVmaAllocator(), m_vma_allocation);
    m_mapped_address = nullptr;
  }
  return MGPU_SUCCESS;
}

MGPUResult Buffer::FlushRange(u64 offset, u64 size) {
  MGPU_VK_FORWARD_ERROR(vmaFlushAllocation(m_device->GetVmaAllocator(), m_vma_allocation, offset, size));
  return MGPU_SUCCESS;
}

void Buffer::TransitionState(State new_state, VkCommandBuffer vk_command_buffer) {
  // thoughts:
  // read-read dependency may not need a barrier (does it ever need one?)
  // read-write and write-write dependencies always need a barrier
  //ATOM_PANIC("transition state unimplemented");
}

}  // namespace mgpu::vulkan
