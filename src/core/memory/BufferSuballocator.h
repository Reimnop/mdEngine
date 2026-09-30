#pragma once

#include <functional>
#include <vector>

#include "Allocation.h"

namespace mdEngine
{
  using GrowBufferCallback = std::function<void(size_t newCapacity)>;

  class BufferSuballocator
  {
  public:
    explicit BufferSuballocator(size_t capacity = 1024);

    [[nodiscard]] size_t getCapacity() const { return capacity; }

    void grow(size_t capacity);
    Allocation allocate(size_t size, const GrowBufferCallback& growCallback = nullptr);
    bool tryAllocate(size_t size, Allocation& alloc);
    void free(Allocation alloc);

  private:
    size_t capacity;

    std::vector<Allocation> freeRanges;

    static bool offsetComparer(const Allocation& a, const Allocation& b);

    void mergeAdjacent(size_t idx);
  };
} // mdEngine
