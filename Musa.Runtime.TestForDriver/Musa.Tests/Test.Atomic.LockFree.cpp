#ifdef TEST_HAS_ATOMIC

#include <atomic>

#include "LLVM.TestSuite/test_macros.h"
#include "LLVM.TestSuite/atomic_helpers.h"


namespace
{
    template <typename T>
    void checkAlwaysLockFree()
    {
        if (std::atomic<T>::is_always_lock_free) {
            assert(std::atomic<T>().is_lock_free());
        }
    }
}

TEST(is_always_lock_free)
{
    // structs and unions can't be defined in the template invocation.
    // Work around this with a typedef.
#define CHECK_ALWAYS_LOCK_FREE(T)                                              \
  do {                                                                         \
    typedef T type;                                                            \
    checkAlwaysLockFree<type>();                                               \
  } while (0)

    CHECK_ALWAYS_LOCK_FREE(bool);
    CHECK_ALWAYS_LOCK_FREE(char);
    CHECK_ALWAYS_LOCK_FREE(signed char);
    CHECK_ALWAYS_LOCK_FREE(unsigned char);
#if TEST_STD_VER > 17 && defined(__cpp_char8_t)
    CHECK_ALWAYS_LOCK_FREE(char8_t);
#endif
    CHECK_ALWAYS_LOCK_FREE(char16_t);
    CHECK_ALWAYS_LOCK_FREE(char32_t);
    CHECK_ALWAYS_LOCK_FREE(wchar_t);
    CHECK_ALWAYS_LOCK_FREE(short);
    CHECK_ALWAYS_LOCK_FREE(unsigned short);
    CHECK_ALWAYS_LOCK_FREE(int);
    CHECK_ALWAYS_LOCK_FREE(unsigned int);
    CHECK_ALWAYS_LOCK_FREE(long);
    CHECK_ALWAYS_LOCK_FREE(unsigned long);
    CHECK_ALWAYS_LOCK_FREE(long long);
    CHECK_ALWAYS_LOCK_FREE(unsigned long long);
    CHECK_ALWAYS_LOCK_FREE(std::nullptr_t);
    CHECK_ALWAYS_LOCK_FREE(void*);
    CHECK_ALWAYS_LOCK_FREE(float);
    CHECK_ALWAYS_LOCK_FREE(double);
    CHECK_ALWAYS_LOCK_FREE(long double);
    CHECK_ALWAYS_LOCK_FREE(struct Empty {});
    CHECK_ALWAYS_LOCK_FREE(struct OneInt {
        int i;
    });
    CHECK_ALWAYS_LOCK_FREE(struct IntArr2 {
        int i[2];
    });
    CHECK_ALWAYS_LOCK_FREE(struct FloatArr3 {
        float i[3];
    });
    CHECK_ALWAYS_LOCK_FREE(struct LLIArr2 {
        long long int i[2];
    });
    CHECK_ALWAYS_LOCK_FREE(struct LLIArr4 {
        long long int i[4];
    });
    CHECK_ALWAYS_LOCK_FREE(struct LLIArr8 {
        long long int i[8];
    });
    CHECK_ALWAYS_LOCK_FREE(struct LLIArr16 {
        long long int i[16];
    });
    CHECK_ALWAYS_LOCK_FREE(struct Padding {
        char c; /* padding */ long long int i;
    });
    CHECK_ALWAYS_LOCK_FREE(union IntFloat {
        int i; float f;
    });
    CHECK_ALWAYS_LOCK_FREE(enum class CharEnumClass : char {
        foo
    });

#undef CHECK_ALWAYS_LOCK_FREE

    // C macro and static constexpr must be consistent.
    enum class CharEnumClass : char {
        foo
    };
    static_assert(std::atomic<bool>::is_always_lock_free == (2 == ATOMIC_BOOL_LOCK_FREE), "");
    static_assert(std::atomic<char>::is_always_lock_free == (2 == ATOMIC_CHAR_LOCK_FREE), "");
    static_assert(std::atomic<CharEnumClass>::is_always_lock_free == (2 == ATOMIC_CHAR_LOCK_FREE), "");
    static_assert(std::atomic<signed char>::is_always_lock_free == (2 == ATOMIC_CHAR_LOCK_FREE), "");
    static_assert(std::atomic<unsigned char>::is_always_lock_free == (2 == ATOMIC_CHAR_LOCK_FREE), "");
#if TEST_STD_VER > 17 && defined(__cpp_char8_t)
    static_assert(std::atomic<char8_t>::is_always_lock_free == (2 == ATOMIC_CHAR8_T_LOCK_FREE), "");
#endif
    static_assert(std::atomic<char16_t>::is_always_lock_free == (2 == ATOMIC_CHAR16_T_LOCK_FREE), "");
    static_assert(std::atomic<char32_t>::is_always_lock_free == (2 == ATOMIC_CHAR32_T_LOCK_FREE), "");
    static_assert(std::atomic<wchar_t>::is_always_lock_free == (2 == ATOMIC_WCHAR_T_LOCK_FREE), "");
    static_assert(std::atomic<short>::is_always_lock_free == (2 == ATOMIC_SHORT_LOCK_FREE), "");
    static_assert(std::atomic<unsigned short>::is_always_lock_free == (2 == ATOMIC_SHORT_LOCK_FREE), "");
    static_assert(std::atomic<int>::is_always_lock_free == (2 == ATOMIC_INT_LOCK_FREE), "");
    static_assert(std::atomic<unsigned int>::is_always_lock_free == (2 == ATOMIC_INT_LOCK_FREE), "");
    static_assert(std::atomic<long>::is_always_lock_free == (2 == ATOMIC_LONG_LOCK_FREE), "");
    static_assert(std::atomic<unsigned long>::is_always_lock_free == (2 == ATOMIC_LONG_LOCK_FREE), "");
    static_assert(std::atomic<long long>::is_always_lock_free == (2 == ATOMIC_LLONG_LOCK_FREE), "");
    static_assert(std::atomic<unsigned long long>::is_always_lock_free == (2 == ATOMIC_LLONG_LOCK_FREE), "");
    static_assert(std::atomic<void*>::is_always_lock_free == (2 == ATOMIC_POINTER_LOCK_FREE), "");
    static_assert(std::atomic<std::nullptr_t>::is_always_lock_free == (2 == ATOMIC_POINTER_LOCK_FREE), "");

#if TEST_STD_VER >= 20
    static_assert(std::atomic_signed_lock_free::is_always_lock_free, "");
    static_assert(std::atomic_unsigned_lock_free::is_always_lock_free, "");
#endif
}

#endif
