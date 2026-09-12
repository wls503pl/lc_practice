#pragma once

#include <cstddef>
#include <new>
#include <utility>
#include <stdexcept>

template<typename T>
class myVector {
public:
    myVector() : data_(nullptr), size_(0), capacity_(0) {}

    ~myVector() {
        for (size_t i = 0; i < size_; ++i) {
            data_[i].~T();
        }
        ::operator delete(data_);
        
        data_ = nullptr;
        size_ = 0;
        capacity_ = 0;
    }

    myVector(const myVector& other) {
        data_ = static_cast<T*>(::operator new (other.size_ * sizeof(T)));
        capacity_ = other.size_;
        size_ = 0;
        for (size_t i = 0; i < other.size_; ++i) {
            new (&data_[i]) T(other.data_[i]);
            ++size_;
        }
    }

    myVector& operator=(const myVector& other) {
        if (this == &other) {
            return *this;
        }

        for (size_t i = 0; i < size_; ++i) {
            data_[i].~T();
        }
        ::operator delete(data_);

        data_ = static_cast<T*>(::operator new(other.size_ * sizeof(T)));
        capacity_ = other.size_;
        size_ = 0;
        for (size_t i = 0; i < other.size_; ++i) {
            new (&data_[i]) T(other.data_[i]);
            ++size_;
        }

        return *this;
    }

    myVector(myVector&& other) {
        data_ = other.data_;
        size_ = other.size_;
        capacity_ = other.capacity_;

        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
    }

    void push_back(const T& value) {
        if (size_ == capacity_) {
            grow();
        }

        new (&data_[size_]) T(value);
        ++size_;
    }

    void pop_back() {
        --size_;
        data_[size_].~T();
    }

    T& operator[](size_t idx) {
        return data_[idx];
    }

    T& at(size_t idx) {
        if (idx >= size_) {
            throw std::out_of_range("idx out of range!");
        }
        return data_[idx];
    }

    size_t size() const { return size_; }
    size_t capacity() const { return capacity_; }
    bool empty() const { return size_ == 0; }

private:
    T* data_;
    size_t size_;
    size_t capacity_;

    void grow() {
        auto new_capacity_ = (capacity_ == 0) ? 1 : (2 * capacity_);
        auto new_data_ = static_cast<T*>(::operator new(new_capacity_ * sizeof(T)));

        for (size_t i = 0; i < size_; ++i) {
            new(&new_data_[i]) T(std::move(data_[i]));
        }

        for (size_t i = 0; i < size_; ++i) {
            data_[i].~T();
        }
        ::operator delete(data_);

        data_ = new_data_;
        capacity_ = new_capacity_;
    }
};