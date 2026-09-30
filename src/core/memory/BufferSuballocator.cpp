#include "BufferSuballocator.h"

#include <algorithm>
#include <stdexcept>
#include <string>

namespace mdEngine
{
  BufferSuballocator::BufferSuballocator(size_t capacity) : capacity(capacity)
  {
    freeRanges.push_back(Allocation{0, capacity});
  }

  bool BufferSuballocator::offsetComparer(const Allocation& a, const Allocation& b)
  {
    return a.offset < b.offset;
  }

  void BufferSuballocator::grow(size_t capacity)
  {
    if (capacity <= this->capacity)
      throw std::logic_error("cannot grow buffer allocator from size " + std::to_string(this->capacity) + " to size " + std::to_string(capacity));

    if (freeRanges.empty())
    {
      // buffer was completely full, new space is the only free range
      freeRanges.push_back(Allocation{this->capacity, capacity - this->capacity});
      this->capacity = capacity;
      return;
    }

    auto lastFree = freeRanges.back();
    auto lastFreeEnd = lastFree.offset + lastFree.size;

    // grow the last free range
    if (lastFreeEnd == this->capacity)
    {
      auto newLastFreeSize = capacity - this->capacity + lastFree.size;
      lastFree.size = newLastFreeSize;
      freeRanges.back() = lastFree;
    }
    // append a new free range
    else
    {
      auto newLastFreeSize = capacity - this->capacity;
      lastFree = Allocation{this->capacity, newLastFreeSize};
      freeRanges.push_back(lastFree);
    }

    // set new capacity
    this->capacity = capacity;
  }

  Allocation BufferSuballocator::allocate(size_t size, const GrowBufferCallback& growCallback)
  {
    Allocation alloc;
    auto grown = false;
    while (!tryAllocate(size, alloc))
    {
      grow(capacity * 2);
      grown = true;
    }

    if (grown && growCallback)
      growCallback(capacity);

    return alloc;
  }

  bool BufferSuballocator::tryAllocate(size_t size, Allocation& alloc)
  {
    // find a free range with a suitable size
    for (size_t i = 0; i < freeRanges.size(); i++)
    {
      auto range = freeRanges[i];
      if (range.size >= size)
      {
        alloc = range;
        alloc.size = size;
        freeRanges.erase(freeRanges.begin() + i);

        auto usedEnd = alloc.offset + size;
        auto rangeEnd = range.offset + range.size;

        if (usedEnd < rangeEnd)
          freeRanges.insert(freeRanges.begin() + i, Allocation{usedEnd, rangeEnd - usedEnd});

        return true;
      }
    }

    // OOM
    alloc = Allocation{};
    return false;
  }

  void BufferSuballocator::free(Allocation alloc)
  {
    auto idx = std::lower_bound(freeRanges.begin(), freeRanges.end(), alloc, offsetComparer) - freeRanges.begin();
    freeRanges.insert(freeRanges.begin() + idx, alloc);
    mergeAdjacent(idx);
  }

  void BufferSuballocator::mergeAdjacent(size_t idx)
  {
    if (idx + 1 < freeRanges.size() && freeRanges[idx].offset + freeRanges[idx].size == freeRanges[idx + 1].offset)
    {
      auto merged = Allocation{
        freeRanges[idx].offset,
        freeRanges[idx].size + freeRanges[idx + 1].size};
      freeRanges[idx] = merged;
      freeRanges.erase(freeRanges.begin() + idx + 1);
    }

    if (idx > 0 && freeRanges[idx - 1].offset + freeRanges[idx - 1].size == freeRanges[idx].offset)
    {
      auto merged = Allocation{
        freeRanges[idx - 1].offset,
        freeRanges[idx - 1].size + freeRanges[idx].size};
      freeRanges[idx - 1] = merged;
      freeRanges.erase(freeRanges.begin() + idx);
    }
  }
} // mdEngine
