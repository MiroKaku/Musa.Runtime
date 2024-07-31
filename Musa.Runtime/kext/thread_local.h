#pragma once
#include <type_traits>


extern "C++" [[noreturn]] void __CLRCALL_PURE_OR_CDECL _Xruntime_error(_In_z_ const char*);

template<typename T>
class thread_local_t final
{
    unsigned long _tls_index = TLS_OUT_OF_INDEXES;

#ifdef _DEBUG
    mutable T* _tls_value = nullptr;
#endif

public:
    template<typename ...P>
    /*explicit*/ thread_local_t(P... args)
    {
        _tls_index = TlsAlloc();
        if (_tls_index == TLS_OUT_OF_INDEXES) {
            _Xruntime_error("thread_local call TlsAlloc() failed.");
        }

        const auto data = new T{std::forward<P>(args) ...};
        if (!TlsSetValue(_tls_index, data)) {
            _Xruntime_error("thread_local call TlsSetValue() failed.");
        }

    #ifdef _DEBUG
        _tls_value = data;
    #endif
    }

    ~thread_local_t() noexcept
    {
        if (_tls_index != TLS_OUT_OF_INDEXES) {
            if (const auto data = static_cast<T*>(TlsGetValue(_tls_index))) {
                delete data;
            }
            TlsFree(_tls_index);
        }

        _tls_index = TLS_OUT_OF_INDEXES;
    #ifdef _DEBUG
        _tls_value = nullptr;
    #endif
    }

    thread_local_t(const thread_local_t&) = delete;
    thread_local_t(thread_local_t&&) = delete;
    thread_local_t& operator=(const thread_local_t&) = delete;
    thread_local_t& operator=(thread_local_t&&) = delete;

    thread_local_t& operator=(T&& value)
    {
        *get() = std::move(value);
        return *this;
    }

    thread_local_t& operator=(const T& value) const
    {
        *get() = value;
        return *const_cast<thread_local_t*>(this);
    }

    T* operator->() const noexcept
    {
        return get();
    }

    T& operator*() const noexcept
    {
        return *get();
    }

    /*explicit*/ operator T& () const noexcept
    {
        return *get();
    }

private:
    [[nodiscard]] T* get() const noexcept
    {
    #ifdef _DEBUG
        return _tls_value = static_cast<T*>(TlsGetValue(_tls_index));
    #else
        return static_cast<T*>(TlsGetValue(_tls_index));
    #endif
    }
};

#define thread_local thread_local_t
