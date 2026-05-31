#include <Veil.h>
#include <new>
#include <cstdlib>
#include <exception>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <list>
#include <deque>
#include <array>
#include <queue>
#include <stack>
#include <algorithm>
#include <numeric>
#include <random>
#include <functional>
#include <atomic>
#include <cstdint>
#include <memory>
#include <tuple>
#include <chrono>
#include <limits>
#include <iterator>
#include <utility>
#include <bit>
#include <format>
#include <regex>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <cctype>
#include <cstdlib>
#include <sys/stat.h>
#include <sys/types.h>
#include <io.h>
#include <direct.h>
#include <fcntl.h>
#include <kmalloc.h>
#include <kallocator.h>

#include "Test.h"


namespace Main
{
    static void RunSehTests(ULONG& TestsRun, ULONG& TestsFailed)
    {
        // CRT: SEH (Structured Exception Handling)
        {
            // Test: __try/__except — catch access violation
            bool filter_reached = false;
            bool handler_reached = false;
            __try
            {
                *(volatile int*)nullptr = 0;
            }
            __except(filter_reached = true, EXCEPTION_EXECUTE_HANDLER)
            {
                handler_reached = true;
            }
            KTEST_EXPECT(handler_reached, "SEH_AccessViolationCaught");
        }
        {
            // Test: __try/__finally — finally executes
            int finally_count = 0;
            __try
            {
                ++finally_count;
            }
            __finally
            {
                ++finally_count;
            }
            KTEST_EXPECT(finally_count == 2, "SEH_FinallyExecutes");
        }
        {
            // Test: __try/__except — filter can decline to handle
            bool exception_occurred = false;
            bool did_not_catch = false;
            __try
            {
                __try
                {
                    exception_occurred = true;
                    RaiseException(0xE0000001, 0, 0, nullptr);
                }
                __except(EXCEPTION_CONTINUE_SEARCH)
                {
                }
            }
            __except(EXCEPTION_EXECUTE_HANDLER)
            {
                did_not_catch = true;
            }
            KTEST_EXPECT(exception_occurred && did_not_catch, "SEH_ContinueSearch");
        }
        {
            // Test: GetExceptionCode() returns the exception code
            DWORD code = 0;
            __try
            {
                RaiseException(0xE0000042, 0, 0, nullptr);
            }
            __except(code = GetExceptionCode(), EXCEPTION_EXECUTE_HANDLER)
            {
            }
            KTEST_EXPECT(code == 0xE0000042, "SEH_GetExceptionCode");
        }
        {
            // Test: __try/__finally — finally runs even on exception
            int counter = 0;
            __try
            {
                __try
                {
                    counter = 1;
                    RaiseException(0xE0000002, 0, 0, nullptr);
                }
                __finally
                {
                    counter = 2;
                }
            }
            __except(EXCEPTION_EXECUTE_HANDLER)
            {
            }
            KTEST_EXPECT(counter == 2, "SEH_FinallyRunsOnException");
        }
    }

    static void RunTests()
    {
        ULONG TestsRun    = 0;
        ULONG TestsFailed = 0;

        MusaLOG("=== Musa.Runtime Kernel Test Suite ===");

        // CRT: new / delete
        {
            int* p = new int(42);
            KTEST_EXPECT(*p == 42, "NewDelete_ScalarInt");
            delete p;
            double* d = new double(3.14);
            KTEST_EXPECT(*d == 3.14, "NewDelete_ScalarDouble");
            delete d;
        }
        {
            int* arr = new int[100];
            for (int i = 0; i < 100; ++i) arr[i] = i * 2;
            KTEST_EXPECT(arr[50] == 100, "NewDelete_Array_WriteRead");
            delete[] arr;
            char* zarr = new char[256]();
            bool z = true;
            for (int i = 0; i < 256; ++i) { if (zarr[i] != 0) z = false; }
            KTEST_EXPECT(z, "NewDelete_ArrayZeroInit");
            delete[] zarr;
        }

        // CRT: kmalloc
        {
            void* p = kmalloc(1024, PagedPool, 'TsrT');
            KTEST_EXPECT(p != nullptr, "KMalloc_1024Bytes");
            if (p) kfree(p, 'TsrT');
            void* z = kmalloc(0, PagedPool, 'TsrT');
            KTEST_EXPECT(z != nullptr, "KMalloc_ZeroSize");
            if (z) kfree(z, 'TsrT');
        }
        {
            int* arr = static_cast<int*>(kcalloc(10, sizeof(int), NonPagedPool, 'TsrT'));
            KTEST_EXPECT(arr != nullptr, "KCalloc_NonNull");
            if (arr) {
                bool z = true;
                for (int i = 0; i < 10; ++i) { if (arr[i] != 0) z = false; }
                KTEST_EXPECT(z, "KCalloc_Zeroed");
                kfree(arr, 'TsrT');
            }
        }
        {
            void* p = kmalloc(64, PagedPool, 'TsrT');
            KTEST_EXPECT(p != nullptr, "KRealloc_AllocInitial");
            if (p) {
                memset(p, 0xAB, 64);
                void* np = krealloc(p, 64, 128, PagedPool, 'TsrT');
                KTEST_EXPECT(np != nullptr, "KRealloc_Succeeds");
                if (np) {
                    const auto* cp = static_cast<const unsigned char*>(np);
                    bool ok = true;
                    for (int i = 0; i < 64; ++i) { if (cp[i] != 0xAB) ok = false; }
                    KTEST_EXPECT(ok, "KRealloc_DataPreserved");
                    kfree(np, 'TsrT');
                }
            }
        }

        // CRT: Exceptions
        {
            bool caught = false;
            try { throw std::runtime_error("test"); }
            catch (const std::runtime_error&) { caught = true; }
            KTEST_EXPECT(caught, "Exception_ThrowCatchRuntimeError");
        }
        {
            bool caught = false;
            try { throw 42; }
            catch (...) { caught = true; }
            KTEST_EXPECT(caught, "Exception_CatchAll");
        }
        {
            int level = 0;
            try { try { try { throw std::wstring(L"deep"); } catch (int&) { level = 1; } } catch (const std::string&) { level = 2; } }
            catch (...) { level = 3; }
            KTEST_EXPECT(level == 3, "Exception_NestedCatchAll");
        }

        RunSehTests(TestsRun, TestsFailed);

        // Vector

        {
            std::vector<int, std::kallocator<int>> vec;
            KTEST_EXPECT(vec.empty(), "Vector_InitiallyEmpty");
            vec.push_back(1); vec.push_back(2); vec.push_back(3);
            KTEST_EXPECT(vec.size() == 3, "Vector_SizeAfterPush");
            KTEST_EXPECT(vec[0] == 1 && vec[1] == 2 && vec[2] == 3, "Vector_IndexAccess");
            vec.pop_back();
            KTEST_EXPECT(vec.size() == 2 && vec.back() == 2, "Vector_PopBack");
        }
        {
            std::vector<int, std::kallocator<int, NonPagedPool, 'TsrT'>> vec;
            for (int i = 0; i < 100; ++i) vec.push_back(i * i);
            KTEST_EXPECT(vec.size() == 100 && vec[99] == 99 * 99, "Vector_NonPagedPool");
        }
        {
            std::vector<int, std::kallocator<int>> vec;
            vec.reserve(100);
            KTEST_EXPECT(vec.capacity() >= 100, "Vector_Reserve");
            vec.resize(50, -1);
            KTEST_EXPECT(vec.size() == 50 && vec[0] == -1 && vec[49] == -1, "Vector_Resize");
        }
        {
            std::vector<int, std::kallocator<int>> vec(10, 42);
            KTEST_EXPECT(vec.size() == 10 && vec[0] == 42, "Vector_FillConstructor");
        }
        {
            std::vector<int, std::kallocator<int>> a = {1, 2, 3}, b = {1, 2, 3}, c = {1, 2, 4};
            KTEST_EXPECT(a == b, "Vector_Equal");
            KTEST_EXPECT(a != c, "Vector_NotEqual");
            KTEST_EXPECT(a < c, "Vector_LessThan");
        }
        {
            std::vector<int, std::kallocator<int>> vec = {1, 2, 3, 4, 5};
            auto it = vec.erase(vec.begin() + 2);
            KTEST_EXPECT(vec.size() == 4 && *it == 4, "Vector_Erase");
            vec.insert(vec.begin() + 1, 99);
            KTEST_EXPECT(vec[1] == 99 && vec.size() == 5, "Vector_Insert");
        }
        {
            std::vector<int, std::kallocator<int>> vec = {3, 1, 4, 1, 5};
            vec.shrink_to_fit();
            KTEST_EXPECT(vec.size() == 5, "Vector_ShrinkToFit");
            vec.clear();
            KTEST_EXPECT(vec.empty() && vec.size() == 0, "Vector_Clear");
        }
        {
            std::vector<int, std::kallocator<int>> src = {100, 200, 300};
            auto dst = src;
            KTEST_EXPECT(dst == src, "Vector_CopyConstructor");
            auto moved = std::move(dst);
            KTEST_EXPECT(moved.size() == 3, "Vector_MoveConstructor");
        }
        {
            std::vector<int, std::kallocator<int>> vec;
            vec.emplace_back(1); vec.emplace_back(2); vec.emplace_back(3);
            KTEST_EXPECT(vec.size() == 3 && vec[2] == 3, "Vector_EmplaceBack");
        }
        {
            std::vector<int, std::kallocator<int>> vec;
            vec.reserve(0);
            KTEST_EXPECT(vec.empty(), "Vector_ReserveZero");
        }

        // String
        {
            std::string s = "hello";
            KTEST_EXPECT(s.size() == 5 && s == "hello", "String_Basic");
            s += " world";
            KTEST_EXPECT(s == "hello world" && s.find("world") == 6, "String_ConcatFind");
        }
        {
            std::string empty;
            KTEST_EXPECT(empty.empty() && empty.size() == 0, "String_Empty");
        }
        {
            std::string s(10, 'A');
            KTEST_EXPECT(s.size() == 10 && s[0] == 'A' && s[9] == 'A', "String_FillConstructor");
        }
        {
            std::string s = "hello world";
            KTEST_EXPECT(s.substr(0, 5) == "hello" && s.substr(6) == "world", "String_Substr");
            KTEST_EXPECT(s.compare("hello world") == 0, "String_Compare");
            KTEST_EXPECT(s.find("world") == 6 && s.find("xyz") == std::string::npos, "String_Find");
        }
        {
            std::string a = "abc", b = "abc", c = "abd";
            KTEST_EXPECT(a == b, "String_Equal");
            KTEST_EXPECT(a < c, "String_LessThan");
            KTEST_EXPECT(a != c, "String_NotEqual");
        }
        {
            std::string s = "hello";
            s.append(" world");
            KTEST_EXPECT(s == "hello world", "String_Append");
            s.insert(5, ",");
            KTEST_EXPECT(s == "hello, world", "String_Insert");
        }
        {
            KTEST_EXPECT(std::to_string(42).size() > 0, "String_ToString");
        }
        {
            KTEST_EXPECT(std::to_wstring(12345) == L"12345", "String_ToWString");
        }

        // Map
        {
            using MT = std::map<int, std::string, std::less<int>, std::kallocator<std::pair<const int, std::string>>>;
            MT m;
            m[1] = "one"; m[2] = "two"; m[3] = "three";
            KTEST_EXPECT(m.size() == 3 && m[1] == "one", "Map_InsertAccess");
            KTEST_EXPECT(m.count(2) == 1 && m.count(99) == 0, "Map_Count");
            m.erase(2);
            KTEST_EXPECT(m.size() == 2, "Map_Erase");
        }
        {
            using MT = std::map<int, std::string, std::less<int>, std::kallocator<std::pair<const int, std::string>>>;
            MT m;
            auto r = m.insert({1, "one"});
            KTEST_EXPECT(r.second && r.first->second == "one", "Map_InsertReturn");
            r = m.insert({1, "uno"});
            KTEST_EXPECT(!r.second && r.first->second == "one", "Map_InsertDuplicate");
        }
        {
            using MT = std::map<int, std::string, std::less<int>, std::kallocator<std::pair<const int, std::string>>>;
            MT m;
            m[1] = "one"; m[2] = "two"; m[3] = "three";
            auto it = m.find(2);
            KTEST_EXPECT(it != m.end() && it->second == "two", "Map_Find");
            m.erase(it);
            KTEST_EXPECT(m.size() == 2 && m.find(2) == m.end(), "Map_EraseIterator");
        }
        {
            using MT = std::map<std::string, int, std::less<std::string>, std::kallocator<std::pair<const std::string, int>>>;
            MT m;
            m["apple"] = 5; m["banana"] = 3; m["cherry"] = 7;
            int t = 0;
            for (const auto& p : m) t += p.second;
            KTEST_EXPECT(t == 15, "Map_RangeForSum");
        }
        {
            using MT = std::map<int, int, std::less<int>, std::kallocator<std::pair<const int, int>>>;
            MT m;
            m.try_emplace(1, 100); m.try_emplace(2, 200);
            KTEST_EXPECT(m.size() == 2 && m.at(1) == 100, "Map_TryEmplace");
        }

        // Unordered map
        {
            using MT = std::unordered_map<int, std::string, std::hash<int>, std::equal_to<int>, std::kallocator<std::pair<const int, std::string>>>;
            MT m;
            m[10] = "ten"; m[20] = "twenty";
            KTEST_EXPECT(m.size() == 2 && m.at(20) == "twenty", "UnorderedMap_At");
            auto it = m.find(10);
            KTEST_EXPECT(it != m.end() && it->second == "ten", "UnorderedMap_Find");
        }
        {
            using MT = std::unordered_map<int, std::string, std::hash<int>, std::equal_to<int>, std::kallocator<std::pair<const int, std::string>>>;
            MT m;
            m.try_emplace(1, "one"); m.try_emplace(2, "two");
            KTEST_EXPECT(m.size() == 2 && m.at(1) == "one", "UnorderedMap_TryEmplace");
        }
        {
            using MT = std::unordered_map<int, std::string, std::hash<int>, std::equal_to<int>, std::kallocator<std::pair<const int, std::string>>>;
            MT m;
            m[10] = "ten"; m[20] = "twenty";
            bool rm = m.erase(10) == 1;
            KTEST_EXPECT(rm && m.size() == 1, "UnorderedMap_EraseByKey");
        }

        // Set
        {
            std::set<int, std::less<int>, std::kallocator<int>> s;
            s.insert(5); s.insert(3); s.insert(7); s.insert(3);
            KTEST_EXPECT(s.size() == 3, "Set_Size");
            KTEST_EXPECT(s.count(5) && !s.count(99), "Set_Count");
            auto it = s.begin();
            KTEST_EXPECT(*it == 3 && *(++it) == 5 && *(++it) == 7, "Set_Ordering");
        }
        {
            std::set<int, std::less<int>, std::kallocator<int>> s;
            s.insert(10); s.insert(20); s.insert(30);
            auto it = s.lower_bound(15);
            KTEST_EXPECT(it != s.end() && *it == 20, "Set_LowerBound");
            it = s.upper_bound(20);
            KTEST_EXPECT(it != s.end() && *it == 30, "Set_UpperBound");
        }
        {
            std::set<int, std::less<int>, std::kallocator<int>> s;
            s.insert(1); s.insert(2); s.insert(3);
            auto it = s.find(2);
            KTEST_EXPECT(it != s.end() && *it == 2, "Set_Find");
            s.erase(it);
            KTEST_EXPECT(s.size() == 2 && !s.count(2), "Set_EraseIterator");
        }

        // Unordered set
        {
            std::unordered_set<int, std::hash<int>, std::equal_to<int>, std::kallocator<int>> s;
            s.insert(100); s.insert(200); s.insert(300);
            KTEST_EXPECT(s.count(200) == 1 && s.count(999) == 0, "UnorderedSet_Count");
            KTEST_EXPECT(s.bucket_count() > 0, "UnorderedSet_BucketCount");
        }
        {
            std::unordered_set<int, std::hash<int>, std::equal_to<int>, std::kallocator<int>> s;
            s.insert(10); s.insert(20);
            s.erase(10);
            KTEST_EXPECT(s.size() == 1 && !s.count(10), "UnorderedSet_Erase");
        }

        // List
        {
            std::list<int, std::kallocator<int>> lst;
            lst.push_back(1); lst.push_back(2); lst.push_front(0);
            KTEST_EXPECT(lst.size() == 3 && lst.front() == 0 && lst.back() == 2, "List_PushFrontBack");
            lst.pop_front();
            KTEST_EXPECT(lst.front() == 1, "List_PopFront");
        }
        {
            std::list<int, std::kallocator<int>> lst = {5, 3, 1, 4, 2};
            lst.sort();
            auto it = lst.begin();
            bool ok = *it == 1; ++it; ok = ok && *it == 2; ++it; ok = ok && *it == 3; ++it; ok = ok && *it == 4; ++it; ok = ok && *it == 5;
            KTEST_EXPECT(ok, "List_Sort");
        }
        {
            std::list<int, std::kallocator<int>> a = {1, 2, 3}, b = {4, 5};
            a.merge(b);
            KTEST_EXPECT(a.size() == 5 && b.empty(), "List_Merge");
        }
        {
            std::list<int, std::kallocator<int>> lst2 = {1, 1, 2, 2, 3};
            lst2.unique();
            KTEST_EXPECT(lst2.size() == 3, "List_Unique_RemovesAdjacent");
        }

        // Deque
        {
            std::deque<int, std::kallocator<int>> dq;
            dq.push_back(10); dq.push_front(5); dq.push_back(15);
            KTEST_EXPECT(dq.size() == 3 && dq[0] == 5 && dq[1] == 10 && dq[2] == 15, "Deque_Index");
        }
        {
            std::deque<int, std::kallocator<int>> dq = {1, 2, 3};
            dq.pop_front(); dq.pop_back();
            KTEST_EXPECT(dq.size() == 1 && dq.front() == 2, "Deque_PopFrontBack");
            dq.emplace_front(0); dq.emplace_back(4);
            KTEST_EXPECT(dq.front() == 0 && dq.back() == 4, "Deque_Emplace");
        }

        // Array
        {
            std::array<int, 5> arr = {1, 2, 3, 4, 5};
            KTEST_EXPECT(arr.size() == 5 && arr[0] == 1 && arr.at(4) == 5, "Array_Basic");
            arr.fill(42);
            bool a = true; for (auto v : arr) { if (v != 42) a = false; }
            KTEST_EXPECT(a, "Array_Fill");
        }
        {
            std::array<int, 4> a = {{1, 2, 3, 4}}, b = {{1, 2, 3, 4}};
            KTEST_EXPECT(a == b, "Array_Equal");
            KTEST_EXPECT(a.max_size() == 4, "Array_MaxSize");
        }

        // Queue
        {
            std::queue<int, std::deque<int, std::kallocator<int>>> q;
            q.push(1); q.push(2); q.push(3);
            KTEST_EXPECT(q.size() == 3 && q.front() == 1 && q.back() == 3, "Queue_FIFO");
            q.pop();
            KTEST_EXPECT(q.front() == 2, "Queue_Pop");
        }
        {
            std::queue<int, std::deque<int, std::kallocator<int>>> q;
            for (int i = 1; i <= 5; ++i) q.push(i);
            KTEST_EXPECT(q.size() == 5, "Queue_Size");
            while (!q.empty()) q.pop();
            KTEST_EXPECT(q.empty(), "Queue_EmptyAfterPop");
        }

        // Stack
        {
            std::stack<int, std::vector<int, std::kallocator<int>>> s;
            s.push(10); s.push(20); s.push(30);
            KTEST_EXPECT(s.size() == 3 && s.top() == 30, "Stack_LIFO");
            s.pop();
            KTEST_EXPECT(s.top() == 20, "Stack_Pop");
        }

        // Priority queue
        {
            std::priority_queue<int, std::vector<int, std::kallocator<int>>> pq;
            pq.push(3); pq.push(1); pq.push(4); pq.push(1); pq.push(5);
            KTEST_EXPECT(pq.top() == 5, "PriorityQueue_Top");
            pq.pop();
            KTEST_EXPECT(pq.top() == 4, "PriorityQueue_AfterPop");
        }

        // Edge cases
        {
            std::vector<int, std::kallocator<int>> v;
            std::map<int, int, std::less<int>, std::kallocator<std::pair<const int, int>>> m;
            std::string s;
            KTEST_EXPECT(v.empty() && m.empty() && s.empty(), "Edge_AllEmpty");
            KTEST_EXPECT(v.begin() == v.end(), "Edge_BeginEndEqual");
        }
        {
            std::vector<int, std::kallocator<int>> v = {42};
            KTEST_EXPECT(v.size() == 1 && v.front() == 42 && v.back() == 42, "Edge_SingleElementVector");
        }

        // Algorithms: sort
        {
            std::vector<int, std::kallocator<int>> vec = {5, 3, 8, 1, 9, 2, 7, 4, 6, 0};
            std::sort(vec.begin(), vec.end());
            bool ok = true;
            for (size_t i = 0; i < vec.size(); ++i) { if (vec[i] != static_cast<int>(i)) ok = false; }
            KTEST_EXPECT(ok, "Sort_Ascending");
        }
        {
            std::vector<int, std::kallocator<int>> vec = {1, 2, 3, 4, 5};
            std::sort(vec.begin(), vec.end(), std::greater<int>());
            KTEST_EXPECT(vec[0] == 5 && vec[4] == 1, "Sort_Descending");
        }
        {
            std::vector<int, std::kallocator<int>> vec = {3, 1, 4, 1, 5, 9, 2, 6};
            std::sort(vec.begin(), vec.end());
            bool ok = true;
            for (size_t i = 1; i < vec.size(); ++i) { if (vec[i] < vec[i-1]) ok = false; }
            KTEST_EXPECT(ok, "Algorithm_IsSortedAfterSort");
        }

        // Algorithms: find
        {
            std::vector<int, std::kallocator<int>> vec = {10, 20, 30, 40, 50};
            auto it = std::find(vec.begin(), vec.end(), 30);
            KTEST_EXPECT(it != vec.end() && *it == 30, "Find_Existing");
            KTEST_EXPECT(std::find(vec.begin(), vec.end(), 99) == vec.end(), "Find_NotFound");
        }
        {
            std::vector<int, std::kallocator<int>> vec = {10, 20, 30, 40, 50};
            auto lb = std::lower_bound(vec.begin(), vec.end(), 25);
            KTEST_EXPECT(lb != vec.end() && *lb == 30, "Algorithm_LowerBound");
            auto ub = std::upper_bound(vec.begin(), vec.end(), 30);
            KTEST_EXPECT(ub != vec.end() && *ub == 40, "Algorithm_UpperBound");
        }

        // Algorithms: accumulate
        {
            std::vector<int, std::kallocator<int>> vec = {1, 2, 3, 4, 5};
            KTEST_EXPECT(std::accumulate(vec.begin(), vec.end(), 0) == 15, "Accumulate_Sum");
            KTEST_EXPECT(std::accumulate(vec.begin(), vec.end(), 1, std::multiplies<int>()) == 120, "Accumulate_Product");
        }

        // Algorithms: transform
        {
            std::vector<int, std::kallocator<int>> src = {1, 2, 3, 4, 5}, dst(5);
            std::transform(src.begin(), src.end(), dst.begin(), [](int x) { return x * x; });
            KTEST_EXPECT(dst[0] == 1 && dst[1] == 4 && dst[2] == 9 && dst[3] == 16 && dst[4] == 25, "Transform_Square");
        }

        // Algorithms: copy / fill
        {
            std::vector<int, std::kallocator<int>> src = {100, 200, 300}, dst(3);
            std::copy(src.begin(), src.end(), dst.begin());
            KTEST_EXPECT(dst[0] == 100 && dst[1] == 200 && dst[2] == 300, "Algorithm_Copy");
        }
        {
            std::vector<int, std::kallocator<int>> vec(5);
            std::fill(vec.begin(), vec.end(), 42);
            KTEST_EXPECT(vec[0] == 42 && vec[4] == 42, "Algorithm_Fill");
        }

        // Algorithms: remove
        {
            std::vector<int, std::kallocator<int>> vec = {1, 2, 3, 2, 4, 2, 5};
            vec.erase(std::remove(vec.begin(), vec.end(), 2), vec.end());
            KTEST_EXPECT(vec.size() == 4, "Algorithm_Remove");
        }

        // Algorithms: count / all_of / any_of
        {
            std::vector<int, std::kallocator<int>> vec = {1, 2, 3, 4, 5};
            KTEST_EXPECT(std::count(vec.begin(), vec.end(), 3) == 1, "Algorithm_Count");
            KTEST_EXPECT(std::count(vec.begin(), vec.end(), 99) == 0, "Algorithm_CountZero");
        }
        {
            std::vector<int, std::kallocator<int>> vec = {1, 2, 3, 4, 5};
            KTEST_EXPECT(std::all_of(vec.begin(), vec.end(), [](int x) { return x > 0; }), "Algorithm_AllOf");
            KTEST_EXPECT(std::any_of(vec.begin(), vec.end(), [](int x) { return x % 2 == 0; }), "Algorithm_AnyOf");
            KTEST_EXPECT(std::none_of(vec.begin(), vec.end(), [](int x) { return x < 0; }), "Algorithm_NoneOf");
        }

        // Algorithms: min/max element
        {
            std::vector<int, std::kallocator<int>> vec = {5, 3, 8, 1, 9};
            KTEST_EXPECT(*std::min_element(vec.begin(), vec.end()) == 1, "Algorithm_MinElement");
            KTEST_EXPECT(*std::max_element(vec.begin(), vec.end()) == 9, "Algorithm_MaxElement");
        }

        // Algorithms: binary_search
        {
            std::vector<int, std::kallocator<int>> vec = {1, 3, 5, 7, 9, 11};
            KTEST_EXPECT(std::binary_search(vec.begin(), vec.end(), 7), "BinarySearch_Found");
            KTEST_EXPECT(!std::binary_search(vec.begin(), vec.end(), 4), "BinarySearch_NotFound");
        }

        // Algorithms: min/max
        {
            KTEST_EXPECT(std::min(3, 7) == 3, "Algorithm_Min");
            KTEST_EXPECT(std::max(3, 7) == 7, "Algorithm_Max");
            KTEST_EXPECT(std::min({5, 2, 8, 1, 9}) == 1, "Algorithm_MinList");
            KTEST_EXPECT(std::max({5, 2, 8, 1, 9}) == 9, "Algorithm_MaxList");
        }

        // Algorithms: reverse / rotate
        {
            std::vector<int, std::kallocator<int>> vec = {1, 2, 3, 4, 5};
            std::reverse(vec.begin(), vec.end());
            KTEST_EXPECT(vec[0] == 5 && vec[4] == 1, "Reverse");
        }
        // Algorithms: equal

        // Algorithms: equal
        {
            std::vector<int, std::kallocator<int>> a = {1, 2, 3}, b = {1, 2, 3}, c = {1, 2, 4};
            KTEST_EXPECT(std::equal(a.begin(), a.end(), b.begin()), "Algorithm_Equal");
            KTEST_EXPECT(!std::equal(a.begin(), a.end(), c.begin()), "Algorithm_NotEqual");
        }

        // Algorithms: iota
        {
            std::vector<int, std::kallocator<int>> vec(10);
            std::iota(vec.begin(), vec.end(), 1);
            KTEST_EXPECT(vec[0] == 1 && vec[9] == 10, "Algorithm_Iota");
        }

        // Algorithms: partition
        {
            std::vector<int, std::kallocator<int>> vec = {1, 2, 3, 4, 5, 6};
            auto mid = std::partition(vec.begin(), vec.end(), [](int x) { return x <= 3; });
            KTEST_EXPECT(std::all_of(vec.begin(), mid, [](int x) { return x <= 3; }), "Algorithm_Partition");
        }

        // Algorithms: replace_if
        {
            std::vector<int, std::kallocator<int>> vec = {1, 2, 3, 4, 5};
            std::replace_if(vec.begin(), vec.end(), [](int x) { return x % 2 == 0; }, 0);
            KTEST_EXPECT(vec[1] == 0 && vec[3] == 0 && vec[0] == 1, "Algorithm_ReplaceIf");
        }

        // Algorithms: for_each
        {
            std::vector<int, std::kallocator<int>> vec = {1, 2, 3, 4, 5};
            int sum = 0;
            std::for_each(vec.begin(), vec.end(), [&sum](int x) { sum += x; });
            KTEST_EXPECT(sum == 15, "Algorithm_ForEach");
        }

        // Algorithms: merge
        {
            std::vector<int, std::kallocator<int>> a = {1, 2, 3}, b = {4, 5, 6}, result(6);
            std::merge(a.begin(), a.end(), b.begin(), b.end(), result.begin());
            KTEST_EXPECT(result[0] == 1 && result[5] == 6, "Algorithm_Merge");
        }

        // Iterator
        {
            std::vector<int, std::kallocator<int>> vec = {10, 20, 30, 40, 50};
            auto it = vec.begin();
            std::advance(it, 2);
            KTEST_EXPECT(*it == 30, "Iterator_Advance");
            KTEST_EXPECT(std::distance(vec.begin(), it) == 2, "Iterator_Distance");
        }

        // Numeric: partial_sum
        {
            std::vector<int, std::kallocator<int>> src = {1, 2, 3, 4, 5}, dst(5);
            std::partial_sum(src.begin(), src.end(), dst.begin());
            KTEST_EXPECT(dst[0] == 1 && dst[1] == 3 && dst[2] == 6 && dst[4] == 15, "Numeric_PartialSum");
        }

        // Numeric: adjacent_difference
        {
            std::vector<int, std::kallocator<int>> vec = {10, 3, 7, 1, 5}, result(5);
            std::adjacent_difference(vec.begin(), vec.end(), result.begin());
            KTEST_EXPECT(result[0] == 10 && result[1] == -7 && result[2] == 4, "Numeric_AdjacentDiff");
        }

        // Numeric: inner_product
        {
            std::vector<int, std::kallocator<int>> a = {1, 2, 3}, b = {4, 5, 6};
            KTEST_EXPECT(std::inner_product(a.begin(), a.end(), b.begin(), 0) == 32, "Numeric_InnerProduct");
        }

        // Random
        {
            std::mt19937_64 rng(42);
            auto v1 = rng();
            std::mt19937_64 rng2(42);
            KTEST_EXPECT(rng2() == v1, "Random_DeterministicSeed");
            std::mt19937_64 rng3(99);
            KTEST_EXPECT(rng3() != v1, "Random_DifferentSeed");
        }
        {
            std::mt19937_64 rng(12345);
            std::uniform_int_distribution<int> dist(1, 100);
            bool ok = true;
            for (int i = 0; i < 100; ++i) { int v = dist(rng); if (v < 1 || v > 100) ok = false; }
            KTEST_EXPECT(ok, "Random_UniformIntInRange");
        }

        // Atomic
        {
            std::atomic<int> a(0);
            a.store(42);
            KTEST_EXPECT(a.load() == 42, "Atomic_LoadStore");
            a = 100;
            KTEST_EXPECT(int(a) == 100, "Atomic_OperatorAssign");
        }
        {
            std::atomic<int> a(10);
            int old = a.exchange(20);
            KTEST_EXPECT(old == 10 && a.load() == 20, "Atomic_Exchange");
        }
        {
            std::atomic<int> a(5);
            int expected = 5;
            bool ok = a.compare_exchange_strong(expected, 10);
            KTEST_EXPECT(ok && a.load() == 10, "Atomic_CAS_Success");
            expected = 99;
            ok = a.compare_exchange_strong(expected, 20);
            KTEST_EXPECT(!ok && a.load() == 10 && expected == 10, "Atomic_CAS_Failure");
        }
        {
            std::atomic<int> a(0);
            int old = a.fetch_add(5);
            KTEST_EXPECT(old == 0 && a.load() == 5, "Atomic_FetchAdd");
            a += 10;
            KTEST_EXPECT(a.load() == 15, "Atomic_OperatorPlus");
        }
        {
            std::atomic<int> a(20);
            int old = a.fetch_sub(7);
            KTEST_EXPECT(old == 20 && a.load() == 13, "Atomic_FetchSub");
        }
        {
            std::atomic<uint32_t> a(0xFF00);
            uint32_t v1 = a.fetch_or(0x000F);
            KTEST_EXPECT(v1 == 0xFF00, "Atomic_FetchOr_ReturnOld");
            KTEST_EXPECT(a.load() == 0xFF0F, "Atomic_FetchOr_Result");
            uint32_t v2 = a.fetch_and(0xF0F0);
            KTEST_EXPECT(v2 == 0xFF0F, "Atomic_FetchAnd_ReturnOld");
            KTEST_EXPECT(a.load() == 0xF000, "Atomic_FetchAnd_Result");
            uint32_t v3 = a.fetch_xor(0xF0F0);
            KTEST_EXPECT(v3 == 0xF000, "Atomic_FetchXor_ReturnOld");
            KTEST_EXPECT(a.load() == 0x00F0, "Atomic_FetchXor_Result");
        }
        {
            std::atomic<bool> flag(false);
            KTEST_EXPECT(!flag.load(), "AtomicBool_False");
            flag.store(true);
            KTEST_EXPECT(flag.load(), "AtomicBool_True");
        }
        {
            std::atomic_flag f = ATOMIC_FLAG_INIT;
            KTEST_EXPECT(!f.test_and_set(), "AtomicFlag_FirstTestAndSet");
            KTEST_EXPECT(f.test_and_set(), "AtomicFlag_SecondTestAndSet");
            f.clear();
            KTEST_EXPECT(!f.test_and_set(), "AtomicFlag_AfterClear");
        }
        {
            int value = 42;
            std::atomic<int*> ptr(&value);
            KTEST_EXPECT(*ptr.load() == 42, "AtomicPointer_Load");
            int other = 100;
            int* old = ptr.exchange(&other);
            KTEST_EXPECT(old == &value && *ptr.load() == 100, "AtomicPointer_Exchange");
        }
        {
            std::atomic<int> a;
            KTEST_EXPECT(a.is_lock_free(), "Atomic_IsLockFree");
        }

        // Pair
        {
            std::pair<int, std::string> p(42, "answer");
            KTEST_EXPECT(p.first == 42 && p.second == "answer", "Pair_Basic");
            auto p2 = std::make_pair(100, "hundred");
            KTEST_EXPECT(p2.first == 100 && p2.second == std::string("hundred"), "Pair_MakePair");
        }
        {
            std::pair<int, int> a = {1, 2}, b = {1, 2}, c = {1, 3};
            KTEST_EXPECT(a == b, "Pair_Equality");
            KTEST_EXPECT(a < c, "Pair_LessThan");
        }

        // Tuple
        {
            auto t = std::make_tuple(1, 2.5, std::string("hello"));
            KTEST_EXPECT(std::get<0>(t) == 1, "Tuple_GetInt");
            KTEST_EXPECT(std::get<1>(t) == 2.5, "Tuple_GetDouble");
            KTEST_EXPECT(std::get<2>(t) == "hello", "Tuple_GetString");
        }
        {
            std::tuple<int, std::string, double> t1(1, "a", 3.14), t2(1, "a", 3.14);
            KTEST_EXPECT(t1 == t2, "Tuple_Equality");
            static_assert(std::tuple_size<decltype(t1)>::value == 3, "tuple size should be 3");
        }
        {
            std::pair<int, int> p = {3, 4};
            int x, y;
            std::tie(x, y) = p;
            KTEST_EXPECT(x == 3 && y == 4, "Tuple_Tie");
        }

        // Functional
        {
            KTEST_EXPECT(std::plus<int>{}(2, 3) == 5, "Functional_Plus");
            KTEST_EXPECT(std::minus<int>{}(10, 3) == 7, "Functional_Minus");
            KTEST_EXPECT(std::multiplies<int>{}(4, 5) == 20, "Functional_Multiplies");
        }
        {
            KTEST_EXPECT(std::equal_to<int>{}(5, 5) && !std::equal_to<int>{}(5, 6), "Functional_EqualTo");
            KTEST_EXPECT(std::less<int>{}(3, 5) && !std::less<int>{}(5, 3), "Functional_Less");
            KTEST_EXPECT(std::greater<int>{}(5, 3) && !std::greater<int>{}(3, 5), "Functional_Greater");
        }
        {
            KTEST_EXPECT(std::negate<int>{}(42) == -42, "Functional_Negate");
        }
        {
            std::function<int(int, int)> fn = std::plus<int>();
            KTEST_EXPECT(fn(3, 4) == 7, "Functional_Function");
            fn = [](int a, int b) { return a * b; };
            KTEST_EXPECT(fn(3, 4) == 12, "Functional_FunctionLambda");
        }

        // Type Traits
        {
            KTEST_EXPECT(std::is_integral<int>::value, "TypeTraits_IsIntegral");
            KTEST_EXPECT(!std::is_integral<double>::value, "TypeTraits_NotIntegral");
            KTEST_EXPECT(std::is_floating_point<double>::value, "TypeTraits_IsFloatingPoint");
            KTEST_EXPECT(!std::is_floating_point<int>::value, "TypeTraits_NotFloatingPoint");
        }
        {
            KTEST_EXPECT(std::is_pointer<int*>::value, "TypeTraits_IsPointer");
            KTEST_EXPECT(!std::is_pointer<int>::value, "TypeTraits_NotPointer");
            KTEST_EXPECT(std::is_reference<int&>::value, "TypeTraits_IsReference");
        }
        {
            KTEST_EXPECT((std::is_same<int, int>::value), "TypeTraits_IsSame");
            KTEST_EXPECT((!std::is_same<int, double>::value), "TypeTraits_NotSame");
            KTEST_EXPECT(std::is_void<void>::value, "TypeTraits_IsVoid");
        }
        {
            KTEST_EXPECT((std::is_constructible<int, int>::value), "TypeTraits_IsConstructible");
            KTEST_EXPECT(std::is_default_constructible<int>::value, "TypeTraits_IsDefaultConstructible");
            KTEST_EXPECT(std::is_copy_constructible<int>::value, "TypeTraits_IsCopyConstructible");
        }
        {
            KTEST_EXPECT(std::is_arithmetic<int>::value, "TypeTraits_IsArithmetic");
            KTEST_EXPECT(std::is_arithmetic<float>::value, "TypeTraits_IsArithmeticFloat");
            KTEST_EXPECT(!std::is_arithmetic<std::string>::value, "TypeTraits_NotArithmetic");
        }

        // unique_ptr
        {
            auto up = std::unique_ptr<int>(new int(42));
            KTEST_EXPECT(up != nullptr && *up == 42, "UniquePtr_Basic");
        }
        {
            std::unique_ptr<int> up1(new int(10));
            std::unique_ptr<int> up2 = std::move(up1);
            KTEST_EXPECT(up1 == nullptr && *up2 == 10, "UniquePtr_Move");
        }
        {
            auto up = std::make_unique<int>(100);
            KTEST_EXPECT(*up == 100, "UniquePtr_MakeUnique");
            up.reset();
            KTEST_EXPECT(up == nullptr, "UniquePtr_Reset");
        }
        {
            auto up = std::make_unique<int>(42);
            int* raw = up.release();
            KTEST_EXPECT(up == nullptr && *raw == 42, "UniquePtr_Release");
            delete raw;
        }
        {
            auto up = std::make_unique<int[]>(10);
            for (int i = 0; i < 10; ++i) up[i] = i * i;
            KTEST_EXPECT(up[0] == 0 && up[9] == 81, "UniquePtr_Array");
        }

        // shared_ptr
        {
            auto sp = std::make_shared<int>(42);
            KTEST_EXPECT(*sp == 42 && sp.use_count() == 1, "SharedPtr_Basic");
        }
        {
            auto sp1 = std::make_shared<int>(100);
            {
                auto sp2 = sp1;
                KTEST_EXPECT(sp1.use_count() == 2 && sp2.use_count() == 2, "SharedPtr_Shared");
            }
            KTEST_EXPECT(sp1.use_count() == 1, "SharedPtr_AfterScope");
        }
        {
            std::shared_ptr<int> sp1 = std::make_shared<int>(42);
            std::shared_ptr<int> sp2;
            KTEST_EXPECT(sp2 == nullptr, "SharedPtr_DefaultNull");
            sp2 = sp1;
            KTEST_EXPECT(sp2 != nullptr && *sp2 == 42, "SharedPtr_CopyAssign");
        }
        {
            auto sp = std::make_shared<int>(42);
            sp.reset();
            KTEST_EXPECT(sp == nullptr, "SharedPtr_Reset");
        }
        {
            auto sp = std::make_shared<int>(42);
            int* raw = sp.get();
            KTEST_EXPECT(raw != nullptr && *raw == 42, "SharedPtr_Get");
        }

        // Numeric limits
        {
            KTEST_EXPECT(std::numeric_limits<int>::min() < std::numeric_limits<int>::max(), "NumericLimits_Int");
            KTEST_EXPECT(std::numeric_limits<unsigned int>::max() > 0, "NumericLimits_UInt");
            KTEST_EXPECT(std::numeric_limits<double>::is_iec559, "NumericLimits_Float");
            static_assert(std::numeric_limits<int>::digits > 0, "int digits should be positive");
        }

        // Bit manipulation
        {
            KTEST_EXPECT(std::popcount(0u) == 0, "Bit_PopcountZero");
            KTEST_EXPECT(std::popcount(7u) == 3, "Bit_PopcountSeven");
            KTEST_EXPECT(std::popcount(0xFFu) == 8, "Bit_PopcountFF");
        }
        {
            KTEST_EXPECT(std::bit_width(0u) == 0, "Bit_WidthZero");
            KTEST_EXPECT(std::bit_width(8u) == 4, "Bit_WidthEight");
        }
        {
            KTEST_EXPECT(std::has_single_bit(8u), "Bit_HasSingleBit");
            KTEST_EXPECT(!std::has_single_bit(6u), "Bit_NotSingleBit");
            KTEST_EXPECT(std::bit_ceil(5u) == 8u, "Bit_Ceil");
        }

        // Utility
        {
            int x = 10;
            int old = std::exchange(x, 20);
            KTEST_EXPECT(old == 10 && x == 20, "Utility_Exchange");
        }
        {
            int x = 42;
            const int& cx = std::as_const(x);
            KTEST_EXPECT(cx == 42, "Utility_AsConst");
        }
        {
            std::string s = "hello";
            std::string s2 = std::move(s);
            KTEST_EXPECT(s2 == "hello", "Utility_Move");
        }

        // Chrono
        {
            std::chrono::milliseconds ms(1000);
            KTEST_EXPECT(ms.count() == 1000, "Chrono_Milliseconds");
            auto s = std::chrono::duration_cast<std::chrono::seconds>(ms);
            KTEST_EXPECT(s.count() == 1, "Chrono_CastToSeconds");
        }
        {
            auto now = std::chrono::steady_clock::now();
            auto later = now + std::chrono::milliseconds(100);
            KTEST_EXPECT(later > now, "Chrono_SteadyClockComparison");
        }

        // Allocator
        {
            auto alloc = std::kallocator<int>();
            int* p = alloc.allocate(10);
            for (int i = 0; i < 10; ++i) p[i] = i;
            KTEST_EXPECT(p[5] == 5, "Allocator_DirectUse");
            alloc.deallocate(p, 10);
        }
        {
            auto a1 = std::kallocator<int>();
            auto a2 = std::kallocator<double>();
            KTEST_EXPECT(a1 == a2, "Allocator_Equality");
        }


        // string to integer conversions
        {
            KTEST_EXPECT(std::stoi("42") == 42, "Stoi_Basic");
            KTEST_EXPECT(std::stoi("-10") == -10, "Stoi_Negative");
        }
        {
            KTEST_EXPECT(std::stol("100000") == 100000L, "Stol_Basic");
            KTEST_EXPECT(std::stol("-500") == -500L, "Stol_Negative");
        }
        {
            KTEST_EXPECT(std::stoul("99999") == 99999UL, "Stoul_Basic");
        }
        {
            KTEST_EXPECT(std::stoll("1234567890123") == 1234567890123LL, "Stoll_Basic");
            KTEST_EXPECT(std::stoll("-9876543210") == -9876543210LL, "Stoll_Negative");
        }
        {
            KTEST_EXPECT(std::stoull("18446744073709551615") == 18446744073709551615ULL, "Stoull_Basic");
        }

        // string to float/double conversions
        {
            float f = std::stof("3.14");
            KTEST_EXPECT(f > 3.13f && f < 3.15f, "Stof_Basic");
        }
        {
            float f = std::stof("-2.5");
            KTEST_EXPECT(f > -2.51f && f < -2.49f, "Stof_Negative");
        }
        {
            double d = std::stod("3.14159265358979");
            KTEST_EXPECT(d > 3.14159 && d < 3.14160, "Stod_Basic");
        }
        {
            double d = std::stod("-1.5");
            KTEST_EXPECT(d > -1.51 && d < -1.49, "Stod_Negative");
        }


        // wide string to int conversions
        {
            KTEST_EXPECT(std::stoi(L"42") == 42, "WStoi_Basic");
            KTEST_EXPECT(std::stoul(L"99999") == 99999UL, "WStoul_Basic");
        }


        // std::format (C++20)
        {
            auto s = std::format("hello {}", "world");
            KTEST_EXPECT(s == "hello world", "Format_Basic");
        }
        {
            auto s = std::format("{}+{}={}", 1, 2, 3);
            KTEST_EXPECT(s == "1+2=3", "Format_Integers");
        }
        // format_to
        {
            char buf[64];
            auto r = std::format_to(buf, "x={}", 42);
            *r = '\0';
            KTEST_EXPECT(strcmp(buf, "x=42") == 0, "FormatTo_Basic");
        }
        // format_to_n
        {
            char buf[16] = {};
            auto r = std::format_to_n(buf, 15, "{}", 12345);
            *r.out = '\0';
            KTEST_EXPECT(r.size == 5 && strcmp(buf, "12345") == 0, "FormatToN");
        }
        // width and alignment
        {
            auto s = std::format("{:>5}", 42);
            KTEST_EXPECT(s == "   42", "Format_RightAlign");
        }
        {
            auto s = std::format("{:<5}", 42);
            KTEST_EXPECT(s == "42   ", "Format_LeftAlign");
        }
        {
            auto s = std::format("{:^5}", 42);
            KTEST_EXPECT(s == " 42  ", "Format_CenterAlign");
        }
        // fill
        {
            auto s = std::format("{:*>5}", 42);
            KTEST_EXPECT(s == "***42", "Format_FillChar");
        }
        // hex
        {
            auto s = std::format("{:x}", 255);
            KTEST_EXPECT(s == "ff", "Format_Hex");
        }
        {
            auto s = std::format("{:#x}", 255);
            KTEST_EXPECT(s == "0xff", "Format_HexPrefix");
        }
        {
            auto s = std::format("{:#X}", 255);
            KTEST_EXPECT(s == "0XFF", "Format_HexUpper");
        }
        // octal
        {
            auto s = std::format("{:o}", 64);
            KTEST_EXPECT(s == "100", "Format_Octal");
        }
        // binary
        {
            auto s = std::format("{:b}", 5);
            KTEST_EXPECT(s == "101", "Format_Binary");
        }
        // float
        {
            auto s = std::format("{:.2f}", 3.14159);
            KTEST_EXPECT(s == "3.14", "Format_Float");
        }
        // sign
        {
            auto s = std::format("{:+}", 42);
            KTEST_EXPECT(s == "+42", "Format_Sign");
        }
        {
            auto s = std::format("{:-}", 42);
            KTEST_EXPECT(s == "42", "Format_MinusSign");
        }
        // multiple arguments
        {
            auto s = std::format("{} {} {}", "a", 1, 2.5);
            KTEST_EXPECT(s == "a 1 2.5", "Format_MultiArgs");
        }
        // position specifiers
        {
            auto s = std::format("{1} {0}", "world", "hello");
            KTEST_EXPECT(s == "hello world", "Format_PosArgs");
        }

        // std::regex
        {
            std::regex re("hello");
            KTEST_EXPECT(std::regex_match("hello", re), "Regex_Match");
            KTEST_EXPECT(!std::regex_match("world", re), "Regex_NoMatch");
        }
        {
            std::regex re("\\d+");
            KTEST_EXPECT(std::regex_match("12345", re), "Regex_MatchDigits");
            KTEST_EXPECT(!std::regex_match("abc", re), "Regex_NoMatchDigits");
        }
        {
            std::regex re("world");
            KTEST_EXPECT(std::regex_search("hello world", re), "Regex_Search");
        }
        {
            std::regex re("\\s+");
            auto s = std::regex_replace("a  b   c", re, " ");
            KTEST_EXPECT(s == "a b c", "Regex_Replace");
        }
        // Exception safety
        {
            std::vector<int, std::kallocator<int>> vec = {1, 2, 3};
            size_t orig_size = vec.size();
            try { vec.reserve(vec.max_size()); } catch (const std::exception&) {}
            KTEST_EXPECT(vec.size() == orig_size, "ExceptionSafety_VectorRollback");
        }

        // Large allocation
        {
            auto* big = static_cast<unsigned char*>(kmalloc(1024 * 1024, PagedPool, 'TsrT'));
            KTEST_EXPECT(big != nullptr, "Stress_LargeAlloc1MB");
            if (big) {
                big[0] = 0xAA;
                big[1024 * 1024 - 1] = 0x55;
                KTEST_EXPECT(big[0] == 0xAA && big[1024 * 1024 - 1] == 0x55, "Stress_LargeAllocWrite");
                kfree(big, 'TsrT');
            }
        }

        // Sync primitives
        {
            SRWLOCK lock = SRWLOCK_INIT;
            AcquireSRWLockExclusive(&lock);
            ReleaseSRWLockExclusive(&lock);
            KTEST_EXPECT(TryAcquireSRWLockExclusive(&lock), "Sync_SRWLock");
            ReleaseSRWLockExclusive(&lock);
        }
        {
            CRITICAL_SECTION cs;
            InitializeCriticalSection(&cs);
            EnterCriticalSection(&cs);
            KTEST_EXPECT(TryEnterCriticalSection(&cs), "Sync_CriticalSection");
            LeaveCriticalSection(&cs);
            LeaveCriticalSection(&cs);
            DeleteCriticalSection(&cs);
        }

        // ============================================================
        // UCRT Unlocked: ctype — Character Classification
        // ============================================================
        {
            KTEST_EXPECT(isalpha('A') != 0, "CType_isalpha_Upper");
            KTEST_EXPECT(isalpha('z') != 0, "CType_isalpha_Lower");
            KTEST_EXPECT(isalpha('0') == 0, "CType_isalpha_Digit");
            KTEST_EXPECT(isalpha(' ') == 0, "CType_isalpha_Space");
            KTEST_EXPECT(isdigit('0') != 0, "CType_isdigit_Zero");
            KTEST_EXPECT(isdigit('9') != 0, "CType_isdigit_Nine");
            KTEST_EXPECT(isdigit('A') == 0, "CType_isdigit_Alpha");
            KTEST_EXPECT(isupper('A') != 0, "CType_isupper");
            KTEST_EXPECT(isupper('z') == 0, "CType_isupper_Lower");
            KTEST_EXPECT(islower('a') != 0, "CType_islower");
            KTEST_EXPECT(islower('Z') == 0, "CType_islower_Upper");
            KTEST_EXPECT(isspace(' ') != 0, "CType_isspace_Space");
            KTEST_EXPECT(isspace('\t') != 0, "CType_isspace_Tab");
            KTEST_EXPECT(isspace('\n') != 0, "CType_isspace_Newline");
            KTEST_EXPECT(isspace('A') == 0, "CType_isspace_Alpha");
            KTEST_EXPECT(ispunct('!') != 0, "CType_ispunct_Exclaim");
            KTEST_EXPECT(ispunct(',') != 0, "CType_ispunct_Comma");
            KTEST_EXPECT(ispunct('A') == 0, "CType_ispunct_Alpha");
            KTEST_EXPECT(isalnum('A') != 0, "CType_isalnum_Alpha");
            KTEST_EXPECT(isalnum('9') != 0, "CType_isalnum_Digit");
            KTEST_EXPECT(isalnum(' ') == 0, "CType_isalnum_Space");
            KTEST_EXPECT(isxdigit('0') != 0, "CType_isxdigit_0");
            KTEST_EXPECT(isxdigit('9') != 0, "CType_isxdigit_9");
            KTEST_EXPECT(isxdigit('A') != 0, "CType_isxdigit_A");
            KTEST_EXPECT(isxdigit('f') != 0, "CType_isxdigit_f");
            KTEST_EXPECT(isxdigit('g') == 0, "CType_isxdigit_g");
            KTEST_EXPECT(isprint('A') != 0, "CType_isprint_Alpha");
            KTEST_EXPECT(isprint(' ') != 0, "CType_isprint_Space");
            KTEST_EXPECT(isgraph('A') != 0, "CType_isgraph_Alpha");
            KTEST_EXPECT(isgraph(' ') == 0, "CType_isgraph_Space");
            KTEST_EXPECT(iscntrl('\n') != 0, "CType_iscntrl_Newline");
            KTEST_EXPECT(iscntrl('A') == 0, "CType_iscntrl_Alpha");
            KTEST_EXPECT(isblank(' ') != 0, "CType_isblank_Space");
            KTEST_EXPECT(isblank('\t') != 0, "CType_isblank_Tab");
            KTEST_EXPECT(isblank('A') == 0, "CType_isblank_Alpha");
            KTEST_EXPECT(toupper('a') == 'A', "CType_toupper");
            KTEST_EXPECT(toupper('A') == 'A', "CType_toupper_Idempotent");
            KTEST_EXPECT(tolower('Z') == 'z', "CType_tolower");
            KTEST_EXPECT(tolower('z') == 'z', "CType_tolower_Idempotent");
            KTEST_EXPECT(__isascii('A') != 0, "CType___isascii");
            KTEST_EXPECT(__isascii(0x80) == 0, "CType___isascii_HighBit");
            KTEST_EXPECT(__toascii(0x80) == 0, "CType___toascii");
        }
        // _isctype / _isctype_l
        {
            KTEST_EXPECT(_isctype('A', _UPPER) != 0, "IsCType_Upper");
            KTEST_EXPECT(_isctype('z', _LOWER) != 0, "IsCType_Lower");
            KTEST_EXPECT(_isctype('5', _DIGIT) != 0, "IsCType_Digit");
            KTEST_EXPECT(_isctype(' ', _SPACE) != 0, "IsCType_Space");
            KTEST_EXPECT(_isctype('!', _PUNCT) != 0, "IsCType_Punct");
            KTEST_EXPECT(_isctype('\n', _CONTROL) != 0, "IsCType_Control");
            KTEST_EXPECT(_isctype(' ', _BLANK) != 0, "IsCType_Blank");
            KTEST_EXPECT(_isctype('\t', _BLANK) != 0, "IsCType_BlankTab");
            KTEST_EXPECT(_isctype('F', _HEX) != 0, "IsCType_Hex");
            KTEST_EXPECT(_isctype('A', _ALPHA) != 0, "IsCType_Alpha");
            KTEST_EXPECT(_isctype('A', _DIGIT) == 0, "IsCType_Negative");
            KTEST_EXPECT(_isctype_l('a', _LOWER, nullptr) != 0, "IsCType_L");
        }
#ifdef _DEBUG
        {
            KTEST_EXPECT(_chvalidator('A', _UPPER) != 0, "ChValidator");
            KTEST_EXPECT(_chvalidator_l(nullptr, '9', _DIGIT) != 0, "ChValidator_L");
        }
#endif


        // ============================================================
        // UCRT Unlocked: String I/O (sprintf)
        // ============================================================
        {
            char buf[256] = {};
            int n = sprintf(buf, "hello %s %d", "world", 42);
            KTEST_EXPECT(n > 0 && strcmp(buf, "hello world 42") == 0, "Sprintf");
        }
        {
            char buf[256] = {};
            sprintf(buf, "%d %x %o", 255, 255, 255);
            KTEST_EXPECT(strcmp(buf, "255 ff 377") == 0, "Sprintf_Formats");
        }


        // ============================================================
        // UCRT Unlocked: String (strcpy/strcat/strlen/strchr/strstr)
        // ============================================================
        {
            char buf[32] = {};
            strcpy(buf, "kernel");
            KTEST_EXPECT(strcmp(buf, "kernel") == 0 && strlen(buf) == 6, "StrCpy_Len");
            strcat(buf, "_mode");
            KTEST_EXPECT(strcmp(buf, "kernel_mode") == 0, "StrCat");
            KTEST_EXPECT(strchr("abcde", 'c') != nullptr, "StrChr");
            KTEST_EXPECT(strstr("hello world", "world") != nullptr, "StrStr");
        }


        {
            char buf[64] = {};
            time_t t = 1700000000;
            strftime(buf, sizeof(buf), "%Y-%m-%d", gmtime(&t));
            KTEST_EXPECT(strlen(buf) == 10 && buf[4] == '-', "StrFTime");
        }




        // ============================================================
        // UCRT Unlocked: stdlib (atoi/atol)
        // ============================================================
        {
            KTEST_EXPECT(atoi("42") == 42, "Atoi");
            KTEST_EXPECT(atol("1234567890") == 1234567890L, "Atol");
        }
        {
            KTEST_EXPECT(abs(-42) == 42 && abs(0) == 0, "Abs");
            KTEST_EXPECT(labs(-123456789L) == 123456789L, "LAbs");
        }
        {
            div_t d = div(10, 3);
            KTEST_EXPECT(d.quot == 3 && d.rem == 1, "Div");
        }


        // ============================================================
        // UCRT Unlocked: Time functions
        // ============================================================
        {
            time_t t = time(nullptr);
            KTEST_EXPECT(t > 0, "Time_Positive");
            time_t t2 = time(nullptr);
            KTEST_EXPECT(t2 >= t, "Time_Monotonic");
        }
        {
            KTEST_EXPECT(difftime(200, 100) == 100.0, "DiffTime");
        }
        {
            time_t t = 1700000000;
            struct tm* g = gmtime(&t);
            KTEST_EXPECT(g != nullptr && g->tm_year >= 70, "GmTime");
            struct tm* l = localtime(&t);
            KTEST_EXPECT(l != nullptr, "LocalTime");
        }
        {
            time_t t = time(nullptr);
            KTEST_EXPECT(asctime(localtime(&t)) != nullptr, "AscTime");
            KTEST_EXPECT(ctime(&t) != nullptr, "CTime");
        }
        {
            struct tm tm = {};
            tm.tm_year = 100; tm.tm_mon = 0; tm.tm_mday = 1;
            tm.tm_isdst = -1;
            KTEST_EXPECT(mktime(&tm) > 0, "MkTime");
        }
        {
            char buf[64] = {};
            time_t t = 1700000000;
            strftime(buf, sizeof(buf), "%Y-%m-%d", gmtime(&t));
            KTEST_EXPECT(strlen(buf) == 10 && buf[4] == '-', "StrFTime");
        }

        // ============================================================
        // UCRT Unlocked: Environment variables
        // ============================================================
        {
            int r = _putenv_s("MUSA_TEST", "hello");
            KTEST_EXPECT(r == 0, "PutEnvS");
            char* v = getenv("MUSA_TEST");
            KTEST_EXPECT(v != nullptr && strcmp(v, "hello") == 0, "GetEnv");
        }

        // ============================================================
        // UCRT Unlocked: Filesystem path utilities
        // ============================================================
        {
            char drive[8] = {}, dir[256] = {}, fname[64] = {}, ext[32] = {};
            _splitpath("C:\\Windows\\System32\\test.dll", drive, dir, fname, ext);
            KTEST_EXPECT(strcmp(drive, "C:") == 0, "SplitPath_Drive");
            KTEST_EXPECT(strcmp(fname, "test") == 0, "SplitPath_Fname");
            KTEST_EXPECT(strcmp(ext, ".dll") == 0, "SplitPath_Ext");
        }
        {
            char path[256] = {};
            _makepath(path, "C:", "\\dir\\sub\\", "file", ".txt");
            KTEST_EXPECT(strcmp(path, "C:\\dir\\sub\\file.txt") == 0, "MakePath");
        }

        // ============================================================
        // UCRT Unlocked: Snprintf / Sscanf
        // ============================================================
        {
            char buf[256] = {};
            int n = snprintf(buf, 10, "%s-%d", "abcdefgh", 99);
            KTEST_EXPECT(n > 9 && strcmp(buf, "abcdefgh-") == 0, "Snprintf_Trunc");
        }
        {
            int a = 0, b = 0;
            int n = sscanf("42 99", "%d %d", &a, &b);
            KTEST_EXPECT(n == 2 && a == 42 && b == 99, "Sscanf_Ints");
        }
        {
            unsigned int u = 0;
            int n = sscanf("0xDEAD", "%x", &u);
            KTEST_EXPECT(n == 1 && u == 0xDEAD, "Sscanf_Hex");
        }

        // ============================================================
        // UCRT Unlocked: strtol / strtoul / strtod
        // ============================================================
        {
            KTEST_EXPECT(strtol("0xFF", nullptr, 0) == 255L, "StrTol_Hex");
            KTEST_EXPECT(strtoul("4294967295", nullptr, 10) == 4294967295UL, "StrToul");
        }
        {
            double v = strtod("3.14159", nullptr);
            KTEST_EXPECT(v > 3.0 && v < 3.2, "StrToD");
        }

        // ============================================================
        // UCRT Unlocked: lowio — isatty
        // ============================================================
        {
            KTEST_EXPECT(_isatty(_fileno(stdout)) == 0, "IsATTY_Stdout");
        }
        // ============================================================
        // UCRT Unlocked: Memory functions
        // ============================================================
        {
            char src[16] = "test", dst[16] = {};
            memcpy(dst, src, 5);
            KTEST_EXPECT(memcmp(dst, src, 5) == 0, "MemCpy_MemCmp");
        }
        {
            char buf[16] = {};
            memset(buf, 'X', 10);
            KTEST_EXPECT(buf[0] == 'X' && buf[9] == 'X' && buf[10] == 0, "MemSet");
        }
        {
            char buf[16] = "0123456789";
            memmove(buf + 3, buf, 5);
            KTEST_EXPECT(buf[0] == '0' && buf[3] == '0' && buf[8] == '8', "MemMove_Overlap");
        }

        // ============================================================
        // UCRT Unlocked: String edge cases
        // ============================================================
        {
            char buf[32] = {};
            strncpy(buf, "hello world", 5);
            KTEST_EXPECT(strcmp(buf, "hello") == 0, "StrNCpy");
        }
        {
            const char* s = "hello world";
            KTEST_EXPECT(strrchr(s, 'o') == s + 7, "StrRChr");
            KTEST_EXPECT(memchr(s, 'w', 11) == s + 6, "MemChr");
        }
        {
            KTEST_EXPECT(strncmp("abc", "abd", 2) == 0, "StrNCmp_Equal2");
            KTEST_EXPECT(strncmp("abc", "abd", 3) < 0, "StrNCmp_Diff3");
        }

        // ============================================================
        // UCRT Unlocked: stdlib — qsort / bsearch
        // ============================================================
        {
            int arr[] = {5, 3, 8, 1, 9, 2};
            qsort(arr, 6, sizeof(int), [](const void* a, const void* b) {
                return *(const int*)a - *(const int*)b;
            });
            KTEST_EXPECT(arr[0] == 1 && arr[5] == 9, "QSort_Ascending");
        }
        {
            int arr[] = {1, 2, 3, 4, 5, 6};
            int key = 4;
            int* found = static_cast<int*>(bsearch(&key, arr, 6, sizeof(int),
                [](const void* a, const void* b) { return *(const int*)a - *(const int*)b; }));
            KTEST_EXPECT(found != nullptr && *found == 4, "BSearch_Found");
            key = 99;
            found = static_cast<int*>(bsearch(&key, arr, 6, sizeof(int),
                [](const void* a, const void* b) { return *(const int*)a - *(const int*)b; }));
            KTEST_EXPECT(found == nullptr, "BSearch_NotFound");
        }

        // ============================================================
        // UCRT Unlocked: ctype edge cases
        // ============================================================
        {
            KTEST_EXPECT(isalpha(EOF) == 0, "CType_EOF");
            KTEST_EXPECT(isdigit(-1) == 0, "CType_NegOne");
            KTEST_EXPECT(isupper(0x80) == 0, "CType_HighBit");
            KTEST_EXPECT(toupper('@') == '@', "ToUpper_NonAlpha");
            KTEST_EXPECT(tolower('[') == '[', "ToLower_NonAlpha");
        }

        // ============================================================
        // UCRT Unlocked: sscanf complex
        // ============================================================
        // UCRT Unlocked: locale — setlocale / localeconv
        // ============================================================
        {
            // Query locale returns "C" in kernel mode
            const char* loc = setlocale(LC_ALL, nullptr);
            KTEST_EXPECT(loc != nullptr && strcmp(loc, "C") == 0, "Locale_Setlocale_QueryC");
        }
        {
            // Setting locale to non-C should fail in kernel mode
            const char* loc = setlocale(LC_ALL, "en_US.UTF-8");
            KTEST_EXPECT(loc == nullptr, "Locale_Setlocale_ChangeFails");
        }
        {
            struct lconv* lc = localeconv();
            KTEST_EXPECT(lc != nullptr && strcmp(lc->decimal_point, ".") == 0, "Locale_Localeconv_DecimalPoint");
            KTEST_EXPECT(lc->thousands_sep[0] == '\0', "Locale_Localeconv_ThousandsSep");
        }

        // ============================================================
        // UCRT Unlocked: locale — strtod / strtof / atof
        // ============================================================
        {
            double d = strtod("3.14159", nullptr);
            KTEST_EXPECT(d > 3.14158 && d < 3.14160, "Strtod_Basic");
        }
        {
            char* endp;
            double d = strtod("2.5e3abc", &endp);
            KTEST_EXPECT(d == 2500.0 && *endp == 'a', "Strtod_ExponentEndptr");
        }
        {
            char* endp;
            double d = strtod("-inf", &endp);
            KTEST_EXPECT(d < 0.0 && !_finite(d), "Strtod_NegInf");
        }
        {
            char* endp;
            double d = strtod("  +1.23 ", &endp);
            KTEST_EXPECT(d > 1.229 && d < 1.231, "Strtod_WhitespaceSign");
        }
        {
            float f = strtof("1.5", nullptr);
            KTEST_EXPECT(f > 1.49f && f < 1.51f, "Strtof_Basic");
        }
        {
            double d = atof("123.456");
            KTEST_EXPECT(d > 123.455 && d < 123.457, "Atof_Basic");
        }
        {
            double d = _wtof(L"789.012");
            KTEST_EXPECT(d > 789.011 && d < 789.013, "Wtof_Basic");
        }

        // ============================================================
        // UCRT Unlocked: locale — mbstowcs / wcstombs
        // ============================================================
        {
            wchar_t wbuf[32];
            size_t n = mbstowcs(wbuf, "hello", sizeof(wbuf) / sizeof(wchar_t));
            KTEST_EXPECT(n == 5 && wcscmp(wbuf, L"hello") == 0, "Mbstowcs_ASCII");
        }
        {
            char buf[32];
            size_t n = wcstombs(buf, L"hello", sizeof(buf));
            KTEST_EXPECT(n == 5 && strcmp(buf, "hello") == 0, "Wcstombs_ASCII");
        }
        {
            wchar_t wbuf[32];
            size_t n = mbstowcs(wbuf, "caf\xc3\xa9", sizeof(wbuf) / sizeof(wchar_t));
            KTEST_EXPECT(n == 4 && wbuf[3] == 0xE9, "Mbstowcs_UTF8Cafe");
        }
        {
            char buf[32];
            size_t n = wcstombs(buf, L"caf\u00E9", sizeof(buf));
            KTEST_EXPECT(n == 5 && buf[3] == '\xC3' && buf[4] == '\xA9', "Wcstombs_UTF8Cafe");
        }

        // ============================================================
        // UCRT Unlocked: locale — mbrtowc / wcrtomb
        // ============================================================
        {
            wchar_t wc;
            mbstate_t state{};
            const char s[] = "h";
            size_t n = mbrtowc(&wc, s, 1, &state);
            KTEST_EXPECT(n == 1 && wc == L'h', "Mbrtowc_ASCII");
        }
        {
            wchar_t wc;
            mbstate_t state{};
            const char s[] = "\xC3\xA9"; // é in UTF-8
            size_t n = mbrtowc(&wc, s, 2, &state);
            KTEST_EXPECT(n == 2 && wc == 0x00E9, "Mbrtowc_UTF8TwoByte");
        }
        {
            char buf[4];
            mbstate_t state{};
            size_t n = wcrtomb(buf, L'é', &state);
            KTEST_EXPECT(n == 2 && buf[0] == '\xC3' && buf[1] == '\xA9', "Wcrtomb_UTF8TwoByte");
        }
        {
            char buf[4];
            mbstate_t state{};
            size_t n = wcrtomb(buf, L'a', &state);
            KTEST_EXPECT(n == 1 && buf[0] == 'a', "Wcrtomb_ASCII");
        }

        // ============================================================
        // UCRT Unlocked: locale — mblen
        // ============================================================
        {
            KTEST_EXPECT(mblen("a", 1) == 1, "Mblen_ASCII");
            KTEST_EXPECT(mblen("\xC3\xA9", 2) == 2, "Mblen_UTF8TwoByte");
        }

        // ============================================================
        // UCRT Unlocked: locale — string case conversion
        // ============================================================
        {
            char buf[] = "HELLO WORLD";
            char* r = _strlwr(buf);
            KTEST_EXPECT(r == buf && strcmp(buf, "hello world") == 0, "Strlwr_Basic");
        }
        {
            char buf[] = "hello world";
            char* r = _strupr(buf);
            KTEST_EXPECT(r == buf && strcmp(buf, "HELLO WORLD") == 0, "Strupr_Basic");
        }
        {
            wchar_t wbuf[] = L"HELLO WORLD";
            wchar_t* r = _wcslwr(wbuf);
            KTEST_EXPECT(r == wbuf && wcscmp(wbuf, L"hello world") == 0, "Wcslwr_Basic");
        }
        {
            wchar_t wbuf[] = L"hello world";
            wchar_t* r = _wcsupr(wbuf);
            KTEST_EXPECT(r == wbuf && wcscmp(wbuf, L"HELLO WORLD") == 0, "Wcsupr_Basic");
        }

        // ============================================================
        // UCRT Unlocked: locale — string collation
        // ============================================================
        {
            int r = strcoll("abc", "abc");
            KTEST_EXPECT(r == 0, "Strcoll_Equal");
        }
        {
            int r = strcoll("abc", "abd");
            KTEST_EXPECT(r < 0, "Strcoll_LessThan");
        }
        {
            int r = strcoll("abd", "abc");
            KTEST_EXPECT(r > 0, "Strcoll_GreaterThan");
        }
        {
            int r = _wcsicoll(L"ABC", L"abc");
            KTEST_EXPECT(r == 0, "Wcsicoll_CaseInsensitive");
        }

        // _strnicoll — case-insensitive collation
        {
            int r = _strnicoll("ABC", "abc", 3);
            KTEST_EXPECT(r == 0, "Strnicoll_CaseInsensitive");
        }
        {
            int r = _strnicoll("abc", "abd", 3);
            KTEST_EXPECT(r < 0, "Strnicoll_LessThan");
        }
        {
            int r = _strnicoll("abd", "abc", 3);
            KTEST_EXPECT(r > 0, "Strnicoll_GreaterThan");
        }

        // _wcsnicoll — wide case-insensitive collation
        {
            int r = _wcsnicoll(L"ABC", L"abc", 3);
            KTEST_EXPECT(r == 0, "Wcsnicoll_CaseInsensitive");
        }
        {
            int r = _wcsnicoll(L"abc", L"abd", 3);
            KTEST_EXPECT(r < 0, "Wcsnicoll_LessThan");
        }
        {
            int r = _wcsnicoll(L"abd", L"abc", 3);
            KTEST_EXPECT(r > 0, "Wcsnicoll_GreaterThan");
        }

        // ============================================================
        // UCRT Unlocked: locale — printf %e/%f/%g decimal_point
        // ============================================================
        {
            char buf[64];
            sprintf(buf, "%.2f", 3.14);
            KTEST_EXPECT(buf[1] == '.', "Printf_FloatDecimalPoint");
        }
        {
            char buf[64];
            sprintf(buf, "%.2e", 12345.0);
            KTEST_EXPECT(strstr(buf, "e+") != nullptr || strstr(buf, "E+") != nullptr, "Printf_Exponential");
        }
        {
            char buf[64];
            sprintf(buf, "%.4g", 3.14159);
            KTEST_EXPECT(buf[1] == '.', "Printf_GFormatDecimal");
        }

        // ============================================================
        // UCRT Unlocked: locale — wcsrtombs
        // ============================================================
        {
            wchar_t src[] = L"hello";
            const wchar_t* psrc = src;
            char buf[32];
            mbstate_t state{};
            size_t n = wcsrtombs(buf, &psrc, sizeof(buf), &state);
            KTEST_EXPECT(n == 5 && strcmp(buf, "hello") == 0, "Wcsrtombs_ASCII");
        }

        // ============================================================
        // UCRT Unlocked: locale — wctob
        // ============================================================
        {
            int c = wctob(L'a');
            KTEST_EXPECT(c == 'a', "Wctob_ASCII");
        }
        {
            int c = wctob(L'é');
            KTEST_EXPECT(c == EOF, "Wctob_NonASCII");
        }

        // ============================================================
        // ============================================================
        // ============================================================
        {
            float f = 0;
            KTEST_EXPECT(sscanf("3.14", "%f", &f) == 1 && f > 3.0f && f < 3.2f, "Sscanf_Float");
        }

        // ============================================================
        // UCRT Unlocked: lowio
        // ============================================================
        {
            int fd = _open("C:\\musa_lo.tmp", _O_CREAT | _O_RDWR | _O_BINARY, _S_IREAD | _S_IWRITE);
            KTEST_EXPECT(fd >= 0, "LowIO_Open");
            if (fd >= 0) {
                const char msg[] = "Hello kernel I/O!";
                int w = _write(fd, msg, sizeof(msg) - 1);
                KTEST_EXPECT(w == sizeof(msg) - 1, "LowIO_Write");
                KTEST_EXPECT(_lseek(fd, 0, SEEK_SET) == 0, "LowIO_LSeek");
                char buf[64] = {};
                int n = _read(fd, buf, sizeof(buf) - 1);
                KTEST_EXPECT(n == sizeof(msg) - 1, "LowIO_Read");
                KTEST_EXPECT(strcmp(buf, msg) == 0, "LowIO_ReadContent");
                KTEST_EXPECT(_close(fd) == 0, "LowIO_Close");
            }
            _unlink("C:\\musa_lo.tmp");
        }

        // ============================================================
        // UCRT Unlocked: stdio
        // ============================================================
        {
            FILE* f1 = fopen("C:\\musa_sio.tmp", "w+b");
            KTEST_EXPECT(f1 != nullptr, "StdIO_FOpen");
            if (f1) {
                const char data[] = "kernel stdio test";
                KTEST_EXPECT(fwrite(data, 1, sizeof(data) - 1, f1) == sizeof(data) - 1, "StdIO_FWrite");
                fflush(f1);
                rewind(f1);
                char buf[64] = {};
                KTEST_EXPECT(fread(buf, 1, sizeof(buf) - 1, f1) == sizeof(data) - 1, "StdIO_FRead");
                KTEST_EXPECT(strcmp(buf, data) == 0, "StdIO_FReadContent");
                KTEST_EXPECT(fclose(f1) == 0, "StdIO_FClose");
            }
            _unlink("C:\\musa_sio.tmp");
        }

        // ============================================================
        // UCRT Unlocked: formatted file I/O via sprintf+fwrite / fread+sscanf
        // ============================================================
        {
            FILE* f2 = fopen("C:\\musa_fmt.tmp", "w+b");
            KTEST_EXPECT(f2 != nullptr, "FmtIO_Open");
            if (f2) {
                char buf[128];
                int len = sprintf(buf, "answer=%d\n", 42);
                KTEST_EXPECT(fwrite(buf, 1, len, f2) == (size_t)len, "FmtIO_SprintfWrite");
                rewind(f2);
                char rbuf[128] = {};
                fread(rbuf, 1, sizeof(rbuf) - 1, f2);
                int val = 0;
                KTEST_EXPECT(sscanf(rbuf, "answer=%d", &val) == 1 && val == 42, "FmtIO_Sscanf");
                fclose(f2);
            }
            _unlink("C:\\musa_fmt.tmp");
        }

        // ============================================================
        // UCRT Unlocked: lowio tell / eof / filelength
        // ============================================================
        {
            int fd = _open("C:\\musa_te.tmp", _O_CREAT | _O_RDWR | _O_BINARY, _S_IREAD | _S_IWRITE);
            if (fd >= 0) {
                _write(fd, "1234567890", 10);
                KTEST_EXPECT(_tell(fd) == 10, "LowIO_Tell");
                KTEST_EXPECT(_lseek(fd, 0, SEEK_SET) == 0, "LowIO_LSeek");
                KTEST_EXPECT(_filelength(fd) == 10, "LowIO_FileLength");
                KTEST_EXPECT(_eof(fd) == 0, "LowIO_EOF_BeforeRead");
                char c;
                while (_read(fd, &c, 1) == 1) { }
                KTEST_EXPECT(_eof(fd) != 0, "LowIO_EOF_AfterRead");
                _close(fd);
            }
            _unlink("C:\\musa_te.tmp");
        }

        // ============================================================
        // UCRT Unlocked: filesystem — _stat / _fullpath
        // ============================================================
        {
            KTEST_EXPECT(_mkdir("C:\\musa_fsd") == 0, "FS_MkDir");
            FILE* fd = fopen("C:\\musa_fsd\\f.txt", "w");
            if (fd) { fwrite("data", 1, 4, fd); fclose(fd); }
            struct _stat64i32 st = {};
            KTEST_EXPECT(_stat64i32("C:\\musa_fsd\\f.txt", &st) == 0, "FS_Stat");
            KTEST_EXPECT(st.st_size == 4, "FS_StatSize");
            KTEST_EXPECT(_unlink("C:\\musa_fsd\\f.txt") == 0, "FS_Unlink");
            KTEST_EXPECT(_rmdir("C:\\musa_fsd") == 0, "FS_RmDir");
        }
        {
            char full[256] = {};
            char* r = _fullpath(full, "test.txt", sizeof(full));
            KTEST_EXPECT(r != nullptr && strlen(r) > 0, "FS_FullPath");
        }
        {
            FILE* fa = fopen("C:\\musa_rn.tmp", "w");
            if (fa) { fwrite("x", 1, 1, fa); fclose(fa); }
            KTEST_EXPECT(rename("C:\\musa_rn.tmp", "C:\\musa_rn2.tmp") == 0, "FS_Rename");
            _unlink("C:\\musa_rn2.tmp");
        }

        // ============================================================
        // UCRT Unlocked: stdio — fseek/ftell, fopen append mode
        // ============================================================
        {
            FILE* f = fopen("C:\\musa_seek.tmp", "w+b");
            if (f) {
                fwrite("0123456789", 1, 10, f);
                KTEST_EXPECT(fseek(f, 5, SEEK_SET) == 0, "StdIO_FSeek_Set");
                KTEST_EXPECT(ftell(f) == 5, "StdIO_FTell");
                KTEST_EXPECT(fseek(f, -2, SEEK_CUR) == 0, "StdIO_FSeek_Cur");
                KTEST_EXPECT(ftell(f) == 3, "StdIO_FSeek_CurTell");
                KTEST_EXPECT(fseek(f, 0, SEEK_END) == 0, "StdIO_FSeek_End");
                KTEST_EXPECT(ftell(f) == 10, "StdIO_FSeek_EndTell");
                fclose(f);
            }
            _unlink("C:\\musa_seek.tmp");
        }
        {
            FILE* f = fopen("C:\\musa_append.tmp", "a");
            KTEST_EXPECT(f != nullptr, "StdIO_FOpenAppend");
            if (f) {
                KTEST_EXPECT(fwrite("abc", 1, 3, f) == 3, "StdIO_AppendWrite");
                fclose(f);
                f = fopen("C:\\musa_append.tmp", "rb");
                char buf[16] = {};
                size_t n = fread(buf, 1, sizeof(buf) - 1, f);
                KTEST_EXPECT(n == 3 && strcmp(buf, "abc") == 0, "StdIO_AppendRead");
                fclose(f);
            }
            _unlink("C:\\musa_append.tmp");
        }

        // ============================================================
        // UCRT Unlocked: lowio — dup/dup2
        // ============================================================
        {
            int fd = _open("C:\\musa_dup.tmp", _O_CREAT | _O_RDWR | _O_BINARY, _S_IREAD | _S_IWRITE);
            if (fd >= 0) {
                _write(fd, "dup_test", 8);
                int fd2 = _dup(fd);
                KTEST_EXPECT(fd2 >= 0 && fd2 != fd, "LowIO_Dup");
                if (fd2 >= 0) {
                    _lseek(fd2, 0, SEEK_SET);
                    char buf[16] = {};
                    _read(fd2, buf, sizeof(buf) - 1);
                    KTEST_EXPECT(strcmp(buf, "dup_test") == 0, "LowIO_DupRead");
                    _close(fd2);
                }
                _close(fd);
            }
            _unlink("C:\\musa_dup.tmp");
        }

        // ============================================================
        // UCRT Unlocked: wide char string
        // ============================================================
        {
            wchar_t wbuf[32] = L"kernel";
            KTEST_EXPECT(wcslen(wbuf) == 6, "WcsLen");
            wchar_t wdst[32] = {};
            wcscpy(wdst, wbuf);
            KTEST_EXPECT(wcscmp(wdst, wbuf) == 0, "WcsCpy_WcsCmp");
        }

        // ============================================================
        // UCRT Unlocked: strtok
        // ============================================================
        {
            char s[] = "one,two,three";
            char* t1 = strtok(s, ",");
            char* t2 = strtok(nullptr, ",");
            char* t3 = strtok(nullptr, ",");
            KTEST_EXPECT(t1 && strcmp(t1, "one") == 0, "StrTok_1");
            KTEST_EXPECT(t2 && strcmp(t2, "two") == 0, "StrTok_2");
            KTEST_EXPECT(t3 && strcmp(t3, "three") == 0, "StrTok_3");
        }



        // ============================================================
        // UCRT Unlocked: stdlib — srand/rand, lfind
        // ============================================================
        {
            srand(42);
            int v1 = rand();
            srand(42);
            int v2 = rand();
            KTEST_EXPECT(v1 == v2, "SRand_Rand_Deterministic");
        }
        {
            int arr[] = {1, 3, 5, 7, 9};
            unsigned int n = 5;
            int key = 5;
            int* found = static_cast<int*>(_lfind(&key, arr, &n, sizeof(int),
                [](const void* a, const void* b) { return *(const int*)a - *(const int*)b; }));
            KTEST_EXPECT(found != nullptr && *found == 5, "LFind_Found");
            key = 42;
            found = static_cast<int*>(_lfind(&key, arr, &n, sizeof(int),
                [](const void* a, const void* b) { return *(const int*)a - *(const int*)b; }));
            KTEST_EXPECT(found == nullptr, "LFind_NotFound");

        // CRT: strerror / strerror_s
        {
            const char* msg = strerror(0);
            KTEST_EXPECT(msg != nullptr, "Strerror_Zero_NonNull");
        }
        {
            const char* msg = strerror(EINVAL);
            KTEST_EXPECT(msg != nullptr, "Strerror_EINVAL_NonNull");
            KTEST_EXPECT(msg[0] != '\0', "Strerror_EINVAL_NotEmpty");
        }
        {
            const char* msg = strerror(9999);
            KTEST_EXPECT(msg != nullptr, "Strerror_Unknown_NonNull");
        }
        {
            char buf[256] = {};
            errno_t err = strerror_s(buf, sizeof(buf), 0);
            KTEST_EXPECT(err == 0, "Strerror_s_Zero_Success");
            KTEST_EXPECT(buf[0] != '\0', "Strerror_s_Zero_NotEmpty");
        }
        {
            char buf[4] = {};
            errno_t err = strerror_s(buf, sizeof(buf), EINVAL);
            KTEST_EXPECT(err == 0, "Strerror_s_SmallBuf_Success");
        }

        // CRT: string functions (strspn/strcspn/strpbrk/memccpy/memicmp/strdup)
        {
            size_t spn = strspn("hello world", "helo");
            KTEST_EXPECT(spn == 5, "Strspn_Hello");
        }
        {
            size_t spn = strspn("abc", "xyz");
            KTEST_EXPECT(spn == 0, "Strspn_NoMatch");
        }
        {
            size_t spn = strcspn("hello world", " ");
            KTEST_EXPECT(spn == 5, "Strcspn_Space");
        }
        {
            size_t spn = strcspn("abc", "xyz");
            KTEST_EXPECT(spn == 3, "Strcspn_NoMatch");
        }
        {
            char* p = strpbrk((char*)"hello world", " ");
            KTEST_EXPECT(p != nullptr && *p == ' ', "Strpbrk_Space");
        }
        {
            char* p = strpbrk((char*)"abc", "xyz");
            KTEST_EXPECT(p == nullptr, "Strpbrk_NoMatch");
        }
        {
            char buf[16] = {};
            void* ret = _memccpy(buf, "hello", 'l', 5);
            KTEST_EXPECT(ret == buf + 3, "Memccpy_RetPtr");
            KTEST_EXPECT(buf[0]=='h' && buf[1]=='e' && buf[2]=='l', "Memccpy_Content");
        }
        {
            int r = _memicmp("HELLO", "hello", 5);
            KTEST_EXPECT(r == 0, "Memicmp_Equal");
        }
        {
            int r = _memicmp("ABCD", "abCd", 4);
            KTEST_EXPECT(r == 0, "Memicmp_MixedCase");
        }
        {
            int r = _memicmp("abc", "abd", 3);
            KTEST_EXPECT(r != 0, "Memicmp_Diff");
        }
        {
            size_t cnt = __strncnt("hello", 3);
            KTEST_EXPECT(cnt == 3, "Strncnt_Clamped");
        }
        {
            size_t cnt = __strncnt("hi", 10);
            KTEST_EXPECT(cnt == 2, "Strncnt_ShortStr");
        }
        {
            char* dup = _strdup("test");
            KTEST_EXPECT(dup != nullptr, "Strdup_NonNull");
            KTEST_EXPECT(strcmp(dup, "test") == 0, "Strdup_Equal");
            free(dup);
        }
        {
            wchar_t* dup = _wcsdup(L"test");
            KTEST_EXPECT(dup != nullptr, "Wcsdup_NonNull");
            KTEST_EXPECT(wcscmp(dup, L"test") == 0, "Wcsdup_Equal");
            free(dup);
        }

        // _strrev — reverse string in place
        {
            char buf[] = "hello";
            char* r = _strrev(buf);
            KTEST_EXPECT(r == buf, "Strrev_ReturnsInput");
            KTEST_EXPECT(strcmp(buf, "olleh") == 0, "Strrev_Reverse");
        }

        // _strset — fill string with char
        {
            char buf[] = "hello";
            char* r = _strset(buf, 'x');
            KTEST_EXPECT(r == buf, "Strset_ReturnsInput");
            KTEST_EXPECT(strcmp(buf, "xxxxx") == 0, "Strset_Fill");
        }

        // _strnset — fill n chars
        {
            char buf[] = "hello";
            char* r = _strnset(buf, 'x', 2);
            KTEST_EXPECT(r == buf, "Strnset_ReturnsInput");
            KTEST_EXPECT(strcmp(buf, "xxllo") == 0, "Strnset_Fill2");
        }

        // _wcsrev — reverse wide string in place
        {
            wchar_t buf[] = L"hello";
            wchar_t* r = _wcsrev(buf);
            KTEST_EXPECT(r == buf, "Wcsrev_ReturnsInput");
            KTEST_EXPECT(wcscmp(buf, L"olleh") == 0, "Wcsrev_Reverse");
        }

        // _wcsset — fill wide string with char
        {
            wchar_t buf[] = L"hello";
            wchar_t* r = _wcsset(buf, L'x');
            KTEST_EXPECT(r == buf, "Wcsset_ReturnsInput");
            KTEST_EXPECT(wcscmp(buf, L"xxxxx") == 0, "Wcsset_Fill");
        }

        // _wcsnset — fill n wide chars
        {
            wchar_t buf[] = L"hello";
            wchar_t* r = _wcsnset(buf, L'x', 2);
            KTEST_EXPECT(r == buf, "Wcsnset_ReturnsInput");
            KTEST_EXPECT(wcscmp(buf, L"xxllo") == 0, "Wcsnset_Fill2");
        }

        // _strnset_s — safe fill n chars
        {
            char buf[16] = "hello";
            errno_t e = _strnset_s(buf, sizeof(buf), 'x', 2);
            KTEST_EXPECT(e == 0, "Strnset_s_OK");
            KTEST_EXPECT(strcmp(buf, "xxllo") == 0, "Strnset_s_Fill2");
        }

        // _strset_s — safe fill string
        {
            char buf[16] = "hello";
            errno_t e = _strset_s(buf, sizeof(buf), 'x');
            KTEST_EXPECT(e == 0, "Strset_s_OK");
            KTEST_EXPECT(strcmp(buf, "xxxxx") == 0, "Strset_s_Fill");
        }

        // _wcsnset_s — safe wide fill n chars
        {
            wchar_t buf[16] = L"hello";
            errno_t e = _wcsnset_s(buf, sizeof(buf)/sizeof(wchar_t), L'x', 2);
            KTEST_EXPECT(e == 0, "Wcsnset_s_OK");
            KTEST_EXPECT(wcscmp(buf, L"xxllo") == 0, "Wcsnset_s_Fill2");
        }

        // _wcsset_s — safe wide fill string
        {
            wchar_t buf[16] = L"hello";
            errno_t e = _wcsset_s(buf, sizeof(buf)/sizeof(wchar_t), L'x');
            KTEST_EXPECT(e == 0, "Wcsset_s_OK");
            KTEST_EXPECT(wcscmp(buf, L"xxxxx") == 0, "Wcsset_s_Fill");
        }

        // memcpy_s — safe memcpy
        {
            char dst[16] = {};
            const char src[] = "hello";
            errno_t e = memcpy_s(dst, sizeof(dst), src, sizeof(src));
            KTEST_EXPECT(e == 0, "Memcpy_s_OK");
            KTEST_EXPECT(strcmp(dst, "hello") == 0, "Memcpy_s_Copy");
        }

        // wcstok_s — safe wide tokenize
        {
            wchar_t buf[] = L"a b c";
            wchar_t* ctx = nullptr;
            wchar_t* tok1 = wcstok_s(buf, L" ", &ctx);
            KTEST_EXPECT(tok1 != nullptr && wcscmp(tok1, L"a") == 0, "Wcstok_s_Token1");
            wchar_t* tok2 = wcstok_s(nullptr, L" ", &ctx);
            KTEST_EXPECT(tok2 != nullptr && wcscmp(tok2, L"b") == 0, "Wcstok_s_Token2");
            wchar_t* tok3 = wcstok_s(nullptr, L" ", &ctx);
            KTEST_EXPECT(tok3 != nullptr && wcscmp(tok3, L"c") == 0, "Wcstok_s_Token3");
            wchar_t* tok4 = wcstok_s(nullptr, L" ", &ctx);
            KTEST_EXPECT(tok4 == nullptr, "Wcstok_s_End");
        }

        // wcstok — wide tokenize with thread-local context
        {
            wchar_t buf[] = L"a,b";
            wchar_t* ctx = nullptr;
            wchar_t* tok1 = wcstok(buf, L",", &ctx);
            KTEST_EXPECT(tok1 != nullptr && wcscmp(tok1, L"a") == 0, "Wcstok_Token1");
            wchar_t* tok2 = wcstok(nullptr, L",", &ctx);
            KTEST_EXPECT(tok2 != nullptr && wcscmp(tok2, L"b") == 0, "Wcstok_Token2");
            wchar_t* tok3 = wcstok(nullptr, L",", &ctx);
            KTEST_EXPECT(tok3 == nullptr, "Wcstok_End");
        }

        // strcat_s — safe append
        {
            char buf[16] = "a";
            errno_t e = strcat_s(buf, sizeof(buf), "bc");
            KTEST_EXPECT(e == 0, "Strcat_s_OK");
            KTEST_EXPECT(strcmp(buf, "abc") == 0, "Strcat_s_Result");
        }

        // strcpy_s — safe copy
        {
            char buf[16] = {};
            errno_t e = strcpy_s(buf, sizeof(buf), "hello");
            KTEST_EXPECT(e == 0, "Strcpy_s_OK");
            KTEST_EXPECT(strcmp(buf, "hello") == 0, "Strcpy_s_Result");
        }

        // wcscat_s — safe wide append
        {
            wchar_t buf[16] = L"a";
            errno_t e = wcscat_s(buf, sizeof(buf)/sizeof(wchar_t), L"bc");
            KTEST_EXPECT(e == 0, "Wcscat_s_OK");
            KTEST_EXPECT(wcscmp(buf, L"abc") == 0, "Wcscat_s_Result");
        }

        // wcscpy_s — safe wide copy
        {
            wchar_t buf[16] = {};
            errno_t e = wcscpy_s(buf, sizeof(buf)/sizeof(wchar_t), L"hello");
            KTEST_EXPECT(e == 0, "Wcscpy_s_OK");
            KTEST_EXPECT(wcscmp(buf, L"hello") == 0, "Wcscpy_s_Result");
        }

        // wmemcpy_s — safe wide memcpy
        {
            wchar_t dst[16] = {};
            const wchar_t src[] = L"hello";
            errno_t e = wmemcpy_s(dst, sizeof(dst)/sizeof(wchar_t), src, sizeof(src)/sizeof(wchar_t));
            KTEST_EXPECT(e == 0, "Wmemcpy_s_OK");
            KTEST_EXPECT(wcscmp(dst, L"hello") == 0, "Wmemcpy_s_Result");
        }

        // wmemmove_s — safe wide memmove
        {
            wchar_t buf[16] = L"abc";
            errno_t e = wmemmove_s(buf + 1, (sizeof(buf)/sizeof(wchar_t)) - 1, buf, 3);
            KTEST_EXPECT(e == 0, "Wmemmove_s_OK");
            KTEST_EXPECT(wcscmp(buf, L"aabc") == 0, "Wmemmove_s_Result");
        }

        // strncat_s — safe append n chars
        {
            char buf[16] = "a";
            errno_t e = strncat_s(buf, sizeof(buf), "bcdef", 2);
            KTEST_EXPECT(e == 0, "Strncat_s_OK");
            KTEST_EXPECT(strcmp(buf, "abc") == 0, "Strncat_s_Result");
        }

        // strncpy_s — safe copy n chars
        {
            char buf[16] = {};
            errno_t e = strncpy_s(buf, sizeof(buf), "hello", 2);
            KTEST_EXPECT(e == 0, "Strncpy_s_OK");
            KTEST_EXPECT(strcmp(buf, "he") == 0, "Strncpy_s_Result");
        }

        // wcsncat_s — safe wide append n chars
        {
            wchar_t buf[16] = L"a";
            errno_t e = wcsncat_s(buf, sizeof(buf)/sizeof(wchar_t), L"bcdef", 2);
            KTEST_EXPECT(e == 0, "Wcsncat_s_OK");
            KTEST_EXPECT(wcscmp(buf, L"abc") == 0, "Wcsncat_s_Result");
        }

        // wcsncpy_s — safe wide copy n chars
        {
            wchar_t buf[16] = {};
            errno_t e = wcsncpy_s(buf, sizeof(buf)/sizeof(wchar_t), L"hello", 2);
            KTEST_EXPECT(e == 0, "Wcsncpy_s_OK");
            KTEST_EXPECT(wcscmp(buf, L"he") == 0, "Wcsncpy_s_Result");
        }

        // std::filesystem
        {
            namespace fs = std::filesystem;

            // --- Path operations ---
            KTEST_EXPECT(fs::path(L"C:\\Temp\\test.txt").wstring() == L"C:\\Temp\\test.txt", "FsPath_Construct");
            KTEST_EXPECT((fs::path(L"C:\\Temp") / L"subdir").wstring() == L"C:\\Temp\\subdir", "FsPath_Concat");
            {
                fs::path p(L"C:\\Temp\\subdir\\file.txt");
                KTEST_EXPECT(p.parent_path().wstring() == L"C:\\Temp\\subdir", "FsPath_Parent");
                KTEST_EXPECT(p.filename().wstring() == L"file.txt", "FsPath_Filename");
                KTEST_EXPECT(p.stem().wstring() == L"file", "FsPath_Stem");
                KTEST_EXPECT(p.extension().wstring() == L".txt", "FsPath_Extension");
            }
            KTEST_EXPECT(fs::path(L"C:\\Temp").is_absolute(), "FsPath_IsAbsolute");
            KTEST_EXPECT(fs::path(L"relative").is_relative(), "FsPath_IsRelative");
            {
                fs::path p(L"C:\\Temp\\file.txt");
                p.replace_extension(L".log");
                KTEST_EXPECT(p.extension().wstring() == L".log", "FsPath_ReplaceExt");
            }
            KTEST_EXPECT(fs::path(L"a") == fs::path(L"a"), "FsPath_Eq");
            KTEST_EXPECT(fs::path(L"a") != fs::path(L"b"), "FsPath_Neq");
            KTEST_EXPECT(fs::path().empty(), "FsPath_Empty");
            {
                fs::path p(L"a");
                p.clear();
                KTEST_EXPECT(p.empty(), "FsPath_Clear");
            }
            // root_name / root_path
            {
                fs::path p(L"C:\\Temp\\file.txt");
                KTEST_EXPECT(p.root_name().wstring() == L"C:", "FsPath_RootName");
                KTEST_EXPECT(p.root_path().wstring() == L"C:\\", "FsPath_RootPath");
                KTEST_EXPECT(p.relative_path().wstring() == L"Temp\\file.txt", "FsPath_RelativePath");
            }
            // has_extension / has_filename / has_parent_path
            {
                fs::path p(L"C:\\Temp\\file.txt");
                KTEST_EXPECT(p.has_extension(), "FsPath_HasExtension");
                KTEST_EXPECT(p.has_filename(), "FsPath_HasFilename");
                KTEST_EXPECT(p.has_parent_path(), "FsPath_HasParent");
                fs::path nf(L"C:\\");
                KTEST_EXPECT(!nf.has_filename(), "FsPath_NoFilename");
            }
            // concat with +=
            {
                fs::path p(L"C:\\Temp");
                p += L"\\sub";
                KTEST_EXPECT(p.wstring() == L"C:\\Temp\\sub", "FsPath_Append");
            }
            // remove_filename
            {
                fs::path p(L"C:\\Temp\\file.txt");
                p.remove_filename();
                KTEST_EXPECT(p.wstring() == L"C:\\Temp\\", "FsPath_RemoveFilename");
            }

            // --- Filesystem operations ---
            fs::path tmpdir = fs::temp_directory_path() / L"MusaFsTest";

            // Clean up from previous runs
            fs::remove_all(tmpdir);

            // create_directory / exists / is_directory
            KTEST_EXPECT(fs::create_directory(tmpdir), "FsCreateDir");
            KTEST_EXPECT(fs::exists(tmpdir), "FsExists_Dir");
            KTEST_EXPECT(fs::is_directory(tmpdir), "FsIsDir");
            KTEST_EXPECT(!fs::is_regular_file(tmpdir), "FsIsDir_NotFile");
            // create_directory on existing dir succeeds
            KTEST_EXPECT(fs::create_directory(tmpdir), "FsCreateDir_Existing");

            // Create a test file via C fopen
            fs::path testfile = tmpdir / L"test.txt";
            {
                FILE* f = _wfopen(testfile.c_str(), L"w");
                KTEST_EXPECT(f != nullptr, "FsFopen");
                if (f) {
                    const char* msg = "Hello, filesystem!";
                    size_t written = fwrite(msg, 1, 19, f);
                    KTEST_EXPECT(written == 19, "FsFwrite");
                    fclose(f);
                }
            }

            // exists / is_regular_file
            KTEST_EXPECT(fs::exists(testfile), "FsExists_File");
            KTEST_EXPECT(fs::is_regular_file(testfile), "FsIsFile");
            KTEST_EXPECT(!fs::is_directory(testfile), "FsIsFile_NotDir");

            // file_size
            KTEST_EXPECT(fs::file_size(testfile) == 19, "FsFileSize");

            // status / permissions
            {
                std::error_code ec;
                fs::file_status st = fs::status(testfile, ec);
                KTEST_EXPECT(!ec, "FsStatus_NoError");
                KTEST_EXPECT(fs::is_regular_file(st), "FsStatus_IsFile");
            }

            // last_write_time
            {
                std::error_code ec;
                auto t = fs::last_write_time(testfile, ec);
                KTEST_EXPECT(!ec, "FsLastWriteTime");
                KTEST_EXPECT(t.time_since_epoch().count() > 0, "FsLastWriteTime_NonZero");
            }

            // copy_file
            fs::path copyfile = tmpdir / L"copy.txt";
            KTEST_EXPECT(fs::copy_file(testfile, copyfile), "FsCopyFile");
            KTEST_EXPECT(fs::exists(copyfile), "FsCopyFile_Exists");
            KTEST_EXPECT(fs::file_size(copyfile) == 19, "FsCopyFile_Size");

            // equivalent
            KTEST_EXPECT(fs::equivalent(testfile, copyfile), "FsEquivalent");
            // non-equivalent
            fs::path diff = tmpdir / L"different.txt";
            {
                FILE* f = _wfopen(diff.c_str(), L"w");
                if (f) { fwrite("x", 1, 1, f); fclose(f); }
            }
            KTEST_EXPECT(!fs::equivalent(testfile, diff), "FsNotEquivalent");
            fs::remove(diff);

            // rename
            fs::path renamed = tmpdir / L"renamed.txt";
            fs::rename(testfile, renamed);
            KTEST_EXPECT(!fs::exists(testfile), "FsRename_OldGone");
            KTEST_EXPECT(fs::exists(renamed), "FsRename_NewExists");

            // remove (single file)
            KTEST_EXPECT(fs::remove(copyfile), "FsRemove_File");
            KTEST_EXPECT(!fs::exists(copyfile), "FsRemove_FileGone");

            // create subdirectory
            fs::path subdir = tmpdir / L"sub";
            KTEST_EXPECT(fs::create_directory(subdir), "FsCreateSubDir");
            KTEST_EXPECT(fs::is_directory(subdir), "FsIsSubDir");

            // directory_iterator
            {
                size_t count = 0;
                bool found_sub = false, found_renamed = false;
                for (auto& entry : fs::directory_iterator(tmpdir)) {
                    KTEST_EXPECT(!entry.path().empty(), "FsDirIter_Path");
                    if (entry.is_directory()) found_sub = true;
                    if (entry.is_regular_file()) found_renamed = true;
                    ++count;
                }
                KTEST_EXPECT(count >= 2, "FsDirIter_Count");
                KTEST_EXPECT(found_sub, "FsDirIter_FoundDir");
                KTEST_EXPECT(found_renamed, "FsDirIter_FoundFile");
            }

            // recursive_directory_iterator
            {
                size_t count = 0;
                for (auto& entry : fs::recursive_directory_iterator(tmpdir)) {
                    KTEST_EXPECT(!entry.path().empty(), "FsRecDirIter_Path");
                    ++count;
                }
                KTEST_EXPECT(count >= 2, "FsRecDirIter_Count");
            }

            // space
            {
                std::error_code ec;
                auto si = fs::space(tmpdir, ec);
                KTEST_EXPECT(!ec, "FsSpace_NoError");
                KTEST_EXPECT(si.capacity > 0, "FsSpace_Capacity");
                KTEST_EXPECT(si.free <= si.capacity, "FsSpace_Free");
            }

            // current_path
            {
                std::error_code ec;
                fs::path cp = fs::current_path(ec);
                KTEST_EXPECT(!ec, "FsCurrentPath_NoError");
                KTEST_EXPECT(cp.is_absolute(), "FsCurrentPath_Absolute");
            }

            // hard_link_count
            {
                KTEST_EXPECT(fs::hard_link_count(renamed) >= 1, "FsHardLinkCount");
            }

            // remove_all (recursive)
            KTEST_EXPECT(fs::remove_all(tmpdir) >= 3, "FsRemoveAll");
            KTEST_EXPECT(!fs::exists(tmpdir), "FsRemoveAll_Gone");
        }
        }



        // Results
        MusaLOG("=== Results: %lu/%lu passed ===", TestsRun - TestsFailed, TestsRun);
        if (TestsFailed > 0) {
            MusaLOG("%lu test(s) FAILED", TestsFailed);
        }
    }

    static HANDLE g_TestThread = nullptr;

    static VOID NTAPI TestThreadRoutine(_In_ PVOID)
    {
        MusaLOG("Test thread started.");

        KeExpandKernelStackAndCalloutEx([](PVOID)
        {
            RunTests();
        }, nullptr, MAXIMUM_EXPANSION_SIZE, TRUE, nullptr);

        MusaLOG("Test thread completed.");
        PsTerminateSystemThread(STATUS_SUCCESS);
    }

    EXTERN_C VOID DriverUnload(_In_ PDRIVER_OBJECT DriverObject)
    {
        UNREFERENCED_PARAMETER(DriverObject);
        MusaLOG("Unloading...");

        if (g_TestThread) {
            MusaLOG("Waiting for test thread to complete...");
            ZwWaitForSingleObject(g_TestThread, FALSE, nullptr);
            ZwClose(g_TestThread);
            g_TestThread = nullptr;
        }

        MusaLOG("Exit.");
    }

    EXTERN_C NTSTATUS DriverMain(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath)
    {
        UNREFERENCED_PARAMETER(RegistryPath);

        MusaLOG("Enter.");
        DriverObject->DriverUnload = DriverUnload;

        NTSTATUS status = PsCreateSystemThread(
            &g_TestThread,
            THREAD_ALL_ACCESS,
            nullptr,
            nullptr,
            nullptr,
            TestThreadRoutine,
            nullptr
        );

        if (!NT_SUCCESS(status)) {
            MusaLOG("Failed to create test thread: 0x%08X", status);
        } else {
            MusaLOG("Test thread created successfully.");
        }

        return STATUS_SUCCESS;
    }
}
