#include "Test.h"
#include <stdexcept>
#include <memory>

/////////////////////////////////////////////////////////////
// CRT initialization and runtime behavior tests
// Covers: new/delete round-trip, exception handling, TLS lifecycle
/////////////////////////////////////////////////////////////

// Test: basic new/delete round-trip with default pool
TEST(new_delete_basic)
{
    int* p = new int(42);
    VERIFY(*p == 42);
    delete p;
}

// Test: array new/delete round-trip
TEST(new_delete_array)
{
    int* arr = new int[10];
    for (int i = 0; i < 10; ++i) {
        arr[i] = i;
    }
    VERIFY(arr[5] == 5);
    delete[] arr;
}

// Test: unique_ptr with custom deleter (exercises new/delete)
TEST(unique_ptr_basic)
{
    auto p = std::make_unique<int>(99);
    VERIFY(*p == 99);
}

// Test: exception throw and catch in kernel mode
TEST(exception_basic)
{
    bool caught = false;
    try {
        throw std::runtime_error("kernel test exception");
    }
    catch (const std::runtime_error& e) {
        caught = true;
        VERIFY(e.what() != nullptr);
    }
    VERIFY(caught);
}

// Test: exception through multiple stack frames
TEST(exception_stack_depth)
{
    auto inner = []() {
        throw std::logic_error("deep exception");
    };
    auto middle = [&]() {
        inner();
    };
    bool caught = false;
    try {
        middle();
    }
    catch (const std::logic_error&) {
        caught = true;
    }
    VERIFY(caught);
}

// Test: TLS variable lifecycle
TEST(tls_variable)
{
    thread_local int tls_value = 0;
    VERIFY(tls_value == 0);
    tls_value = 42;
    VERIFY(tls_value == 42);
}

// Test: multiple TLS variables
TEST(tls_multiple)
{
    thread_local double tls_double = 1.5;
    thread_local int tls_int = 100;
    VERIFY(tls_double == 1.5);
    VERIFY(tls_int == 100);
    tls_double = 3.14;
    tls_int = 200;
    VERIFY(tls_double == 3.14);
    VERIFY(tls_int == 200);
}

// Test: static local variable (not thread-safe, but should work single-threaded)
TEST(static_local_basic)
{
    static int init_count = 0;
    ++init_count;
    VERIFY(init_count == 1);
}

// Test: std::vector basic operations (default allocator, which routes through our overrides)
TEST(vector_basic)
{
    std::vector<int> v;
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);
    VERIFY(v.size() == 3);
    VERIFY(v[0] == 1);
    VERIFY(v[2] == 3);
}

// Test: std::string basic operations
TEST(string_basic)
{
    std::string s = "hello kernel";
    VERIFY(s.size() == 12);
    VERIFY(s[0] == 'h');
}

// Test: kallocator with default pool (pool='RsuM', pool=PagedPool)
TEST(kallocator_default_pool)
{
    std::vector<int, std::kallocator<int>> v;
    v.push_back(10);
    v.push_back(20);
    VERIFY(v.size() == 2);
    VERIFY(v[0] == 10);
}

// Test: kallocator with NonPagedPool
TEST(kallocator_non_paged)
{
    std::vector<int, std::kallocator<int, NonPagedPool>> v;
    v.push_back(100);
    v.push_back(200);
    VERIFY(v.size() == 2);
    VERIFY(v[0] == 100);
}

// Test: vector stress (exercises reallocation)
TEST(vector_stress)
{
    std::vector<int> v;
    for (int i = 0; i < 1000; ++i) {
        v.push_back(i);
    }
    VERIFY(v.size() == 1000);
    VERIFY(v.front() == 0);
    VERIFY(v.back() == 999);
}
