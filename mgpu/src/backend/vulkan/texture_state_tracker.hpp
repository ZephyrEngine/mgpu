
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

struct U32Rect2D {
  u32 min[2];
  u32 max[2];

  [[nodiscard]] bool Intersects(const U32Rect2D& other_rect) const {
    // Use >= instead of > because 'max' refers to the highest included cell and thus lies within the geometric rectangle.
    const bool result = max[0] >= other_rect.min[0] && other_rect.max[0] >= min[0] &&
                        max[1] >= other_rect.min[1] && other_rect.max[1] >= min[1];
    return result;
  }

  [[nodiscard]] std::optional<U32Rect2D> GetIntersectionWith(const U32Rect2D& other_rect) const {
    if(Intersects(other_rect)) {
      // TODO: refactor the texture state out of this.
      return U32Rect2D{
        .min = { std::max(min[0], other_rect.min[0]), std::max(min[1], other_rect.min[1]) },
        .max = { std::min(max[0], other_rect.max[0]), std::min(max[1], other_rect.max[1]) },
      };
    }
    return std::nullopt;
  }

  [[nodiscard]] bool IsCongruentTo(const U32Rect2D& other_rect) const {
    return min[0] == other_rect.min[0] &&
           min[1] == other_rect.min[1] &&
           max[0] == other_rect.max[0] &&
           max[1] == other_rect.max[1];
  }
};

struct TextureSubresourceState {
  public:
    TextureSubresourceState(U32Rect2D rect, TextureState state)
        : m_rect{rect}
        , m_state{state} {
    }

    TextureSubresourceState(u32 base_array_layer, u32 array_layer_count, u32 base_mip, u32 mip_count, TextureState state)
        : m_rect{
          U32Rect2D{
            .min = {base_array_layer, base_mip},
            .max = {base_array_layer + array_layer_count - 1u, base_mip + mip_count - 1u}
          }
        }
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

    [[nodiscard]] const U32Rect2D& Rect() const {
      return m_rect;
    }

    [[nodiscard]] U32Rect2D& Rect() {
      return m_rect;
    }

    [[nodiscard]] const TextureState& GetState() const {
      return m_state;
    }

    void SetState(const TextureState& state) {
      m_state = state;
    }

  private:
    U32Rect2D m_rect{};
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
      m_subresource_states.reserve(MaxSimultaneousSubresourceStates(array_layer_count, mip_count));

      // Add an initial rect that covers the entire grid.
      // TODO: this isn't desirable for the TextureStateCombiner
      const auto rect = new TextureSubresourceState{0u, array_layer_count, 0u, mip_count, {
        .m_image_layout = VK_IMAGE_LAYOUT_UNDEFINED,
        .m_access = 0,
        .m_pipeline_stages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT
      }};
      m_subresource_states.push_back(rect);
    }

    [[nodiscard]] std::span<const TextureSubresourceState* const> GetSubresourceStates() const {
      return m_subresource_states;
    }

    void TransitionSubresource(
      const TextureSubresourceState& transition,
      std::function<void(TextureSubresourceState)> on_intersection = [](TextureSubresourceState) {}
    ) {
      std::vector<TextureSubresourceState*>::iterator next_iter;

      for(auto iter = m_subresource_states.begin(); iter != m_subresource_states.end(); iter = next_iter) {
        next_iter = std::next(iter);

        TextureSubresourceState& subresource_state = **iter;
        std::optional<U32Rect2D> intersection_rect = transition.Rect().GetIntersectionWith(subresource_state.Rect());
        if(!intersection_rect.has_value()) {
          continue;
        }
        on_intersection(TextureSubresourceState{intersection_rect.value(), subresource_state.GetState()});

        if(transition.Rect().IsCongruentTo(subresource_state.Rect())) {
          if(subresource_state.GetState() != transition.GetState()) {
            subresource_state.SetState(transition.GetState());
            TryMergeWithNeighbours(subresource_state);
          }
          return;
        }

        // Create up to four new rects with the same state in each direction

        const U32Rect2D& subresource_rect = subresource_state.Rect();
        const U32Rect2D& transition_rect = transition.Rect();

        if(transition_rect.min[0] > subresource_rect.min[0]) {
          const auto new_subresource_state = new TextureSubresourceState{
            U32Rect2D{
              .min = {subresource_rect.min[0], subresource_rect.min[1] },
              .max = { transition_rect.min[0] - 1u, subresource_rect.max[1]}
            },
            subresource_state.GetState()
          };
          m_subresource_states.push_back(new_subresource_state);
        }

        if(transition_rect.max[0] < subresource_rect.max[0]) {
          const auto new_subresource_state = new TextureSubresourceState{
            U32Rect2D{
              .min = { transition_rect.max[0] + 1, subresource_rect.min[1] },
              .max = {subresource_rect.max[0], subresource_rect.max[1] }
            },
            subresource_state.GetState()
          };
          m_subresource_states.push_back(new_subresource_state);
        }

        if(transition_rect.min[1] > subresource_rect.min[1]) {
          const auto new_subresource_state = new TextureSubresourceState{
            U32Rect2D{
              .min = {transition_rect.min[0], subresource_rect.min[1] },
              .max = { transition_rect.max[0], transition_rect.min[1] - 1u }
            },
            subresource_state.GetState()
          };
          m_subresource_states.push_back(new_subresource_state);
        }

        if(transition_rect.max[1] < subresource_rect.max[1]) {
          const auto new_subresource_state = new TextureSubresourceState{
            U32Rect2D{
              .min = { transition_rect.min[0], transition_rect.max[1] + 1u },
              .max = {transition_rect.max[0], subresource_rect.max[1] }
            },
            subresource_state.GetState()
          };
          m_subresource_states.push_back(new_subresource_state);
        }

        // Remove the original overlapping rect and release its memory to the pool
        delete &subresource_state;
        next_iter = m_subresource_states.erase(iter);
      }

      const auto new_subresource_state = new TextureSubresourceState{transition};
      m_subresource_states.push_back(new_subresource_state);
      TryMergeWithNeighbours(*new_subresource_state);
    }

  private:
    static constexpr size_t MaxSimultaneousSubresourceStates(u32 array_layer_count, u32 mip_count) {
      // Allocate enough rectangles to fill the entire grid.
      // Also allocate one extra for safety, not sure if needed!?
      return array_layer_count * mip_count + 1u;
    }

    void TryMergeWithNeighbours(TextureSubresourceState& subresource_state) {
      std::vector<TextureSubresourceState*>::iterator next_iter;

      for(auto iter = m_subresource_states.begin(); iter != m_subresource_states.end(); iter = next_iter) {
        TextureSubresourceState& other_subresource_state = **iter;

        next_iter = std::next(iter);

        if(&other_subresource_state == &subresource_state) {
          continue;
        }

        if(other_subresource_state.GetState() != subresource_state.GetState()) {
          continue;
        }

        for(int axis = 0; axis < 2; axis++) {
          const int other_axis = axis ^ 1;

          if(other_subresource_state.Rect().min[other_axis] != subresource_state.Rect().min[other_axis]) {
            continue;
          }

          if(other_subresource_state.Rect().max[other_axis] != subresource_state.Rect().max[other_axis]) {
            continue;
          }

          if(other_subresource_state.Rect().max[axis] + 1u == subresource_state.Rect().min[axis]) {
            subresource_state.Rect().min[axis] = other_subresource_state.Rect().min[axis];
            delete &other_subresource_state;
            m_subresource_states.erase(iter);
            next_iter = m_subresource_states.begin();
            break;
          }

          if(other_subresource_state.Rect().min[axis] == subresource_state.Rect().max[axis] + 1u) {
            subresource_state.Rect().max[axis] = other_subresource_state.Rect().max[axis];
            delete &other_subresource_state;
            m_subresource_states.erase(iter);
            next_iter = m_subresource_states.begin();
            break;
          }
        }
      }
    }

    std::vector<TextureSubresourceState*> m_subresource_states{};
};

/**
 * For render pass subresource state merging we require:
 * - being able to know the state of rectangles we intersect and being able to control what state is written to the new rectangles based on this information
 * - we also need to know the intersecting areas
 */
using TextureStateCombiner = TextureStateTracker;

} // namespace mgpu::vulkan
