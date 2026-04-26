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
#include <kmalloc.h>
#include <kallocator.h>

#include "Test.h"


namespace Main
{
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
