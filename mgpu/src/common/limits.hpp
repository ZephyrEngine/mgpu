
#pragma once

#include <atom/integer.hpp>

namespace mgpu::limits {

// Hard implementation-defined resource limits
static constexpr size_t max_color_attachments = 8u;
static constexpr size_t max_total_attachments = max_color_attachments + 1u;

} // namespace mgpu::limits
