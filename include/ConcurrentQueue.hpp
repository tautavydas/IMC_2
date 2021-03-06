#pragma once

#include <cmath>
#include <array>
#include <mutex>
#include <optional>

template<typename T, uint64_t SIZE = 4096>
class ConcurrentQueue final {
 private:
  static constexpr unsigned Log2(unsigned n, unsigned p = 0) {
      return (n <= 1) ? p : Log2(n / 2, p + 1);
  }

  static constexpr uint64_t closestExponentOf2() {
      return (1UL << (static_cast<uint64_t>(Log2(SIZE - 1)) + 1));
  }

  static constexpr uint64_t mSize{closestExponentOf2()};
  static constexpr uint64_t mRingModMask{mSize - 1};

  std::array<std::optional<T>, mSize> mMem;
  std::mutex mLock;
  uint64_t mReadPtr{0};
  uint64_t mWritePtr{0};

 public:
  void pop() {
    std::lock_guard<std::mutex> lock(mLock);
    if (mMem[mReadPtr & mRingModMask].has_value()) {
      (*mMem[mReadPtr++ & mRingModMask].value())();
    } else {
      throw std::runtime_error("Queue element is empty");
    }
  }

  [[nodiscard]] bool peek() noexcept {
    std::lock_guard<std::mutex> lock(mLock);
    return mWritePtr != mReadPtr;
  }

  [[nodiscard]] bool isFull() noexcept {
    std::lock_guard<std::mutex> lock(mLock);
    return getCount() == mSize;
  }

  void push(T&& pItem) {
    std::lock_guard<std::mutex> lock(mLock);
    if (getCount() != mSize) {
       mMem[mWritePtr++ & mRingModMask] = std::move(pItem);
    } else {
      throw std::runtime_error("Concurrent queue full cannot write to it!");
    }
  }

 private:
  [[nodiscard]] uint64_t getCount() const noexcept{
    return mWritePtr > mReadPtr ? mWritePtr - mReadPtr : mReadPtr - mWritePtr;
  }
}; 
