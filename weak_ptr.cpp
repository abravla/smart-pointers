#pragma once

#include "sw_fwd.h"  // Forward declaration
#include "shared.h"

// https://en.cppreference.com/w/cpp/memory/weak_ptr
template <typename T>
class WeakPtr {
    friend SharedPtr<T>;

    T* ptr_;
    StoredObject* data_;

public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    WeakPtr() : ptr_(nullptr), data_(nullptr) {
    }

    WeakPtr(const WeakPtr& other) : ptr_(other.ptr_), data_(other.data_) {
        if (data_) {
            ++data_->GetWeakCount();
        }
    }

    WeakPtr(WeakPtr&& other) {
        ptr_ = other.ptr_;
        data_ = other.data_;
        other.ptr_ = nullptr;
        other.data_ = nullptr;
    }
    // Demote `SharedPtr`
    // #2 from https://en.cppreference.com/w/cpp/memory/weak_ptr/weak_ptr
    WeakPtr(const SharedPtr<T>& other) : ptr_(other.ptr_), data_(other.data_) {
        if (data_) {
            ++data_->GetWeakCount();
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    WeakPtr& operator=(const WeakPtr& other) {
        if (this == &other) {
            return *this;
        }
        if (data_) {
            --data_->GetWeakCount();
            if (data_->GetCount() + data_->GetWeakCount() == 0) {
                delete data_;
            }
        }
        ptr_ = other.ptr_;
        data_ = other.data_;
        if (data_) {
            ++data_->GetWeakCount();
        }
        return *this;
    }
    WeakPtr& operator=(WeakPtr&& other) {
        if (this == &other) {
            return *this;
        }
        if (data_) {
            --data_->GetWeakCount();
            if (data_->GetCount() + data_->GetWeakCount() == 0) {
                delete data_;
            }
        }
        ptr_ = std::move(other.ptr_);
        data_ = std::move(other.data_);
        other.ptr_ = nullptr;
        other.data_ = nullptr;
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~WeakPtr() {
        if (data_ == nullptr) {
            return;
        }
        --data_->GetWeakCount();
        if (data_->GetCount() + data_->GetWeakCount() == 0) {
            delete data_;
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() {
        if (ptr_ == nullptr) {
            return;
        }
        --data_->GetWeakCount();
        if (data_->GetCount() + data_->GetWeakCount() == 0) {
            delete data_;
        }
        ptr_ = nullptr;
        data_ = nullptr;
    }

    void Swap(WeakPtr& other) {
        std::swap(data_, other.data_);
        std::swap(ptr_, other.ptr_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    size_t UseCount() const {
        return (data_ ? data_->GetCount() : 0);
    }
    bool Expired() const {
        return UseCount() == 0;
    }
    SharedPtr<T> Lock() const {
        if ((data_ && data_->GetCount() == 0) || !data_) {
            return SharedPtr<T>();
        }
        return SharedPtr<T>(*this);
    }
};
