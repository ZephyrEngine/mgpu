
#pragma once

#include <atom/integer.hpp>
#include <functional>
#include <vector>

namespace mgpu::vulkan {

class DeleterQueue {
  public:
    using DeletionFn = std::function<void(void)>;

    void Schedule(DeletionFn deletion_fn);
    void Drain(u64 until_timestamp);
    void DrainAll();
    [[nodiscard]] u64 GetTimestamp() const { return m_current_timestamp; }
    void BumpTimestamp() { m_current_timestamp++; }

  private:
    struct PendingDelete {
      DeletionFn deletion_fn;
      u64 timestamp{};
    };

    std::vector<PendingDelete> m_pending_deletions{};
    u64 m_current_timestamp{};
};

}  // namespace mgpu::vulkan
