#pragma once

#include "compressed_pair.h"
#include <utility>
#include <cstddef>  // std::nullptr_t

template <class T>
struct Slug {
    void operator()(T* ptr) {
        delete ptr;
    }
};

template <class T>
struct Slug<T[]> {
    void operator()(T* ptr) {
        delete[] ptr;
    }
};

template <typename T, typename Deleter = Slug<T>>
class UniquePtr : CPElement<Deleter, 0> {
private:
    T* ptr_;

public:
    explicit UniquePtr(T* ptr = nullptr) noexcept : CPElement<Deleter, 0>(Deleter()), ptr_(ptr) {
    }

    UniquePtr(T* ptr, Deleter deleter) noexcept
            : CPElement<Deleter, 0>(std::forward<Deleter>(deleter)), ptr_(ptr) {
    }

    UniquePtr(UniquePtr&& other) noexcept
            : CPElement<Deleter, 0>(std::move(other.GetEl())), ptr_(std::move(other.ptr_)) {
        other.ptr_ = nullptr;
    }

    template <class Father>
    UniquePtr(UniquePtr<Father>&& other) noexcept : CPElement<Deleter, 0>(Deleter()) {
        ptr_ = other.Release();
    }

    template <class Father, class FatherDeleter>
    UniquePtr(UniquePtr<Father, FatherDeleter>&& other) noexcept
            : CPElement<Deleter, 0>(FatherDeleter()) {
        ptr_ = other.Release();
    }

    UniquePtr(const UniquePtr& other) = delete;

    UniquePtr& operator=(UniquePtr&& other) noexcept {
        if (ptr_ == other.ptr_) {
            return *this;
        }
        if (ptr_) {
            CPElement<Deleter, 0>::GetEl()(ptr_);
        }
        CPElement<Deleter, 0>::GetEl() = std::forward<Deleter>(other.GetEl());
        ptr_ = other.Release();
        return *this;
    }

    UniquePtr& operator=(const UniquePtr& other) = delete;

    UniquePtr& operator=(std::nullptr_t) noexcept {
        if (ptr_) {
            CPElement<Deleter, 0>::GetEl()(ptr_);
        }
        ptr_ = nullptr;
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr() noexcept {
        if (ptr_) {
            CPElement<Deleter, 0>::GetEl()(ptr_);
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release() noexcept {
        T* temp = ptr_;
        ptr_ = nullptr;
        return temp;
    }

    void Reset(T* ptr = nullptr) noexcept {
        T* temp = ptr_;
        ptr_ = ptr;
        if (temp) {
            CPElement<Deleter, 0>::GetEl()(temp);
        }
    }

    void Swap(UniquePtr& other) noexcept {
        std::swap(ptr_, other.ptr_);
        std::swap(CPElement<Deleter, 0>::GetEl(), other.GetEl());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const noexcept {
        return ptr_;
    }

    Deleter& GetDeleter() noexcept {
        return CPElement<Deleter, 0>::GetEl();
    }

    const Deleter& GetDeleter() const noexcept {
        return CPElement<Deleter, 0>::GetEl();
    }

    explicit operator bool() const noexcept {
        return ptr_ != nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators

    std::add_lvalue_reference_t<T> operator*() const noexcept {
        return *ptr_;
    }

    T* operator->() const noexcept {
        return ptr_;
    }
};

template <class T, class Deleter>
class UniquePtr<T[], Deleter> {
private:
    T* ptr_;
    Deleter deleter_;

public:
    UniquePtr(T* ptr) noexcept : ptr_(ptr) {
    }

    UniquePtr(T* ptr, Deleter deleter) noexcept
            : ptr_(ptr), deleter_(std::forward<Deleter>(deleter)) {
    }

    UniquePtr(UniquePtr&& other) noexcept
            : ptr_(std::move(other.ptr_)),
              deleter_(std::forward<decltype(other.deleter_)>(other.deleter_)) {
        other.ptr_ = nullptr;
    }

    UniquePtr(const UniquePtr& other) = delete;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    UniquePtr& operator=(UniquePtr&& other) noexcept {
        if (ptr_ == other.ptr_) {
            return *this;
        }
        if (ptr_) {
            deleter_(ptr_);
        }
        deleter_ = std::forward<decltype(other.deleter_)>(other.deleter_);
        ptr_ = other.Release();
        return *this;
    }

    UniquePtr& operator=(const UniquePtr& other) = delete;

    UniquePtr& operator=(std::nullptr_t) noexcept {
        if (ptr_) {
            deleter_(ptr_);
        }
        ptr_ = nullptr;
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr() noexcept {
        if (ptr_) {
            deleter_(ptr_);
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release() noexcept {
        T* temp = ptr_;
        ptr_ = nullptr;
        return temp;
    }

    void Reset(T* ptr = nullptr) noexcept {
        T* temp = ptr_;
        ptr_ = ptr;
        if (temp) {
            deleter_(temp);
        }
    }

    void Swap(UniquePtr& other) noexcept {
        std::swap(ptr_, other.ptr_);
        std::swap(deleter_, other.deleter_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const noexcept {
        return ptr_;
    }

    Deleter& GetDeleter() noexcept {
        return deleter_;
    }

    const Deleter& GetDeleter() const noexcept {
        return deleter_;
    }

    explicit operator bool() const noexcept {
        return ptr_ != nullptr;
    }

    T& operator[](size_t i) {
        return ptr_[i];
    }

    const T& operator[](size_t i) const {
        return ptr_[i];
    }
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators
};
