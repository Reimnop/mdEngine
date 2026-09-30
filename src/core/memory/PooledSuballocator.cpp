#include "PooledSuballocator.h"

#include <bit>
#include <cstdint>

namespace mdEngine
{
  PooledSuballocator::PooledSuballocator(size_t capacity)
      : backingAllocator(capacity)
  {
  }

  void PooledSuballocator::grow(size_t newCapacity)
  {
    backingAllocator.grow(newCapacity);
  }

  Allocation PooledSuballocator::allocate(size_t size, const GrowBufferCallback& growCallback)
  {
    Allocation alloc{};
    auto grown = false;
    while (!tryAllocate(size, alloc))
    {
      grow(getCapacity() * 2);
      grown = true;
    }

    if (grown && growCallback)
      growCallback(getCapacity());

    return alloc;
  }

  bool PooledSuballocator::tryAllocate(size_t size, Allocation& allocation)
  {
    auto sizeClass = getSizeClass(size);

    auto it = pooledAllocationsBySizeClasses.find(sizeClass);
    if (it != pooledAllocationsBySizeClasses.end() && !it->second.empty())
    {
      allocation = it->second.top();
      it->second.pop();
      return true;
    }

    if (backingAllocator.tryAllocate(sizeClass, allocation))
      return true;

    allocation = Allocation{};
    return false;
  }

  void PooledSuballocator::free(Allocation alloc)
  {
    // we use alloc.size directly because it's guaranteed to be a valid size class
    pooledAllocationsBySizeClasses[alloc.size].push(alloc);
  }

  // inspired by jemalloc
  size_t PooledSuballocator::getSizeClass(size_t size)
  {
    // minimum size class
    if (size <= 16)
      return 16;

    const auto exp = static_cast<size_t>(std::bit_width(static_cast<std::uint32_t>(size - 1))) - 1; // exponent of the range size falls into
    const auto pow2 = 1 << (exp + 1);                                                               // top of the range, e.g. 32, 64, 128...
    const auto step = pow2 >> 2;                                                                    // quarter-step within the range, e.g. 8, 16, 32...

    // round up to the next multiple of `step` at or above `size`
    return (size + step - 1) / step * step;
  }
} // mdEngine
