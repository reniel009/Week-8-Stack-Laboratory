// Stack.h — Week 8 CC105 Stack ADT (dynamic array implementation)
// Owns its memory; safe on empty pop/peek; copy disabled to prevent shallow copies.
#pragma once
#include <cstddef>
#include <utility>

template <typename T>
class Stack {
public:
    Stack() : data_(new T[INIT_CAP]), cap_(INIT_CAP), size_(0) {}

    ~Stack() { delete[] data_; }

    // Prevent unsafe shallow copying (spec step 5)
    Stack(const Stack&) = delete;
    Stack& operator=(const Stack&) = delete;

    // Optional: allow move
    Stack(Stack&& other) noexcept { swap(other); }
    Stack& operator=(Stack&& other) noexcept {
        // Copy is disabled because a shallow copy would duplicate the
        // data_ pointer, causing a double-delete[] when both objects
        // are destroyed. A deep copy would also work but is unnecessary
        // for this application.
        if (this != &other) { swap(other); }
        return *this;
    }

    void push(const T& value) {
        if (size_ == cap_) grow();
        data_[size_++] = value;
    }

    bool pop(T& value) {
        if (isEmpty()) return false;
        value = data_[--size_];
        return true;
    }

    bool peek(T& value) const {
        if (isEmpty()) return false;
        value = data_[size_ - 1];
        return true;
    }

    bool isEmpty() const { return size_ == 0; }
    std::size_t size() const { return size_; }

private:
    static constexpr std::size_t INIT_CAP = 2;

    void grow() {
        // Double the capacity, copy elements to the new block, then
        // free the old block with delete[]. Doubling keeps push()
        // amortized O(1) because the total copy work across all
        // pushes stays linear in the number of elements.
        std::size_t newCap = cap_ * 2;
        T* newData = new T[newCap];
        for (std::size_t i = 0; i < size_; ++i) newData[i] = data_[i];
        delete[] data_;     // release old storage to avoid a leak
        data_ = newData;
        cap_ = newCap;
    }

    void swap(Stack& o) noexcept {
        std::swap(data_, o.data_);
        std::swap(cap_, o.cap_);
        std::swap(size_, o.size_);
    }

    T* data_;
    std::size_t cap_;
    std::size_t size_;
};