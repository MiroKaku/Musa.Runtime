#ifdef TEST_HAS_CONTAINERS_QUEUE

#include <queue>

#include "LLVM.TestSuite/test_macros.h"
#include "LLVM.TestSuite/test_allocator.h"
#include "LLVM.TestSuite/test_convertible.h"
#include "LLVM.TestSuite/MoveOnly.h"
#include "LLVM.TestSuite/deduction_guides_sfinae_checks.h"


namespace
{
    template <class C>
    C make(int n)
    {
        C c;
        for (int i = 0; i < n; ++i)
            c.push_back(i);
        return c;
    }

    template <class C>
    C make_move_only(int n)
    {
        C c;
        for (int i = 0; i < n; ++i)
            c.push_back(MoveOnly(i));
        return c;
    }

    template <class C>
    C make_with_push(int n)
    {
        C c;
        for (int i = 0; i < n; ++i)
            c.push(i);
        return c;
    }

    class Emplaceable
    {
        Emplaceable(const Emplaceable&);
        Emplaceable& operator=(const Emplaceable&);

        int int_;
        double double_;
    public:
        Emplaceable() : int_(0), double_(0)
        {}
        Emplaceable(int i, double d) : int_(i), double_(d)
        {}
        Emplaceable(Emplaceable&& x)
            : int_(x.int_), double_(x.double_)
        {
            x.int_ = 0; x.double_ = 0;
        }
        Emplaceable& operator=(Emplaceable&& x)
        {
            int_ = x.int_; x.int_ = 0;
            double_ = x.double_; x.double_ = 0;
            return *this;
        }

        bool operator==(const Emplaceable& x) const
        {
            return int_ == x.int_ && double_ == x.double_;
        }
        bool operator<(const Emplaceable& x) const
        {
            return int_ < x.int_ || (int_ == x.int_ && double_ < x.double_);
        }

        int get() const
        {
            return int_;
        }
    };
}

namespace
{
    struct CtorAllocTest
        : private std::queue<int, std::deque<int, test_allocator<int> > >
    {
        typedef std::queue<int, std::deque<int, test_allocator<int> > > base;

        explicit CtorAllocTest(const test_allocator<int>& a) : base(a)
        {}
        CtorAllocTest(const container_type& container, const test_allocator<int>& a) : base(container, a)
        {}
    #if TEST_STD_VER >= 11
        CtorAllocTest(container_type&& container, const test_allocator<int>& a) : base(std::move(container), a)
        {}
        CtorAllocTest(CtorAllocTest&& q, const test_allocator<int>& a) : base(std::move(q), a)
        {}
    #endif
        test_allocator<int> get_allocator()
        {
            return c.get_allocator();
        }
    };
}

TEST(ctor_alloc)
{
    CtorAllocTest q(test_allocator<int>(3));
    assert(q.get_allocator() == test_allocator<int>(3));
}

namespace
{
    struct CtorContainerAllocTest
        : public std::queue<int, std::deque<int, test_allocator<int>>>
    {
        typedef std::queue<int, std::deque<int, test_allocator<int>>> base;

        explicit CtorContainerAllocTest(const test_allocator<int>& a) : base(a)
        {}
        CtorContainerAllocTest(const container_type& container, const test_allocator<int>& a) : base(container, a)
        {}
    #if TEST_STD_VER >= 11
        CtorContainerAllocTest(container_type&& container, const test_allocator<int>& a) : base(std::move(container), a)
        {}
        CtorContainerAllocTest(CtorContainerAllocTest&& q, const test_allocator<int>& a) : base(std::move(q), a)
        {}
    #endif
        test_allocator<int> get_allocator()
        {
            return c.get_allocator();
        }
    };

}

TEST(ctor_container_alloc)
{
    auto d = make<std::deque<int, test_allocator<int>>>(5);
    CtorContainerAllocTest q(d, test_allocator<int>(4));
    assert(q.get_allocator() == test_allocator<int>(4));
    assert(q.size() == 5);
    for (std::deque<int, test_allocator<int>>::size_type i = 0; i < d.size(); ++i) {
        assert(q.front() == d[i]);
        q.pop();
    }
}

namespace
{
    typedef std::deque<int, test_allocator<int> > C;

    template <class T>
    struct CtorQueueAllocTest
        : public std::queue<T, C>
    {
        typedef std::queue<T, C> base;
        typedef test_allocator<int>      allocator_type;
        typedef typename base::container_type container_type;

        explicit CtorQueueAllocTest(const allocator_type& a) : base(a)
        {}
        CtorQueueAllocTest(const container_type& container, const allocator_type& a) : base(container, a)
        {}
        CtorQueueAllocTest(const CtorQueueAllocTest& q, const allocator_type& a) : base(q, a)
        {}
        allocator_type get_allocator()
        {
            return this->c.get_allocator();
        }
    };
}

TEST(ctor_queue_alloc)
{
    CtorQueueAllocTest<int> q(make<C>(5), test_allocator<int>(4));
    CtorQueueAllocTest<int> q2(q, test_allocator<int>(5));
    assert(q2.get_allocator() == test_allocator<int>(5));
    assert(q2.size() == 5);
}

namespace
{
    template <class T>
    struct CtorRcontainerAllocTest
        : public std::queue<T, std::deque<MoveOnly, test_allocator<MoveOnly>>>
    {
        typedef std::queue<T, std::deque<MoveOnly, test_allocator<MoveOnly>>> base;
        typedef test_allocator<MoveOnly>      allocator_type;
        typedef typename base::container_type container_type;

        explicit CtorRcontainerAllocTest(const allocator_type& a) : base(a)
        {}
        CtorRcontainerAllocTest(const container_type& container, const allocator_type& a) : base(container, a)
        {}
        CtorRcontainerAllocTest(container_type&& container, const allocator_type& a) : base(std::move(container), a)
        {}
        CtorRcontainerAllocTest(CtorRcontainerAllocTest&& q, const allocator_type& a) : base(std::move(q), a)
        {}
        allocator_type get_allocator()
        {
            return this->c.get_allocator();
        }
    };
}

TEST(ctor_rcontainer_alloc)
{
    CtorRcontainerAllocTest<MoveOnly> q(make_move_only<std::deque<MoveOnly, test_allocator<MoveOnly>>>(5), test_allocator<MoveOnly>(4));
    assert(q.get_allocator() == test_allocator<MoveOnly>(4));
    assert(q.size() == 5);
}

namespace
{
    template <class T>
    struct CtorRqueueAllocTest
        : public std::queue<T, std::deque<MoveOnly, test_allocator<MoveOnly>>>
    {
        typedef std::queue<T, std::deque<MoveOnly, test_allocator<MoveOnly>>> base;
        typedef test_allocator<MoveOnly>      allocator_type;
        typedef typename base::container_type container_type;

        explicit CtorRqueueAllocTest(const allocator_type& a) : base(a)
        {}
        CtorRqueueAllocTest(const container_type& container, const allocator_type& a) : base(container, a)
        {}
        CtorRqueueAllocTest(container_type&& container, const allocator_type& a) : base(std::move(container), a)
        {}
        CtorRqueueAllocTest(CtorRqueueAllocTest&& q, const allocator_type& a) : base(std::move(q), a)
        {}
        allocator_type get_allocator()
        {
            return this->c.get_allocator();
        }
    };

}

TEST(ctor_rqueue_alloc)
{
    CtorRqueueAllocTest<MoveOnly> q(make<std::deque<MoveOnly, test_allocator<MoveOnly>>>(5), test_allocator<MoveOnly>(4));
    CtorRqueueAllocTest<MoveOnly> q2(std::move(q), test_allocator<MoveOnly>(5));
    assert(q2.get_allocator() == test_allocator<MoveOnly>(5));
    assert(q2.size() == 5);
}

TEST(ctor_container)
{
    typedef std::deque<int> Container;
    typedef std::queue<int> Q;
    Container d = make<Container>(5);
    Q q(d);
    assert(q.size() == 5);
    for (std::size_t i = 0; i < d.size(); ++i) {
        assert(q.front() == d[i]);
        q.pop();
    }

#if TEST_STD_VER >= 11
    static_assert(!test_convertible<Q, const Container&>(), "");
#endif
}

TEST(ctor_copy)
{
    std::queue<int> q(make<std::deque<int> >(5));
    std::queue<int> q2 = q;
    assert(q2 == q);
}

TEST(ctor_move)
{
    std::queue<MoveOnly> q(make_move_only<std::deque<MoveOnly> >(5));
    std::queue<MoveOnly> q2 = std::move(q);
    assert(q2.size() == 5);
    assert(q.empty());
}

TEST(ctor_rcontainer)
{
    typedef std::deque<MoveOnly> Container;
    typedef std::queue<MoveOnly> Q;
    Q q(make_move_only<std::deque<MoveOnly> >(5));
    assert(q.size() == 5);

#if TEST_STD_VER >= 11
    static_assert(!test_convertible<Q, Container&&>(), "");
#endif
}

TEST(assign_copy)
{
    std::queue<int> q(make<std::deque<int> >(5));
    std::queue<int> q2;
    q2 = q;
    assert(q2 == q);
}

TEST(assign_move)
{
    std::queue<MoveOnly> q(make_move_only<std::deque<MoveOnly> >(5));
    std::queue<MoveOnly> q2;
    q2 = std::move(q);
    assert(q2.size() == 5);
    assert(q.empty());
}

TEST(back)
{
    std::queue<int> q;
    assert(q.size() == 0);
    q.push(1);
    q.push(2);
    q.push(3);
    int& ir = q.back();
    assert(ir == 3);
}

TEST(back_const)
{
    std::queue<int> q;
    assert(q.size() == 0);
    q.push(1);
    q.push(2);
    q.push(3);
    const std::queue<int>& cqr = q;
    const int& cir = cqr.back();
    assert(cir == 3);
}

namespace
{
    template <typename Queue>
    void test_emplace_return_type()
    {
        typedef typename Queue::container_type Container;
        typedef typename Container::value_type value_type;
        typedef decltype(std::declval<Queue>().emplace(std::declval<value_type&>())) queue_return_type;

    #if TEST_STD_VER > 14
        typedef decltype(std::declval<Container>().emplace_back(std::declval<value_type>())) container_return_type;
        static_assert(std::is_same<queue_return_type, container_return_type>::value, "");
    #else
        static_assert(std::is_same<queue_return_type, void>::value, "");
    #endif
    }
}

TEST(emplace)
{
    test_emplace_return_type<std::queue<int> >();
    test_emplace_return_type<std::queue<int, std::list<int> > >();

    std::queue<Emplaceable> q;
#if TEST_STD_VER > 14
    typedef Emplaceable T;
    T& r1 = q.emplace(1, 2.5);
    assert(&r1 == &q.back());
    T& r2 = q.emplace(2, 3.5);
    assert(&r2 == &q.back());
    T& r3 = q.emplace(3, 4.5);
    assert(&r3 == &q.back());
    assert(&r1 == &q.front());
#else
    q.emplace(1, 2.5);
    q.emplace(2, 3.5);
    q.emplace(3, 4.5);
#endif

    assert(q.size() == 3);
    assert(q.front() == Emplaceable(1, 2.5));
    assert(q.back() == Emplaceable(3, 4.5));
}

TEST(empty)
{
    std::queue<int> q;
    assert(q.empty());
    q.push(1);
    assert(!q.empty());
    q.pop();
    assert(q.empty());
}

TEST(front)
{
    std::queue<int> q;
    assert(q.size() == 0);
    q.push(1);
    q.push(2);
    q.push(3);
    int& ir = q.front();
    assert(ir == 1);
}

TEST(front_const)
{
    std::queue<int> q;
    assert(q.size() == 0);
    q.push(1);
    q.push(2);
    q.push(3);
    const std::queue<int>& cqr = q;
    const int& cir = cqr.front();
    assert(cir == 1);
}

TEST(pop)
{
    std::queue<int> q;
    assert(q.size() == 0);
    q.push(1);
    q.push(2);
    q.push(3);
    assert(q.size() == 3);
    assert(q.front() == 1);
    assert(q.back() == 3);
    q.pop();
    assert(q.size() == 2);
    assert(q.front() == 2);
    assert(q.back() == 3);
    q.pop();
    assert(q.size() == 1);
    assert(q.front() == 3);
    assert(q.back() == 3);
    q.pop();
    assert(q.size() == 0);
}

TEST(push)
{
    std::queue<int> q;
    q.push(1);
    assert(q.size() == 1);
    assert(q.front() == 1);
    assert(q.back() == 1);
    q.push(2);
    assert(q.size() == 2);
    assert(q.front() == 1);
    assert(q.back() == 2);
    q.push(3);
    assert(q.size() == 3);
    assert(q.front() == 1);
    assert(q.back() == 3);
}

TEST(push_rv)
{
    std::queue<MoveOnly> q;
    q.push(MoveOnly(1));
    assert(q.size() == 1);
    assert(q.front() == MoveOnly(1));
    assert(q.back() == MoveOnly(1));
    q.push(MoveOnly(2));
    assert(q.size() == 2);
    assert(q.front() == MoveOnly(1));
    assert(q.back() == MoveOnly(2));
    q.push(MoveOnly(3));
    assert(q.size() == 3);
    assert(q.front() == MoveOnly(1));
    assert(q.back() == MoveOnly(3));
}

TEST(size)
{
    std::queue<int> q;
    assert(q.size() == 0);
    q.push(1);
    assert(q.size() == 1);
}

TEST(swap)
{
    std::queue<int> q1 = make_with_push<std::queue<int> >(5);
    std::queue<int> q2 = make_with_push<std::queue<int> >(10);
    std::queue<int> q1_save = q1;
    std::queue<int> q2_save = q2;
    q1.swap(q2);
    assert(q1 == q2_save);
    assert(q2 == q1_save);
}

TEST(eq)
{
    std::queue<int> q1 = make_with_push<std::queue<int> >(5);
    std::queue<int> q2 = make_with_push<std::queue<int> >(10);
    std::queue<int> q1_save = q1;
    std::queue<int> q2_save = q2;
    assert(q1 == q1_save);
    assert(q1 != q2);
    assert(q2 == q2_save);
}

TEST(lt)
{
    std::queue<int> q1 = make_with_push<std::queue<int> >(5);
    std::queue<int> q2 = make_with_push<std::queue<int> >(10);
    assert(q1 < q2);
    assert(q2 > q1);
    assert(q1 <= q2);
    assert(q2 >= q1);
}

#endif
