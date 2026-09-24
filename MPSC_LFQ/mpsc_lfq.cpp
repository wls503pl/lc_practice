#pragma once
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

template <typename T>
class MpscQueue {
public:
    explicit MpscQueue(size_t capacity)
        : capacity_(capacity), buffer_(capacity) {
        for (size_t i = 0; i < capacity_; ++i) {
            buffer_[i].seq.store(i, std::memory_order_relaxed);
        }
    }

    bool push(T item) {
        size_t pos = head_.load(std::memory_order_relaxed);
        for (;;) {
            Slot& slot = buffer_[pos % capacity_];
            size_t seq = slot.seq.load(std::memory_order_acquire);
            int64_t diff = (int64_t)seq - (int64_t)pos;
            if (diff == 0) {
                if (head_.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed)) {
                    break; // claimed this slot
                }
                // CAS failed, pos was updated to the current head_, retry
            } else if (diff < 0) {
                return false; // full
            } else {
                pos = head_.load(std::memory_order_relaxed); // lost the race, retry
            }
        }
        Slot& slot = buffer_[pos % capacity_];
        slot.data = std::move(item);
        slot.seq.store(pos + 1, std::memory_order_release);
        return true;
    }

    std::optional<T> pop() {
        size_t pos = tail_.load(std::memory_order_relaxed);
        Slot& slot = buffer_[pos % capacity_];
        size_t seq = slot.seq.load(std::memory_order_acquire);
        int64_t diff = (int64_t)seq - (int64_t)(pos + 1);
        if (diff != 0) {
            return std::nullopt; // empty
        }
        T item = std::move(slot.data);
        slot.seq.store(pos + capacity_, std::memory_order_release);
        tail_.store(pos + 1, std::memory_order_relaxed);
        return item;
    }

private:
    struct Slot {
        std::atomic<size_t> seq;
        T data;
    };

    size_t capacity_;
    std::vector<Slot> buffer_;

    // multiple producers CAS this; only the single consumer writes tail_
    alignas(64) std::atomic<size_t> head_{0};
    alignas(64) std::atomic<size_t> tail_{0};
};