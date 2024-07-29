
#include "deleter_queue.hpp"

namespace mgpu::vulkan {

void DeleterQueue::Schedule(DeletionFn deletion) {
  m_pending_deletions.push_back(std::move(deletion));
}

void DeleterQueue::Drain() {
  for(const auto& deletion : m_pending_deletions) {
    deletion();
  }

  m_pending_deletions.clear();
}

}  // namespace mgpu::vulkan
