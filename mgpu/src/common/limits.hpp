
#pragma once

#include <atom/integer.hpp>

namespace mgpu::limits {

// Hard implementation-defined resource limits
static constexpr size_t max_color_attachments = 8u;
static constexpr size_t max_total_attachments = max_color_attachments + 1u;
static constexpr size_t max_vertex_input_bindings = 32;
static constexpr size_t max_vertex_input_attributes = 64;

} // namespace mgpu::limits
