
#pragma once

#include <atom/integer.hpp>
#include <atom/panic.hpp>
#include <algorithm>
#include <functional>
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

struct TextureRect {
  // TODO(fleroviux): can we rename min/max to something clearer?
  u32 min[2];
  u32 max[2];
  TextureState state;

  [[nodiscard]] bool Intersects(const TextureRect& other_rect) const {
    // Use >= instead of > because 'max' refers to the highest included cell and thus lies within th geometric rectangle.
    const bool result = max[0] >= other_rect.min[0] && other_rect.max[0] >= min[0] &&
                        max[1] >= other_rect.min[1] && other_rect.max[1] >= min[1];
    return result;
  }

  [[nodiscard]] bool IsCongruentTo(const TextureRect& other_rect) const {
    return min[0] == other_rect.min[0] &&
           min[1] == other_rect.min[1] &&
           max[0] == other_rect.max[0] &&
           max[1] == other_rect.max[1];
  }
};

// TODO: make this reusable?
class TextureRectPool {
  public:
    explicit TextureRectPool(size_t capacity) {
      m_rect_pool = new TextureRect[capacity]{};
      m_rect_pool_stack.reserve(capacity);
      for(size_t i = 0; i < capacity; i++) {
        m_rect_pool_stack.push_back(&m_rect_pool[i]);
      }
    }

   ~TextureRectPool() {
      delete[] m_rect_pool;
    }

    TextureRect& NewRect() {
      if(m_rect_pool_stack.empty()) {
        ATOM_PANIC("failed to allocate rect");
      }
      TextureRect& rect = *m_rect_pool_stack.back();
      m_rect_pool_stack.pop_back();
      return rect;
    }

    void FreeRect(TextureRect& rect) {
      m_rect_pool_stack.push_back(&rect);
    }

  private:
    TextureRect* m_rect_pool;
    std::vector<TextureRect*> m_rect_pool_stack{};
};

/**
 * For tracking the texture subresource states we require:
 * - knowing all the rects we intersected so that we can generate one barrier for every "from" state
 * - we also need to know the intersecting areas
 */
class TextureStateTracker {
  public:
    TextureStateTracker(u32 width, u32 height)
        : m_rect_pool{MaxSimultaneousRects(width, height)} {
      // This ensures we don't invalidate iterators when adding new rects to the vector.
      m_rects.reserve(MaxSimultaneousRects(width, height));

      // Add an initial rect that covers the entire grid.
      // TODO: we do not want this in all use-cases
      TextureRect& rect = m_rect_pool.NewRect();
      rect.min[0] = 0;
      rect.max[0] = (i32)width - 1;
      rect.min[1] = 0;
      rect.max[1] = (i32)height - 1;
      rect.state = {
        .m_image_layout = VK_IMAGE_LAYOUT_UNDEFINED,
        .m_access = 0,
        .m_pipeline_stages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT
      };
      m_rects.push_back(&rect);
    }

    [[nodiscard]] std::span<const TextureRect* const> GetRects() const {
      return m_rects;
    }

    void TransitionRect(const TextureRect& rect_spec, std::function<void(TextureRect)> on_intersection) {
      std::vector<TextureRect*>::iterator next_iter_rect;

      for(auto iter_rect = m_rects.begin(); iter_rect != m_rects.end(); iter_rect = next_iter_rect) {
        TextureRect& maybe_intersecting_rect = **iter_rect;

        next_iter_rect = std::next(iter_rect);

        if(!rect_spec.Intersects(maybe_intersecting_rect)) {
          ++iter_rect;
          continue;
        }

        // TODO: let TextureRect::Intersects() return this as an optional?
        on_intersection({
          .min = { std::max(rect_spec.min[0], maybe_intersecting_rect.min[0]), std::max(rect_spec.min[1], maybe_intersecting_rect.min[1]) },
          .max = { std::min(rect_spec.max[0], maybe_intersecting_rect.max[0]), std::min(rect_spec.max[1], maybe_intersecting_rect.max[1]) },
          .state = maybe_intersecting_rect.state
        });

        if(rect_spec.IsCongruentTo(maybe_intersecting_rect)) {
          if(maybe_intersecting_rect.state != rect_spec.state) {
            maybe_intersecting_rect.state = rect_spec.state;
            TryMergeWithNeighbours(maybe_intersecting_rect);
          }
          return;
        }

        // Create up to four new rects with the same state in each direction

        if(rect_spec.min[0] > maybe_intersecting_rect.min[0]) {
          TextureRect& new_rect = m_rect_pool.NewRect();
          new_rect = maybe_intersecting_rect;
          new_rect.max[0] = rect_spec.min[0] - 1;
          m_rects.push_back(&new_rect);
        }

        if(rect_spec.max[0] < maybe_intersecting_rect.max[0]) {
          TextureRect& new_rect = m_rect_pool.NewRect();
          new_rect = maybe_intersecting_rect;
          new_rect.min[0] = rect_spec.max[0] + 1;
          m_rects.push_back(&new_rect);
        }

        if(rect_spec.min[1] > maybe_intersecting_rect.min[1]) {
          TextureRect& new_rect = m_rect_pool.NewRect();
          new_rect.min[0] = rect_spec.min[0];
          new_rect.max[0] = rect_spec.max[0];
          new_rect.min[1] = maybe_intersecting_rect.min[1];
          new_rect.max[1] = rect_spec.min[1] - 1;
          new_rect.state = maybe_intersecting_rect.state;
          m_rects.push_back(&new_rect);
        }

        if(rect_spec.max[1] < maybe_intersecting_rect.max[1]) {
          TextureRect& new_rect = m_rect_pool.NewRect();
          new_rect.min[0] = rect_spec.min[0];
          new_rect.max[0] = rect_spec.max[0];
          new_rect.min[1] = rect_spec.max[1] + 1;
          new_rect.max[1] = maybe_intersecting_rect.max[1];
          new_rect.state = maybe_intersecting_rect.state;
          m_rects.push_back(&new_rect);
        }

        // Remove the original overlapping rect and release its memory to the pool
        m_rect_pool.FreeRect(maybe_intersecting_rect);
        next_iter_rect = m_rects.erase(iter_rect);
      }

      TextureRect& new_rect = m_rect_pool.NewRect();
      new_rect = rect_spec;
      m_rects.push_back(&new_rect);
      TryMergeWithNeighbours(new_rect);
    }

  private:
    static constexpr size_t MaxSimultaneousRects(u32 width, u32 height) {
      // Allocate enough rectangles to fill the entire grid.
      // Also allocate one extra for safety, not sure if needed!?
      return width * height + 1u;
    }

    void TryMergeWithNeighbours(TextureRect& rect) {
      std::vector<TextureRect*>::iterator next_iter_rect;

      for(auto iter_rect = m_rects.begin(); iter_rect != m_rects.end(); iter_rect = next_iter_rect) {
        TextureRect& maybe_neighbour_rect = **iter_rect;

        next_iter_rect = std::next(iter_rect);

        if(maybe_neighbour_rect.state != rect.state || &maybe_neighbour_rect == &rect) {
          continue;
        }

        for(int axis = 0; axis < 2; axis++) {
          const int other_axis = axis ^ 1;

          if(maybe_neighbour_rect.min[other_axis] != rect.min[other_axis]) {
            continue;
          }

          if(maybe_neighbour_rect.max[other_axis] != rect.max[other_axis]) {
            continue;
          }

          if(maybe_neighbour_rect.max[axis] + 1u == rect.min[axis]) {
            rect.min[axis] = maybe_neighbour_rect.min[axis];
            m_rect_pool.FreeRect(maybe_neighbour_rect);
            m_rects.erase(iter_rect);
            next_iter_rect = m_rects.begin();
            break;
          }

          if(maybe_neighbour_rect.min[axis] == rect.max[axis] + 1u) {
            rect.max[axis] = maybe_neighbour_rect.max[axis];
            m_rect_pool.FreeRect(maybe_neighbour_rect);
            m_rects.erase(iter_rect);
            next_iter_rect = m_rects.begin();
            break;
          }
        }
      }
    }

    TextureRectPool m_rect_pool;
    std::vector<TextureRect*> m_rects{};
};

/**
 * For render pass subresource state merging we require:
 * - being able to know the state of rectangles we intersect and being able to control what state is written to the new rectangles based on this information
 * - we also need to know the intersecting areas
 */
class TextureStateCombiner {
  public:
    TextureStateCombiner(u32 width, u32 height)
        : m_rect_pool{MaxSimultaneousRects(width, height)} {
      // This ensures we don't invalidate iterators when adding new rects to the vector.
      m_rects.reserve(MaxSimultaneousRects(width, height));
    }

    [[nodiscard]] std::span<const TextureRect* const> GetRects() const {
      return m_rects;
    }

    void TransitionRect(const TextureRect& rect_spec) {
      std::vector<TextureRect*>::iterator next_iter_rect;

      for(auto iter_rect = m_rects.begin(); iter_rect != m_rects.end(); iter_rect = next_iter_rect) {
        TextureRect& maybe_intersecting_rect = **iter_rect;

        next_iter_rect = std::next(iter_rect);

        if(!rect_spec.Intersects(maybe_intersecting_rect)) {
          ++iter_rect;
          continue;
        }

        if(rect_spec.IsCongruentTo(maybe_intersecting_rect)) {
          if(maybe_intersecting_rect.state != rect_spec.state) {
            maybe_intersecting_rect.state = rect_spec.state;
            TryMergeWithNeighbours(maybe_intersecting_rect);
          }
          return;
        }

        // Create up to four new rects with the same state in each direction

        if(rect_spec.min[0] > maybe_intersecting_rect.min[0]) {
          TextureRect& new_rect = m_rect_pool.NewRect();
          new_rect = maybe_intersecting_rect;
          new_rect.max[0] = rect_spec.min[0] - 1;
          m_rects.push_back(&new_rect);
        }

        if(rect_spec.max[0] < maybe_intersecting_rect.max[0]) {
          TextureRect& new_rect = m_rect_pool.NewRect();
          new_rect = maybe_intersecting_rect;
          new_rect.min[0] = rect_spec.max[0] + 1;
          m_rects.push_back(&new_rect);
        }

        if(rect_spec.min[1] > maybe_intersecting_rect.min[1]) {
          TextureRect& new_rect = m_rect_pool.NewRect();
          new_rect.min[0] = rect_spec.min[0];
          new_rect.max[0] = rect_spec.max[0];
          new_rect.min[1] = maybe_intersecting_rect.min[1];
          new_rect.max[1] = rect_spec.min[1] - 1;
          new_rect.state = maybe_intersecting_rect.state;
          m_rects.push_back(&new_rect);
        }

        if(rect_spec.max[1] < maybe_intersecting_rect.max[1]) {
          TextureRect& new_rect = m_rect_pool.NewRect();
          new_rect.min[0] = rect_spec.min[0];
          new_rect.max[0] = rect_spec.max[0];
          new_rect.min[1] = rect_spec.max[1] + 1;
          new_rect.max[1] = maybe_intersecting_rect.max[1];
          new_rect.state = maybe_intersecting_rect.state;
          m_rects.push_back(&new_rect);
        }

        // Remove the original overlapping rect and release its memory to the pool
        m_rect_pool.FreeRect(maybe_intersecting_rect);
        next_iter_rect = m_rects.erase(iter_rect);
      }

      TextureRect& new_rect = m_rect_pool.NewRect();
      new_rect = rect_spec;
      m_rects.push_back(&new_rect);
      TryMergeWithNeighbours(new_rect);
    }

  private:
    static constexpr size_t MaxSimultaneousRects(u32 width, u32 height) {
      // Allocate enough rectangles to fill the entire grid.
      // Also allocate one extra for safety, not sure if needed!?
      return width * height + 1u;
    }

    void TryMergeWithNeighbours(TextureRect& rect) {
      std::vector<TextureRect*>::iterator next_iter_rect;

      for(auto iter_rect = m_rects.begin(); iter_rect != m_rects.end(); iter_rect = next_iter_rect) {
        TextureRect& maybe_neighbour_rect = **iter_rect;

        next_iter_rect = std::next(iter_rect);

        if(maybe_neighbour_rect.state != rect.state || &maybe_neighbour_rect == &rect) {
          continue;
        }

        for(int axis = 0; axis < 2; axis++) {
          const int other_axis = axis ^ 1;

          if(maybe_neighbour_rect.min[other_axis] != rect.min[other_axis]) {
            continue;
          }

          if(maybe_neighbour_rect.max[other_axis] != rect.max[other_axis]) {
            continue;
          }

          if(maybe_neighbour_rect.max[axis] + 1u == rect.min[axis]) {
            rect.min[axis] = maybe_neighbour_rect.min[axis];
            m_rect_pool.FreeRect(maybe_neighbour_rect);
            m_rects.erase(iter_rect);
            next_iter_rect = m_rects.begin();
            break;
          }

          if(maybe_neighbour_rect.min[axis] == rect.max[axis] + 1u) {
            rect.max[axis] = maybe_neighbour_rect.max[axis];
            m_rect_pool.FreeRect(maybe_neighbour_rect);
            m_rects.erase(iter_rect);
            next_iter_rect = m_rects.begin();
            break;
          }
        }
      }
    }

    TextureRectPool m_rect_pool;
    std::vector<TextureRect*> m_rects{};
};

} // namespace mgpu::vulkan
