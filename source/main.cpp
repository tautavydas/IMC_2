#include <iostream>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>

#include <ConcurrentQueue.hpp>

int main(int, char**) {
  using Functor = std::function<void()>;

  ConcurrentQueue<std::shared_ptr<Functor>> queue;

  std::mutex access;
  std::condition_variable condition;

  using namespace std::literals;
  std::chrono::seconds const duration{1s};

  std::thread consumer([ &queue, &access, &condition, &duration ] {
    std::chrono::time_point<std::chrono::steady_clock> const finish{std::chrono::steady_clock::now() + duration};
    while (std::chrono::steady_clock::now() < finish) {
      std::unique_lock<std::mutex> locker(access);
      condition.wait_until(locker, finish, [&queue] () -> bool { return queue.peek(); });
      (*queue.pop())();
      locker.unlock();
      condition.notify_one();
    }
  });

  std::thread producer([ &queue, &access, &condition, &duration ] {
    uint64_t counter{0};
    std::chrono::time_point<std::chrono::steady_clock> const finish{std::chrono::steady_clock::now() + duration};
    while (std::chrono::steady_clock::now() < finish) {
      std::unique_lock<std::mutex> locker(access);
      condition.wait_until(locker, finish, [&queue] () -> bool { return !queue.isFull(); });
      queue.push(std::make_shared<Functor>([&counter] {
        std::cout << "Running task " << counter++ << std::endl;
      }));
      locker.unlock();
      condition.notify_one();
    }
  });

  consumer.join();
  producer.join();

  return EXIT_SUCCESS;
}
