
#pragma once

#include <functional>
#include <vector>

namespace mgpu::vulkan {

class DeleterQueue {
  public:
    using DeletionFn = std::function<void(void)>;

    void Schedule(DeletionFn deletion);
    void Drain();

  private:
    std::vector<DeletionFn> m_pending_deletions{};
};

}  // namespace mgpu::vulkan
