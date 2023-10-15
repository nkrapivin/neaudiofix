#pragma once


#define WIN32_LEAN_AND_MEAN             // Исключите редко используемые компоненты из заголовков Windows
#define NOMINMAX 1
#define INITGUID 1
// Файлы заголовков Windows
#include <windows.h>

template<class T>
class NeComPtr {
private:
    using TPtr = T*;

    TPtr ptr_;

    void addref_(void) { ptr_->AddRef(); }
    void release_(void) { ptr_->Release(); }

public:

    void release(void) noexcept {
        if (ptr_) {
            release_();
            ptr_ = nullptr;
        }
    }

    NeComPtr(void) noexcept {
        ptr_ = nullptr;
    }

    NeComPtr(std::nullptr_t) noexcept {
        ptr_ = nullptr;
    }

    NeComPtr(const NeComPtr& other) noexcept {
        ptr_ = other.ptr_;
        if (ptr_) {
            addref_();
        }
    }

    NeComPtr(NeComPtr&& other) noexcept {
        ptr_ = other.ptr_;
        other.ptr_ = nullptr;
    }

    ~NeComPtr(void) {
        release();
    }

    TPtr& operator->(void) noexcept {
        return ptr_;
    }

    const TPtr& operator->(void) const noexcept {
        return ptr_;
    }

    NeComPtr& operator=(const NeComPtr& other) noexcept {
        if (this == &other) {
            return *this;
        }

        release();
        ptr_ = other.ptr_;
        if (ptr_) {
            addref_();
        }
        return *this;
    }

    NeComPtr& operator=(NeComPtr&& other) noexcept {
        if (this == &other) {
            return *this;
        }

        release();
        ptr_ = other.ptr_;
        other.ptr_ = nullptr;
        return *this;
    }

    operator bool() const noexcept {
        return ptr_ != nullptr;
    }

    operator TPtr() const noexcept {
        return ptr_;
    }

    bool operator!() const noexcept {
        return ptr_ == nullptr;
    }

    bool operator==(const NeComPtr& other) const noexcept {
        return ptr_ == other.ptr_;
    }

    bool operator!=(const NeComPtr& other) const noexcept {
        return ptr_ != other.ptr_;
    }

    LPVOID* put(void) noexcept {
        release();
        return reinterpret_cast<LPVOID*>(&ptr_);
    }

    TPtr* put_typed(void) noexcept {
        release();
        return &ptr_;
    }

    TPtr get(void) noexcept {
        return ptr_;
    }
};

