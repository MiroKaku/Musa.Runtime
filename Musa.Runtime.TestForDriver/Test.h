#pragma once


// Logging
#ifdef _DEBUG
#define MusaLOG(fmt, ...) DbgPrintEx(DPFLTR_DEFAULT_ID, DPFLTR_ERROR_LEVEL, \
    "[Musa.Runtime][%s():%u] " fmt "\n", __FUNCTION__, __LINE__, ## __VA_ARGS__)
#else
#define MusaLOG(...)
#pragma warning(disable: 4101 4189)
#endif

// Assertion - does NOT abort, continues to next test
#define KTEST_EXPECT(expr, name)                                              \
    do {                                                                      \
        ++TestsRun;                                                           \
        if (expr) {                                                           \
            MusaLOG("[PASS] %s", name);                                       \
        }                                                                     \
        else {                                                                \
            ++TestsFailed;                                                    \
            MusaLOG("[FAIL] %s", name);                                       \
        }                                                                     \
    } while (false)

#undef ASSERT
#define ASSERT(cond)      KTEST_EXPECT((cond), #cond)
#define ASSERT_EQ(a, b)   KTEST_EXPECT(((a) == (b)), #a " == " #b)
#define ASSERT_NE(a, b)   KTEST_EXPECT(((a) != (b)), #a " != " #b)
#define ASSERT_TRUE(x)    KTEST_EXPECT((x), #x)
#define ASSERT_FALSE(x)   KTEST_EXPECT(!(x), "!(" #x ")")
#define ASSERT_NULL(p)    KTEST_EXPECT(((p) == nullptr), #p " == nullptr")
#define ASSERT_NONNULL(p) KTEST_EXPECT(((p) != nullptr), #p " != nullptr")

///////////////////////////////////////////////////////////////

#define TEST_HAS_NO_THREADS
