
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
  const auto AccessFlagsHaveWrite = [](VkAccessFlags access_flags) {
    return access_flags & (
      VK_ACCESS_SHADER_WRITE_BIT |
      VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
      VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
      VK_ACCESS_TRANSFER_WRITE_BIT |
      VK_ACCESS_HOST_WRITE_BIT |
      VK_ACCESS_MEMORY_WRITE_BIT
    );
  };

  // Add a barrier if the state has changed or if there is a Read-Write, Write-Read or Write-Write dependency.
  const bool old_state_has_write = AccessFlagsHaveWrite(m_state.m_access);
  const bool new_state_has_write = AccessFlagsHaveWrite(new_state.m_access);
  if(m_state != new_state || old_state_has_write || new_state_has_write) {
    const VkBufferMemoryBarrier vk_buffer_barrier{
      .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
      .pNext = nullptr,
      .srcAccessMask = m_state.m_access,
      .dstAccessMask = new_state.m_access,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .buffer = m_vk_buffer,
      .offset = 0u,
      .size = VK_WHOLE_SIZE
    };

    // TODO(fleroviux): attempt to batch multiple pipeline barriers together?
    vkCmdPipelineBarrier(vk_command_buffer, m_state.m_pipeline_stages, new_state.m_pipeline_stages, 0, 0u, nullptr, 1u, &vk_buffer_barrier, 0u, nullptr);
  }

  m_state = new_state;
}

}  // namespace mgpu::vulkan
