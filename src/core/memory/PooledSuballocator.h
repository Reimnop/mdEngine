#pragma once

#include <stack>
#include <unordered_map>

#include "Allocation.h"
#include "BufferSuballocator.h"

namespace mdEngine
{
  class PooledSuballocator
  {
  public:
    explicit PooledSuballocator(size_t capacity = 1024);

    [[nodiscard]] size_t getCapacity() const { return backingAllocator.getCapacity(); }

    void grow(size_t newCapacity);
    Allocation allocate(size_t size, const GrowBufferCallback& growCallback = nullptr);
    bool tryAllocate(size_t size, Allocation& allocation);
    void free(Allocation alloc);
  private:
    BufferSuballocator backingAllocator;
    std::unordered_map<size_t, std::stack<Allocation>> pooledAllocationsBySizeClasses;

    static size_t getSizeClass(size_t size);
  };
} // mdEngine
