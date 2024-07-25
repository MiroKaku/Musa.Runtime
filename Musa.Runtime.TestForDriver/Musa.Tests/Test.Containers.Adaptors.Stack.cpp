#ifdef TEST_HAS_CONTAINERS_STACK

#include <stack>
#include <cassert>

#include "LLVM.TestSuite/test_macros.h"
#include "LLVM.TestSuite/test_allocator.h"
#include "LLVM.TestSuite/test_convertible.h"
#include "LLVM.TestSuite/MoveOnly.h"


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
        : private std::stack<int, std::deque<int, test_allocator<int> > >
    {
        typedef std::stack<int, std::deque<int, test_allocator<int> > > base;

        explicit CtorAllocTest(const test_allocator<int>& a) : base(a)
        {}
        CtorAllocTest(const container_type& cont, const test_allocator<int>& a) : base(cont, a)
        {}
    #if TEST_STD_VER >= 11
        CtorAllocTest(container_type&& cont, const test_allocator<int>& a) : base(std::move(cont), a)
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
        : public std::stack<int, std::deque<int, test_allocator<int> >>
    {
        typedef std::stack<int, std::deque<int, test_allocator<int> >> base;

        explicit CtorContainerAllocTest(const test_allocator<int>& a) : base(a)
        {}
        CtorContainerAllocTest(const container_type& cont, const test_allocator<int>& a) : base(cont, a)
        {}
    #if TEST_STD_VER >= 11
        CtorContainerAllocTest(container_type&& cont, const test_allocator<int>& a) : base(std::move(cont), a)
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
    auto d = make<std::deque<int, test_allocator<int> >>(5);
    CtorContainerAllocTest q(d, test_allocator<int>(4));
    assert(q.get_allocator() == test_allocator<int>(4));
    assert(q.size() == 5);
    for (std::deque<int, test_allocator<int> >::size_type i = 0; i < d.size(); ++i) {
        assert(q.top() == d[d.size() - i - 1]);
        q.pop();
    }
}

namespace
{
    template <class T>
    struct CtorCopyAllocTest
        : public std::stack<T, std::deque<int, test_allocator<int> >>
    {
        typedef std::stack<T, std::deque<int, test_allocator<int> >> base;
        typedef test_allocator<int>      allocator_type;
        typedef typename base::container_type container_type;

        explicit CtorCopyAllocTest(const allocator_type& a) : base(a)
        {}
        CtorCopyAllocTest(const container_type& cont, const allocator_type& a) : base(cont, a)
        {}
        CtorCopyAllocTest(const CtorCopyAllocTest& q, const allocator_type& a) : base(q, a)
        {}
        allocator_type get_allocator()
        {
            return this->c.get_allocator();
        }
    };
}

TEST(ctor_copy_alloc)
{
    CtorCopyAllocTest<int> q(make<std::deque<int, test_allocator<int> >>(5), test_allocator<int>(4));
    CtorCopyAllocTest<int> q2(q, test_allocator<int>(5));
    assert(q2.get_allocator() == test_allocator<int>(5));
    assert(q2.size() == 5);
}

namespace
{
    template <class T>
    struct CtorRcontainerAllocTest
        : public std::stack<T, std::deque<MoveOnly, test_allocator<MoveOnly> >>
    {
        typedef std::stack<T, std::deque<MoveOnly, test_allocator<MoveOnly> >> base;
        typedef test_allocator<MoveOnly>      allocator_type;
        typedef typename base::container_type container_type;

        explicit CtorRcontainerAllocTest(const allocator_type& a) : base(a)
        {}
        CtorRcontainerAllocTest(const container_type& cont, const allocator_type& a) : base(cont, a)
        {}
        CtorRcontainerAllocTest(container_type&& cont, const allocator_type& a) : base(std::move(cont), a)
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
    CtorRcontainerAllocTest<MoveOnly> q(make_move_only<std::deque<MoveOnly, test_allocator<MoveOnly> >>(5), test_allocator<MoveOnly>(4));
    assert(q.get_allocator() == test_allocator<MoveOnly>(4));
    assert(q.size() == 5);
}

namespace
{
    template <class T>
    struct CtorRqueueAllocTest
        : public std::stack<T, std::deque<MoveOnly, test_allocator<MoveOnly> >>
    {
        typedef std::stack<T, std::deque<MoveOnly, test_allocator<MoveOnly> >> base;
        typedef test_allocator<MoveOnly>      allocator_type;
        typedef typename base::container_type container_type;

        explicit CtorRqueueAllocTest(const allocator_type& a) : base(a)
        {}
        CtorRqueueAllocTest(const container_type& cont, const allocator_type& a) : base(cont, a)
        {}
        CtorRqueueAllocTest(container_type&& cont, const allocator_type& a) : base(std::move(cont), a)
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
    CtorRqueueAllocTest<MoveOnly> q(make<std::deque<MoveOnly, test_allocator<MoveOnly> >>(5), test_allocator<MoveOnly>(4));
    CtorRqueueAllocTest<MoveOnly> q2(std::move(q), test_allocator<MoveOnly>(5));
    assert(q2.get_allocator() == test_allocator<MoveOnly>(5));
    assert(q2.size() == 5);
}

TEST(ctor_container)
{
    typedef std::deque<int> Container;
    typedef std::stack<int> Q;
    Container d = make<Container>(5);
    Q q(d);
    assert(q.size() == 5);
    for (std::size_t i = 0; i < d.size(); ++i) {
        assert(q.top() == d[d.size() - i - 1]);
        q.pop();
    }

#if TEST_STD_VER >= 11
    static_assert(!test_convertible<Q, const Container&>(), "");
#endif
}

TEST(ctor_copy)
{
    std::stack<int> q(make<std::deque<int> >(5));
    std::stack<int> q2 = q;
    assert(q2 == q);
}

TEST(ctor_move)
{
    std::stack<MoveOnly> q(make_move_only<std::deque<MoveOnly> >(5));
    std::stack<MoveOnly> q2 = std::move(q);
    assert(q2.size() == 5);
    assert(q.empty());
}

TEST(ctor_rcontainer)
{
    typedef std::deque<MoveOnly> Container;
    typedef std::stack<MoveOnly> Q;
    Q q(make_move_only<Container>(5));
    assert(q.size() == 5);

#if TEST_STD_VER >= 11
    static_assert(!test_convertible<Q, Container&&>(), "");
#endif
}

TEST(assign_copy)
{
    std::stack<int> q(make<std::deque<int> >(5));
    std::stack<int> q2;
    q2 = q;
    assert(q2 == q);
}

TEST(assign_move)
{
    std::stack<MoveOnly> q(make_move_only<std::deque<MoveOnly> >(5));
    std::stack<MoveOnly> q2;
    q2 = std::move(q);
    assert(q2.size() == 5);
    assert(q.empty());
}

namespace
{
    template <typename Stack>
    void test_emplace_return_type()
    {
        typedef typename Stack::container_type Container;
        typedef typename Container::value_type value_type;
        typedef decltype(std::declval<Stack>().emplace(std::declval<value_type&>()))     stack_return_type;

    #if TEST_STD_VER > 14
        typedef decltype(std::declval<Container>().emplace_back(std::declval<value_type>())) container_return_type;
        static_assert(std::is_same<stack_return_type, container_return_type>::value, "");
    #else
        static_assert(std::is_same<stack_return_type, void>::value, "");
    #endif
    }

}
TEST(emplace)
{
    test_emplace_return_type<std::stack<int> >();
    test_emplace_return_type<std::stack<int, std::vector<int> > >();

    std::stack<Emplaceable> q;
#if TEST_STD_VER > 14
    typedef Emplaceable T;
    T& r1 = q.emplace(1, 2.5);
    assert(&r1 == &q.top());
    T& r2 = q.emplace(2, 3.5);
    assert(&r2 == &q.top());
    T& r3 = q.emplace(3, 4.5);
    assert(&r3 == &q.top());
#else
    q.emplace(1, 2.5);
    q.emplace(2, 3.5);
    q.emplace(3, 4.5);
#endif
    assert(q.size() == 3);
    assert(q.top() == Emplaceable(3, 4.5));
}

TEST(empty)
{
    std::stack<int> q;
    assert(q.empty());
    q.push(1);
    assert(!q.empty());
    q.pop();
    assert(q.empty());
}

TEST(pop)
{
    std::stack<int> q;
    assert(q.size() == 0);
    q.push(1);
    q.push(2);
    q.push(3);
    assert(q.size() == 3);
    assert(q.top() == 3);
    q.pop();
    assert(q.size() == 2);
    assert(q.top() == 2);
    q.pop();
    assert(q.size() == 1);
    assert(q.top() == 1);
    q.pop();
    assert(q.size() == 0);
}

TEST(push)
{
    std::stack<int> q;
    q.push(1);
    assert(q.size() == 1);
    assert(q.top() == 1);
    q.push(2);
    assert(q.size() == 2);
    assert(q.top() == 2);
    q.push(3);
    assert(q.size() == 3);
    assert(q.top() == 3);
}

TEST(size)
{
    std::stack<int> q;
    assert(q.size() == 0);
    q.push(1);
    assert(q.size() == 1);
}

TEST(swap)
{
    std::stack<int> q1 = make_with_push<std::stack<int> >(5);
    std::stack<int> q2 = make_with_push<std::stack<int> >(10);
    std::stack<int> q1_save = q1;
    std::stack<int> q2_save = q2;
    q1.swap(q2);
    assert(q1 == q2_save);
    assert(q2 == q1_save);
}

TEST(top)
{
    std::stack<int> q;
    assert(q.size() == 0);
    q.push(1);
    q.push(2);
    q.push(3);
    int& ir = q.top();
    assert(ir == 3);
}

TEST(top_const)
{
    std::stack<int> q;
    assert(q.size() == 0);
    q.push(1);
    q.push(2);
    q.push(3);
    const std::stack<int>& cqr = q;
    const int& cir = cqr.top();
    assert(cir == 3);
}

TEST(eq)
{
    std::stack<int> q1 = make_with_push<std::stack<int> >(5);
    std::stack<int> q2 = make_with_push<std::stack<int> >(10);
    std::stack<int> q1_save = q1;
    std::stack<int> q2_save = q2;
    assert(q1 == q1_save);
    assert(q1 != q2);
    assert(q2 == q2_save);
}

TEST(lt)
{
    std::stack<int> q1 = make_with_push<std::stack<int> >(5);
    std::stack<int> q2 = make_with_push<std::stack<int> >(10);
    assert(q1 < q2);
    assert(q2 > q1);
    assert(q1 <= q2);
    assert(q2 >= q1);
}


#endif
