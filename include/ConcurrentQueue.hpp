#pragma once

template<typename T, uint64_t SIZE = 4096>
class ConcurrentQueue {
 private:
  static constexpr unsigned Log2(unsigned n, unsigned p = 0) {
      return (n <= 1) ? p : Log2(n / 2, p + 1);
  }

  static constexpr uint64_t closestExponentOf2() {
      return (1UL << (static_cast<uint64_t>(Log2(SIZE - 1)) + 1));
  }

  static constexpr uint64_t mSize{closestExponentOf2()};
  static constexpr uint64_t mRingModMask{mSize - 1};

  std::array<T, mSize> mMem;
  std::mutex mLock;
  uint64_t mReadPtr{0};
  uint64_t mWritePtr{0};

 public:
  const T pop() {
    std::lock_guard<std::mutex> lock(mLock);
    return mMem[mReadPtr++ & mRingModMask];
  }

  bool peek() {
    std::lock_guard<std::mutex> lock(mLock);
    return mWritePtr != mReadPtr;
  }

  bool isFull() {
    std::lock_guard<std::mutex> lock(mLock);
    return getCount() == mSize;
  }

  void push(T&& pItem) {
    std::lock_guard<std::mutex> lock(mLock);
    mMem[mWritePtr++ & mRingModMask] = std::move(pItem);
  }

 private:
  uint64_t getCount() const {
    return mWritePtr > mReadPtr ? mWritePtr - mReadPtr : mReadPtr - mWritePtr;
  }
}; 
