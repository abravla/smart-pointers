#pragma once

#include "sw_fwd.h"  // Forward declaration

#include <cstddef>  // std::nullptr_t
#include <iostream>

struct StoredObject {
public:
    virtual size_t& GetCount() = 0;
    virtual ~StoredObject() = default;
};

template <class U, class V>
struct DefaultObj : public StoredObject {
    U* ptr_;
    size_t count = 0;

public:
    DefaultObj() = default;

    DefaultObj(U* ptr) : ptr_(ptr) {
    }

    size_t& GetCount() override {
        return count;
    }

    ~DefaultObj() {
        if (ptr_) {
            delete static_cast<typename std::remove_reference<V>::type*>(ptr_);
        }
    }
};

template <class T>
struct MakeSharedObj : public StoredObject {
    size_t count = 0;
    T* ptr_;
    T obj;

public:
    MakeSharedObj(T&& el, T*& ptr) : obj(std::move(el)) {
        ptr_ = &obj;
        ptr = ptr_;
    }

    size_t& GetCount() override {
        return count;
    }
};

template <class T>
class SharedPtr {
private:
    template <class U>
    friend class SharedPtr;

    template <typename U, typename... Args>
    friend SharedPtr<U> MakeShared(Args&&... args);

    T* ptr_ = nullptr;
    StoredObject* data_;

public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    SharedPtr() noexcept : data_(nullptr) {
    }

    SharedPtr(std::nullptr_t) noexcept : data_(nullptr) {
    }

    template <class V>
    explicit SharedPtr(V* ptr) noexcept : ptr_(ptr), data_(new DefaultObj<T, V>(ptr)) {
        ++data_->GetCount();
    }

    SharedPtr(const SharedPtr& other) noexcept {
        if (other.data_ == nullptr) {
            data_ = nullptr;
            return;
        }
        ptr_ = other.ptr_;
        data_ = other.data_;
        ++data_->GetCount();
    }

    SharedPtr(SharedPtr&& other) noexcept {
        if (other.data_ == nullptr) {
            data_ = nullptr;
            ptr_ = nullptr;
            other.ptr_ = nullptr;
            return;
        }
        ptr_ = std::move(other.ptr_);
        data_ = std::move(other.data_);
        other.data_ = nullptr;
        other.ptr_ = nullptr;
    }

    template <class F>
    SharedPtr(const SharedPtr<F>& other) noexcept {
        if (other.data_ == nullptr) {
            data_ = nullptr;
            return;
        }
        ptr_ = other.ptr_;
        data_ = other.data_;
        ++data_->GetCount();
    }

    template <class F>
    SharedPtr(SharedPtr<F>&& other) noexcept {
        if (other.data_ == nullptr) {
            data_ = nullptr;
            other.data_ = nullptr;
            other.ptr_ = nullptr;
            return;
        }
        ptr_ = std::move(other.ptr_);
        data_ = std::move(other.data_);
        other.ptr_ = nullptr;
        other.data_ = nullptr;
    }

    // Aliasing constructor
    // #8 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    template <typename Y>
    SharedPtr(const SharedPtr<Y>& other, T* ptr) noexcept : ptr_(ptr), data_(other.data_) {
        ptr_ = ptr;
        ++data_->GetCount();
    }

    // Promote `WeakPtr`
    // #11 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    SharedPtr& operator=(const SharedPtr& other) noexcept {
        if (this == &other) {
            return *this;
        }
        if (data_ == nullptr) {
            ptr_ = other.ptr_;
            data_ = other.data_;
            if (data_ != nullptr) {
                ++data_->GetCount();
            }
            return *this;
        }
        --data_->GetCount();
        if (data_->GetCount() == 0) {
            delete data_;
        }
        data_ = other.data_;
        ptr_ = other.ptr_;
        if (data_ != nullptr) {
            ++data_->GetCount();
        }
        return *this;
    }

    SharedPtr& operator=(SharedPtr&& other) noexcept {
        if (this == &other) {
            return *this;
        }
        if (data_ == nullptr) {
            data_ = std::move(other.data_);
            ptr_ = std::move(other.ptr_);
            other.data_ = nullptr;
            other.ptr_ = nullptr;
            if (data_ != nullptr) {
                ++data_->GetCount();
            }
            return *this;
        }
        --data_->GetCount();
        if (data_->GetCount() == 0) {
            delete data_;
        }
        ptr_ = std::move(other.ptr_);
        data_ = std::move(other.data_);
        other.data_ = nullptr;
        other.ptr_ = nullptr;
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~SharedPtr() noexcept {
        if (data_ == nullptr) {
            return;
        }
        --data_->GetCount();
        if (data_->GetCount() == 0) {
            delete data_;
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() {
        if (data_ == nullptr) {
            return;
        }
        --data_->GetCount();
        if (data_->GetCount() == 0) {
            delete data_;
        }
        data_ = nullptr;
        ptr_ = nullptr;
    }

    template<class V>
    void Reset(V* ptr) {
        if (data_ == nullptr) {
            data_ = new DefaultObj<T, V>(ptr);
            ptr_ = ptr;
            data_->GetCount() = 1;
            return;
        }
        --data_->GetCount();
        if (data_->GetCount() == 0 && ptr_ != ptr) {
            delete data_;
        }
        data_ = new DefaultObj<T, V>(ptr);
        ptr_ = ptr;
        data_->GetCount() = 1;
    }

    void Swap(SharedPtr& other) {
        std::swap(data_, other.data_);
        std::swap(ptr_, other.ptr_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        if (data_ == nullptr) {
            return nullptr;
        }
        return ptr_;
    }
    T& operator*() const {
        return *ptr_;
    }
    T* operator->() const {
        return ptr_;
    }
    size_t UseCount() const {
        return (data_ ? data_->GetCount() : 0);
    }
    explicit operator bool() const {
        return data_ && ptr_;
    }
};

template <typename T, typename U>
inline bool operator==(const SharedPtr<T>& left, const SharedPtr<U>& right) {
    return left.Get() == right.Get();
}

// Allocate memory only once
template <typename T, typename... Args>
SharedPtr<T> MakeShared(Args&&... args) {
    SharedPtr<T> ptr;
    ptr.data_ = new MakeSharedObj<T>(std::move(T(std::forward<Args>(args)...)), ptr.ptr_);
    ++ptr.data_->GetCount();
    return ptr;
}
