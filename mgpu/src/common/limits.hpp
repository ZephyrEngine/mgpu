
#pragma once

#include <atom/integer.hpp>

namespace mgpu {

// Implementation-defined hard limits go here:
static constexpr size_t k_max_color_attachments = 16u;
static constexpr size_t k_max_attachments = k_max_color_attachments + 1u;

} // namespace mgpu
