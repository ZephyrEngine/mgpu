
#pragma once

#include <atom/integer.hpp>
#include <atom/panic.hpp>
#include <algorithm>
#include <functional>
#include <optional>
#include <span>
#include <vector>
#include <vulkan/vulkan.h>

namespace mgpu::vulkan {

struct TextureState {
  VkImageLayout m_image_layout{VK_IMAGE_LAYOUT_UNDEFINED};
  VkAccessFlags m_access{};
  VkPipelineStageFlags m_pipeline_stages{};

  bool operator==(const TextureState& other_state) const {
    return m_image_layout == other_state.m_image_layout &&
           m_access == other_state.m_access &&
           m_pipeline_stages == other_state.m_pipeline_stages;
  }
};

struct Rect {
  u32 min[2];
  u32 max[2];

  [[nodiscard]] bool Intersects(const Rect& other_rect) const {
    // Use >= instead of > because 'max' refers to the highest included cell and thus lies within the geometric rectangle.
    const bool result = max[0] >= other_rect.min[0] && other_rect.max[0] >= min[0] &&
                        max[1] >= other_rect.min[1] && other_rect.max[1] >= min[1];
    return result;
  }

  [[nodiscard]] std::optional<Rect> GetIntersectionWith(const Rect& other_rect) const {
    if(Intersects(other_rect)) {
      // TODO: refactor the texture state out of this.
      return Rect{
        .min = { std::max(min[0], other_rect.min[0]), std::max(min[1], other_rect.min[1]) },
        .max = { std::min(max[0], other_rect.max[0]), std::min(max[1], other_rect.max[1]) },
      };
    }
    return std::nullopt;
  }

  [[nodiscard]] bool IsCongruentTo(const Rect& other_rect) const {
    return min[0] == other_rect.min[0] &&
           min[1] == other_rect.min[1] &&
           max[0] == other_rect.max[0] &&
           max[1] == other_rect.max[1];
  }
};

struct TextureSubresourceState {
  public:
    TextureSubresourceState() = default; // TODO: get rid of this entirely
    TextureSubresourceState(Rect rect, TextureState state) : m_rect{rect}, m_state{state} {}

    TextureSubresourceState(u32 base_array_layer, u32 array_layer_count, u32 base_mip, u32 mip_count, TextureState state)
        : m_rect{Rect{
          .min = {base_array_layer, base_mip},
          .max = {base_array_layer + array_layer_count - 1u, base_mip + mip_count - 1u}
        }}
        , m_state{state} {
    }

    [[nodiscard]] u32 BaseArrayLayer() const {
      return m_rect.min[0];
    }

    [[nodiscard]] u32 ArrayLayerCount() const {
      return m_rect.max[0] - m_rect.min[0] + 1u;
    }

    [[nodiscard]] u32 BaseMip() const {
      return m_rect.min[1];
    }

    [[nodiscard]] u32 MipCount() const {
      return m_rect.max[1] - m_rect.min[1] + 1u;
    }

    [[nodiscard]] const Rect& GetRect() const {
      return m_rect;
    }

    [[nodiscard]] const TextureState& State() const {
      return m_state;
    }

  private:
    // TODO: get rid of this
    friend class TextureStateTracker;

    Rect m_rect{};
    TextureState m_state{};
};

// TODO: reimplement pool

/**
 * For tracking the texture subresource states we require:
 * - knowing all the rects we intersected so that we can generate one barrier for every "from" state
 * - we also need to know the intersecting areas
 */
class TextureStateTracker {
  public:
    TextureStateTracker(u32 array_layer_count, u32 mip_count) {
      // This ensures we don't invalidate iterators when adding new rects to the vector.
      m_rects.reserve(MaxSimultaneousRects(array_layer_count, mip_count));

      // Add an initial rect that covers the entire grid.
      const auto rect = new TextureSubresourceState{0u, array_layer_count, 0u, mip_count, {
        .m_image_layout = VK_IMAGE_LAYOUT_UNDEFINED,
        .m_access = 0,
        .m_pipeline_stages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT
      }};
      m_rects.push_back(rect);
    }

    [[nodiscard]] std::span<const TextureSubresourceState* const> GetRects() const {
      return m_rects;
    }

    void TransitionRect(const TextureSubresourceState& rect_spec, std::function<void(TextureSubresourceState)> on_intersection = [](TextureSubresourceState) {}) {
      std::vector<TextureSubresourceState*>::iterator next_iter_rect;

      for(auto iter_rect = m_rects.begin(); iter_rect != m_rects.end(); iter_rect = next_iter_rect) {
        TextureSubresourceState& maybe_intersecting_rect = **iter_rect;

        next_iter_rect = std::next(iter_rect);

        std::optional<Rect> intersection_rect = rect_spec.m_rect.GetIntersectionWith(maybe_intersecting_rect.m_rect);
        if(!intersection_rect.has_value()) {
          continue;
        }
        on_intersection(TextureSubresourceState{intersection_rect.value(), maybe_intersecting_rect.m_state});

        if(rect_spec.m_rect.IsCongruentTo(maybe_intersecting_rect.m_rect)) {
          if(maybe_intersecting_rect.m_state != rect_spec.m_state) {
            maybe_intersecting_rect.m_state = rect_spec.m_state;
            TryMergeWithNeighbours(maybe_intersecting_rect);
          }
          return;
        }

        // Create up to four new rects with the same state in each direction

        if(rect_spec.m_rect.min[0] > maybe_intersecting_rect.m_rect.min[0]) {
          const auto new_rect = new TextureSubresourceState{};
          *new_rect = maybe_intersecting_rect;
          new_rect->m_rect.max[0] = rect_spec.m_rect.min[0] - 1;
          m_rects.push_back(new_rect);
        }

        if(rect_spec.m_rect.max[0] < maybe_intersecting_rect.m_rect.max[0]) {
          const auto new_rect = new TextureSubresourceState{};
          *new_rect = maybe_intersecting_rect;
          new_rect->m_rect.min[0] = rect_spec.m_rect.max[0] + 1;
          m_rects.push_back(new_rect);
        }

        if(rect_spec.m_rect.min[1] > maybe_intersecting_rect.m_rect.min[1]) {
          const auto new_rect = new TextureSubresourceState{};
          new_rect->m_rect.min[0] = rect_spec.m_rect.min[0];
          new_rect->m_rect.max[0] = rect_spec.m_rect.max[0];
          new_rect->m_rect.min[1] = maybe_intersecting_rect.m_rect.min[1];
          new_rect->m_rect.max[1] = rect_spec.m_rect.min[1] - 1;
          new_rect->m_state = maybe_intersecting_rect.State();
          m_rects.push_back(new_rect);
        }

        if(rect_spec.m_rect.max[1] < maybe_intersecting_rect.m_rect.max[1]) {
          const auto new_rect = new TextureSubresourceState{};
          new_rect->m_rect.min[0] = rect_spec.m_rect.min[0];
          new_rect->m_rect.max[0] = rect_spec.m_rect.max[0];
          new_rect->m_rect.min[1] = rect_spec.m_rect.max[1] + 1;
          new_rect->m_rect.max[1] = maybe_intersecting_rect.m_rect.max[1];
          new_rect->m_state = maybe_intersecting_rect.State();
          m_rects.push_back(new_rect);
        }

        // Remove the original overlapping rect and release its memory to the pool
        delete &maybe_intersecting_rect;
        next_iter_rect = m_rects.erase(iter_rect);
      }

      const auto new_rect = new TextureSubresourceState{};
      *new_rect = rect_spec;
      m_rects.push_back(new_rect);
      TryMergeWithNeighbours(*new_rect);
    }

  private:
    static constexpr size_t MaxSimultaneousRects(u32 array_layer_count, u32 mip_count) {
      // Allocate enough rectangles to fill the entire grid.
      // Also allocate one extra for safety, not sure if needed!?
      return array_layer_count * mip_count + 1u;
    }

    void TryMergeWithNeighbours(TextureSubresourceState& rect) {
      std::vector<TextureSubresourceState*>::iterator next_iter_rect;

      for(auto iter_rect = m_rects.begin(); iter_rect != m_rects.end(); iter_rect = next_iter_rect) {
        TextureSubresourceState& maybe_neighbour_rect = **iter_rect;

        next_iter_rect = std::next(iter_rect);

        if(maybe_neighbour_rect.m_state != rect.m_state || &maybe_neighbour_rect == &rect) {
          continue;
        }

        for(int axis = 0; axis < 2; axis++) {
          const int other_axis = axis ^ 1;

          if(maybe_neighbour_rect.m_rect.min[other_axis] != rect.m_rect.min[other_axis]) {
            continue;
          }

          if(maybe_neighbour_rect.m_rect.max[other_axis] != rect.m_rect.max[other_axis]) {
            continue;
          }

          if(maybe_neighbour_rect.m_rect.max[axis] + 1u == rect.m_rect.min[axis]) {
            rect.m_rect.min[axis] = maybe_neighbour_rect.m_rect.min[axis];
            delete &maybe_neighbour_rect;
            m_rects.erase(iter_rect);
            next_iter_rect = m_rects.begin();
            break;
          }

          if(maybe_neighbour_rect.m_rect.min[axis] == rect.m_rect.max[axis] + 1u) {
            rect.m_rect.max[axis] = maybe_neighbour_rect.m_rect.max[axis];
            delete &maybe_neighbour_rect;
            m_rects.erase(iter_rect);
            next_iter_rect = m_rects.begin();
            break;
          }
        }
      }
    }

    std::vector<TextureSubresourceState*> m_rects{};
};

/**
 * For render pass subresource state merging we require:
 * - being able to know the state of rectangles we intersect and being able to control what state is written to the new rectangles based on this information
 * - we also need to know the intersecting areas
 */
using TextureStateCombiner = TextureStateTracker;
/*class TextureStateCombiner {
  public:
    TextureStateCombiner(u32 width, u32 height) {
      // This ensures we don't invalidate iterators when adding new rects to the vector.
      m_rects.reserve(MaxSimultaneousRects(width, height));
    }

    [[nodiscard]] std::span<const TextureSubresourceState* const> GetRects() const {
      return m_rects;
    }

    void TransitionRect(const TextureSubresourceState& rect_spec) {
      std::vector<TextureSubresourceState*>::iterator next_iter_rect;

      for(auto iter_rect = m_rects.begin(); iter_rect != m_rects.end(); iter_rect = next_iter_rect) {
        TextureSubresourceState& maybe_intersecting_rect = **iter_rect;

        next_iter_rect = std::next(iter_rect);

        if(!rect_spec.m_rect.Intersects(maybe_intersecting_rect.m_rect)) {
          continue;
        }

        if(rect_spec.m_rect.IsCongruentTo(maybe_intersecting_rect.m_rect)) {
          if(maybe_intersecting_rect.m_state != rect_spec.m_state) {
            maybe_intersecting_rect.m_state = rect_spec.m_state;
            TryMergeWithNeighbours(maybe_intersecting_rect);
          }
          return;
        }

        // Create up to four new rects with the same state in each direction

        if(rect_spec.m_rect.min[0] > maybe_intersecting_rect.m_rect.min[0]) {
          const auto new_rect = new TextureSubresourceState{};
          *new_rect = maybe_intersecting_rect;
          new_rect->m_rect.max[0] = rect_spec.m_rect.min[0] - 1;
          m_rects.push_back(new_rect);
        }

        if(rect_spec.m_rect.max[0] < maybe_intersecting_rect.m_rect.max[0]) {
          const auto new_rect = new TextureSubresourceState{};
          *new_rect = maybe_intersecting_rect;
          new_rect->m_rect.min[0] = rect_spec.m_rect.max[0] + 1;
          m_rects.push_back(new_rect);
        }

        if(rect_spec.m_rect.min[1] > maybe_intersecting_rect.m_rect.min[1]) {
          const auto new_rect = new TextureSubresourceState{};
          new_rect->m_rect.min[0] = rect_spec.m_rect.min[0];
          new_rect->m_rect.max[0] = rect_spec.m_rect.max[0];
          new_rect->m_rect.min[1] = maybe_intersecting_rect.m_rect.min[1];
          new_rect->m_rect.max[1] = rect_spec.m_rect.min[1] - 1;
          new_rect->m_state = maybe_intersecting_rect.m_state;
          m_rects.push_back(new_rect);
        }

        if(rect_spec.m_rect.max[1] < maybe_intersecting_rect.m_rect.max[1]) {
          const auto new_rect = new TextureSubresourceState{};
          new_rect->m_rect.min[0] = rect_spec.m_rect.min[0];
          new_rect->m_rect.max[0] = rect_spec.m_rect.max[0];
          new_rect->m_rect.min[1] = rect_spec.m_rect.max[1] + 1;
          new_rect->m_rect.max[1] = maybe_intersecting_rect.m_rect.max[1];
          new_rect->m_state = maybe_intersecting_rect.m_state;
          m_rects.push_back(new_rect);
        }

        // Remove the original overlapping rect and release its memory to the pool
        delete &maybe_intersecting_rect;
        next_iter_rect = m_rects.erase(iter_rect);
      }

      const auto new_rect = new TextureSubresourceState{};
      *new_rect = rect_spec;
      m_rects.push_back(new_rect);
      TryMergeWithNeighbours(*new_rect);
    }

  private:
    static constexpr size_t MaxSimultaneousRects(u32 width, u32 height) {
      // Allocate enough rectangles to fill the entire grid.
      // Also allocate one extra for safety, not sure if needed!?
      return width * height + 1u;
    }

    void TryMergeWithNeighbours(TextureSubresourceState& rect) {
      std::vector<TextureSubresourceState*>::iterator next_iter_rect;

      for(auto iter_rect = m_rects.begin(); iter_rect != m_rects.end(); iter_rect = next_iter_rect) {
        TextureSubresourceState& maybe_neighbour_rect = **iter_rect;

        next_iter_rect = std::next(iter_rect);

        if(maybe_neighbour_rect.m_state != rect.m_state || &maybe_neighbour_rect == &rect) {
          continue;
        }

        for(int axis = 0; axis < 2; axis++) {
          const int other_axis = axis ^ 1;

          if(maybe_neighbour_rect.m_rect.min[other_axis] != rect.m_rect.min[other_axis]) {
            continue;
          }

          if(maybe_neighbour_rect.m_rect.max[other_axis] != rect.m_rect.max[other_axis]) {
            continue;
          }

          if(maybe_neighbour_rect.m_rect.max[axis] + 1u == rect.m_rect.min[axis]) {
            rect.m_rect.min[axis] = maybe_neighbour_rect.m_rect.min[axis];
            delete &maybe_neighbour_rect;
            m_rects.erase(iter_rect);
            next_iter_rect = m_rects.begin();
            break;
          }

          if(maybe_neighbour_rect.m_rect.min[axis] == rect.m_rect.max[axis] + 1u) {
            rect.m_rect.max[axis] = maybe_neighbour_rect.m_rect.max[axis];
            delete &maybe_neighbour_rect;
            m_rects.erase(iter_rect);
            next_iter_rect = m_rects.begin();
            break;
          }
        }
      }
    }

    std::vector<TextureSubresourceState*> m_rects{};
};*/

} // namespace mgpu::vulkan
