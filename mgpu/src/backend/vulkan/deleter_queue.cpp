
#include <limits>

#include "deleter_queue.hpp"

namespace mgpu::vulkan {

void DeleterQueue::Schedule(DeletionFn deletion) {
  m_pending_deletions.emplace_back(std::move(deletion), m_current_timestamp);
}

void DeleterQueue::Drain(u64 until_timestamp) {
  size_t i = 0;
  while(i < m_pending_deletions.size() && m_pending_deletions[i].timestamp <= until_timestamp) {
    m_pending_deletions[i++].deletion_fn();
  }
  m_pending_deletions.erase(m_pending_deletions.begin(), m_pending_deletions.begin() + i);
}

void DeleterQueue::DrainAll() {
  Drain(std::numeric_limits<u64>::max());
}

}  // namespace mgpu::vulkan
