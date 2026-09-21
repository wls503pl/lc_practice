#pragma once
#include <atomic>
#include <optional>
#include <cstddef>
#include <vector>

using namespace std;

template<typename T>
class SpscQueue {
public:
    explicit SpscQueue(size_t capacity) : capacity_(capacity + 1), buffer_(capacity_) {}

    bool push(T item) {
        size_t head = head_.load(memory_order_relaxed);
        size_t next_head = (head + 1) % capacity_;
        if (next_head == tail_.load(memory_order_acquire)) {    // 队列已满
            return false;
        }
        buffer_[head] = std::move(item);
        head_.store(next_head, std::memory_order_release);
        return true;   
    }

    std::optional<T> pop() {
        size_t tail = tail_.load(memory_order_relaxed);
        if (tail == head_.load(memory_order_acquire)) {
            return nullopt; // 队列空的
        }
        T item = std::move(buffer_[tail]);
        tail_.store((tail + 1) % capacity_, memory_order_release);
        return item;    // 隐式移动
    }

private:
    size_t capacity_;
    vector<T> buffer_;

    /*
     * 之所以要用 std::atomic<size_t> 而不是普通 size_t，就是因为这个变量要被生产者和消费者两个线程并发读写——裸类型的读写不保证原子性，
     * 也不带 acquire/release 那套内存序语义，会有数据竞争（data race，属于未定义行为）
     */
    alignas(64) atomic<size_t> head_{0};
    alignas(64) atomic<size_t> tail_{0};
 };
