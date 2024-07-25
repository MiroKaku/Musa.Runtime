#ifdef TEST_HAS_CONTAINERS_PRIORITY_QUEUE

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

namespace std {

    template <>
    struct hash<Emplaceable>
    {
        typedef Emplaceable argument_type;
        typedef std::size_t result_type;

        std::size_t operator()(const Emplaceable& x) const
        {
            return x.get();
        }
    };

}

namespace
{
    template <class T>
    struct CtorAllocTest
        : public std::priority_queue<T, std::vector<T, test_allocator<T> > >
    {
        typedef std::priority_queue<T, std::vector<T, test_allocator<T> > > base;
        typedef typename base::container_type container_type;
        typedef typename base::value_compare value_compare;

        explicit CtorAllocTest(const test_allocator<int>& a) : base(a)
        {}
        CtorAllocTest(const value_compare& comp, const test_allocator<int>& a)
            : base(comp, c, a)
        {}
        CtorAllocTest(const value_compare& comp, const container_type& container,
            const test_allocator<int>& a) : base(comp, container, a)
        {}
    #if TEST_STD_VER >= 11
        CtorAllocTest(const value_compare& comp, container_type&& container,
            const test_allocator<int>& a) : base(comp, std::move(container), a)
        {}
        CtorAllocTest(CtorAllocTest&& q, const test_allocator<int>& a) : base(std::move(q), a)
        {}
    #endif
        test_allocator<int> get_allocator()
        {
            return c.get_allocator();
        }

        using base::c;
    };
}

TEST(ctor_alloc)
{
    CtorAllocTest<int> q((test_allocator<int>(3)));
    assert(q.c.get_allocator() == test_allocator<int>(3));
    assert(q.c.size() == 0);
}

namespace
{
    template <class T>
    struct CtorCompAllocTest
        : public std::priority_queue<T, std::vector<T, test_allocator<T> > >
    {
        typedef std::priority_queue<T, std::vector<T, test_allocator<T> > > base;
        typedef typename base::container_type container_type;
        typedef typename base::value_compare value_compare;

        explicit CtorCompAllocTest(const test_allocator<int>& a) : base(a)
        {}
        CtorCompAllocTest(const value_compare& compare, const test_allocator<int>& a)
            : base(compare, a)
        {}
        CtorCompAllocTest(const value_compare& compare, const container_type& container,
            const test_allocator<int>& a) : base(compare, container, a)
        {}
    #if TEST_STD_VER >= 11
        CtorCompAllocTest(const value_compare& compare, container_type&& container,
            const test_allocator<int>& a) : base(compare, std::move(container), a)
        {}
        CtorCompAllocTest(CtorCompAllocTest&& q, const test_allocator<int>& a) : base(std::move(q), a)
        {}
    #endif
        test_allocator<int> get_allocator()
        {
            return c.get_allocator();
        }

        using base::c;
    };
}

TEST(ctor_comp_alloc)
{
    CtorCompAllocTest<int> q(std::less<int>(), test_allocator<int>(3));
    assert(q.c.get_allocator() == test_allocator<int>(3));
    assert(q.c.size() == 0);
}

namespace
{
    template <class T>
    struct CtorCompContAllocTest
        : public std::priority_queue<T, std::vector<T, test_allocator<T> > >
    {
        typedef std::priority_queue<T, std::vector<T, test_allocator<T> > > base;
        typedef typename base::container_type container_type;
        typedef typename base::value_compare value_compare;

        explicit CtorCompContAllocTest(const test_allocator<int>& a) : base(a)
        {}
        CtorCompContAllocTest(const value_compare& compare, const test_allocator<int>& a)
            : base(compare, a)
        {}
        CtorCompContAllocTest(const value_compare& compare, const container_type& container,
            const test_allocator<int>& a) : base(compare, container, a)
        {}
    #if TEST_STD_VER >= 11 // testing rvalue constructor
        CtorCompContAllocTest(const value_compare& compare, container_type&& container,
            const test_allocator<int>& a) : base(compare, std::move(container), a)
        {}
        CtorCompContAllocTest(CtorCompContAllocTest&& q, const test_allocator<int>& a) : base(std::move(q), a)
        {}
    #endif
        test_allocator<int> get_allocator()
        {
            return c.get_allocator();
        }

        using base::c;
    };
}

TEST(ctor_comp_cont_alloc)
{
    typedef std::vector<int, test_allocator<int> > C;
    C v = make<C>(5);
    CtorCompContAllocTest<int> q(std::less<int>(), v, test_allocator<int>(3));
    assert(q.c.get_allocator() == test_allocator<int>(3));
    assert(q.size() == 5);
    assert(q.top() == 4);
}

namespace
{
    template <class T>
    struct CtorCompRcontAllocTest
        : public std::priority_queue<T, std::vector<T, test_allocator<T> > >
    {
        typedef std::priority_queue<T, std::vector<T, test_allocator<T> > > base;
        typedef typename base::container_type container_type;
        typedef typename base::value_compare value_compare;

        explicit CtorCompRcontAllocTest(const test_allocator<int>& a) : base(a)
        {}
        CtorCompRcontAllocTest(const value_compare& compare, const test_allocator<int>& a)
            : base(compare, a)
        {}
        CtorCompRcontAllocTest(const value_compare& compare, const container_type& container,
            const test_allocator<int>& a) : base(compare, container, a)
        {}
    #if TEST_STD_VER >= 11 // testing rvalue ctor
        CtorCompRcontAllocTest(const value_compare& compare, container_type&& container,
            const test_allocator<int>& a) : base(compare, std::move(container), a)
        {}
        CtorCompRcontAllocTest(CtorCompRcontAllocTest&& q, const test_allocator<int>& a) : base(std::move(q), a)
        {}
    #endif
        test_allocator<int> get_allocator()
        {
            return c.get_allocator();
        }

        using base::c;
    };
}

TEST(ctor_comp_rcont_alloc)
{
    typedef std::vector<int, test_allocator<int> > C;
    CtorCompRcontAllocTest<int> q(std::less<int>(), make<C>(5), test_allocator<int>(3));
    assert(q.c.get_allocator() == test_allocator<int>(3));
    assert(q.size() == 5);
    assert(q.top() == 4);
}

namespace
{
    template <class T>
    struct CtorCopyAllocTest
        : public std::priority_queue<T, std::vector<T, test_allocator<T> > >
    {
        typedef std::priority_queue<T, std::vector<T, test_allocator<T> > > base;
        typedef typename base::container_type container_type;
        typedef typename base::value_compare value_compare;

        explicit CtorCopyAllocTest(const test_allocator<int>& a) : base(a)
        {}
        CtorCopyAllocTest(const value_compare& compare, const test_allocator<int>& a)
            : base(compare, c, a)
        {}
        CtorCopyAllocTest(const value_compare& compare, const container_type& container,
            const test_allocator<int>& a) : base(compare, container, a)
        {}
        CtorCopyAllocTest(const CtorCopyAllocTest& q, const test_allocator<int>& a) : base(q, a)
        {}
        test_allocator<int> get_allocator()
        {
            return c.get_allocator();
        }

        using base::c;
    };
}

TEST(ctor_copy_alloc)
{
    CtorCopyAllocTest<int> qo(std::less<int>(),
        make<std::vector<int, test_allocator<int> > >(5),
        test_allocator<int>(2));
    CtorCopyAllocTest<int> q(qo, test_allocator<int>(6));
    assert(q.size() == 5);
    assert(q.c.get_allocator() == test_allocator<int>(6));
    assert(q.top() == int(4));
}

namespace
{
    template<class T, class Cont, class Comp = std::less<T> >
    struct CtorIterIterAllocTest : std::priority_queue<T, Cont, Comp> {
        typedef std::priority_queue<T, Cont, Comp> base;

        template<class It, class Alloc>
        explicit CtorIterIterAllocTest(It first, It last, const Alloc& a) : base(first, last, a)
        {}

        using base::c;
    };
}

TEST(ctor_iter_iter_alloc)
{
    int a[] = {3, 5, 2, 0, 6, 8, 1};
    typedef test_allocator<int> Alloc;
    CtorIterIterAllocTest<int, std::vector<int, Alloc> > q(a, a + 7, Alloc(2));
    assert(q.size() == 7);
    assert(q.top() == 8);
    assert(q.c.get_allocator() == Alloc(2));
}

namespace
{
    template<class T, class Cont, class Comp = std::less<T> >
    struct CtorIterIterCompAllocTest : std::priority_queue<T, Cont, Comp> {
        typedef std::priority_queue<T, Cont, Comp> base;

        template<class It, class Alloc>
        explicit CtorIterIterCompAllocTest(It first, It last, const Comp& compare, const Alloc& a) : base(first, last, compare, a)
        {}

        using base::c;
    };
}

TEST(ctor_iter_iter_comp_alloc)
{
    int a[] = {3, 5, 2, 0, 6, 8, 1};
    typedef test_allocator<int> Alloc;
    CtorIterIterCompAllocTest<int, std::vector<int, Alloc>, std::greater<int> > q(a, a + 7, std::greater<int>(), Alloc(2));
    assert(q.size() == 7);
    assert(q.top() == 0);
    assert(q.c.get_allocator() == Alloc(2));
}

namespace
{
    template<class T, class Cont, class Comp = std::less<T> >
    struct CtorIterIterCompContAllocTest : std::priority_queue<T, Cont, Comp> {
        typedef std::priority_queue<T, Cont, Comp> base;

        template<class It, class Alloc>
        explicit CtorIterIterCompContAllocTest(It first, It last, const Comp& compare, const Cont& v, const Alloc& a) : base(first, last, compare, v, a)
        {}

        using base::c;
    };

}

TEST(ctor_iter_iter_comp_cont_alloc)
{
    typedef test_allocator<int> Alloc;
    int a[] = {3, 5, 2, 0, 6, 8, 1};
    std::vector<int, Alloc> v(a, a + 3);
    CtorIterIterCompContAllocTest<int, std::vector<int, Alloc> > q(a + 3, a + 7, std::less<int>(), v, Alloc(2));
    assert(q.size() == 7);
    assert(q.top() == 8);
    assert(q.c.get_allocator() == Alloc(2));
}

namespace
{
    template<class T, class Cont, class Comp = std::less<T> >
    struct CtorIterIterCompRcontAllocTest : std::priority_queue<T, Cont, Comp> {
        typedef std::priority_queue<T, Cont, Comp> base;

        template<class It, class Alloc>
        explicit CtorIterIterCompRcontAllocTest(It first, It last, const Comp& compare, Cont&& v, const Alloc& a) : base(first, last, compare, std::move(v), a)
        {}

        using base::c;
    };

}

TEST(ctor_iter_iter_comp_rcont_alloc)
{
    using Alloc = test_allocator<MoveOnly>;
    int a[] = {3, 5, 2, 0, 6, 8, 1};
    CtorIterIterCompRcontAllocTest<MoveOnly, std::vector<MoveOnly, Alloc>> q(
        a + 3, a + 7, std::less<MoveOnly>(),
        std::vector<MoveOnly, Alloc>(a, a + 3), Alloc(2));
    assert(q.size() == 7);
    assert(q.top() == MoveOnly(8));
    assert(q.c.get_allocator() == Alloc(2));
}

namespace
{
    template <class T>
    struct CtorMoveAllocTest
        : public std::priority_queue<T, std::vector<T, test_allocator<T> > >
    {
        typedef std::priority_queue<T, std::vector<T, test_allocator<T> > > base;
        typedef typename base::container_type container_type;
        typedef typename base::value_compare value_compare;

        explicit CtorMoveAllocTest(const test_allocator<int>& a) : base(a)
        {}
        CtorMoveAllocTest(const value_compare& compare, const test_allocator<int>& a)
            : base(compare, c, a)
        {}
        CtorMoveAllocTest(const value_compare& compare, const container_type& container,
            const test_allocator<int>& a) : base(compare, container, a)
        {}
        CtorMoveAllocTest(const value_compare& compare, container_type&& container,
            const test_allocator<int>& a) : base(compare, std::move(container), a)
        {}
        CtorMoveAllocTest(CtorMoveAllocTest&& q, const test_allocator<int>& a) : base(std::move(q), a)
        {}
        test_allocator<int> get_allocator()
        {
            return c.get_allocator();
        }

        using base::c;
    };

}

TEST(ctor_move_alloc)
{
    CtorMoveAllocTest<MoveOnly> qo(std::less<MoveOnly>(),
        make_move_only<std::vector<MoveOnly, test_allocator<MoveOnly> > >(5),
        test_allocator<MoveOnly>(2));
    CtorMoveAllocTest<MoveOnly> q(std::move(qo), test_allocator<MoveOnly>(6));
    assert(q.size() == 5);
    assert(q.c.get_allocator() == test_allocator<MoveOnly>(6));
    assert(q.top() == MoveOnly(4));
}

TEST(assign_copy)
{
    std::vector<int> v = make<std::vector<int> >(5);
    std::priority_queue<int, std::vector<int>, std::greater<int> > qo(std::greater<int>(), v);
    std::priority_queue<int, std::vector<int>, std::greater<int> > q;
    q = qo;
    assert(q.size() == 5);
    assert(q.top() == 0);
}

TEST(assign_move)
{
    std::priority_queue<MoveOnly> qo(std::less<MoveOnly>(), make_move_only<std::vector<MoveOnly> >(5));
    std::priority_queue<MoveOnly> q;
    q = std::move(qo);
    assert(q.size() == 5);
    assert(q.top() == MoveOnly(4));
}

TEST(ctor_comp_container)
{
    typedef std::vector<int> Container;
    typedef std::greater<int> Compare;
    typedef std::priority_queue<int, Container, Compare> Q;
    Container v = make<Container>(5);
    Q q(Compare(), v);
    assert(q.size() == 5);
    assert(q.top() == 0);

#if TEST_STD_VER >= 11
    // It should be explicit, so not convertible before C++20.
    static_assert(test_convertible<Q, const Compare&, const Container&>(), "");
#endif
}

TEST(ctor_comp_rcontainer)
{
    typedef std::vector<MoveOnly> Container;
    typedef std::less<MoveOnly> Compare;
    typedef std::priority_queue<MoveOnly> Q;
    Q q(Compare(), make_move_only<Container>(5));
    assert(q.size() == 5);
    assert(q.top() == MoveOnly(4));

    static_assert(test_convertible<Q, const Compare&, Container&&>(), "");
}

TEST(ctor_copy)
{
    std::vector<int> v = make<std::vector<int> >(5);
    std::priority_queue<int, std::vector<int>, std::greater<int> > qo(std::greater<int>(), v);
    std::priority_queue<int, std::vector<int>, std::greater<int> > q = qo;
    assert(q.size() == 5);
    assert(q.top() == 0);
}

TEST(ctor_iter_iter)
{
    int  a[] = {3, 5, 2, 0, 6, 8, 1};
    int* an  = a + std::size(a);
    std::priority_queue<int> q(a, an);
    assert(q.size() == static_cast<std::size_t>(an - a));
    assert(q.top() == 8);
}

TEST(ctor_iter_iter_comp)
{
    int a[] = {3, 5, 2, 0, 6, 8, 1};
    int* an = a + sizeof(a) / sizeof(a[0]);
    std::priority_queue<int, std::vector<int>, std::greater<int> >
        q(a, an, std::greater<int>());
    assert(q.size() == static_cast<std::size_t>(an - a));
    assert(q.top() == 0);
}

TEST(ctor_iter_iter_comp_cont)
{
    int a[] = {3, 5, 2, 0, 6, 8, 1};
    const int n = sizeof(a) / sizeof(a[0]);
    std::vector<int> v(a, a + n / 2);
    std::priority_queue<int> q(a + n / 2, a + n, std::less<int>(), v);
    assert(q.size() == n);
    assert(q.top() == 8);
}

TEST(ctor_iter_iter_comp_rcont)
{
    int a[] = {3, 5, 2, 0, 6, 8, 1};
    const int n = sizeof(a) / sizeof(a[0]);
    std::priority_queue<MoveOnly> q(a + n / 2, a + n,
        std::less<MoveOnly>(),
        std::vector<MoveOnly>(a, a + n / 2));
    assert(q.size() == n);
    assert(q.top() == MoveOnly(8));
}

TEST(ctor_move)
{
    std::priority_queue<MoveOnly> qo(std::less<MoveOnly>(), make_move_only<std::vector<MoveOnly> >(5));
    std::priority_queue<MoveOnly> q = std::move(qo);
    assert(q.size() == 5);
    assert(q.top() == MoveOnly(4));
}

TEST(emplace)
{
    std::priority_queue<Emplaceable> q;
    q.emplace(1, 2.5);
    assert(q.top() == Emplaceable(1, 2.5));
    q.emplace(3, 4.5);
    assert(q.top() == Emplaceable(3, 4.5));
    q.emplace(2, 3.5);
    assert(q.top() == Emplaceable(3, 4.5));
}

TEST(empty)
{
    std::priority_queue<int> q;
    assert(q.empty());
    q.push(1);
    assert(!q.empty());
    q.pop();
    assert(q.empty());
}

TEST(pop)
{
    std::priority_queue<int> q;
    q.push(1);
    assert(q.top() == 1);
    q.push(3);
    assert(q.top() == 3);
    q.push(2);
    assert(q.top() == 3);
    q.pop();
    assert(q.top() == 2);
    q.pop();
    assert(q.top() == 1);
    q.pop();
    assert(q.empty());
}

TEST(push)
{
    std::priority_queue<int> q;
    q.push(1);
    assert(q.top() == 1);
    q.push(3);
    assert(q.top() == 3);
    q.push(2);
    assert(q.top() == 3);
}

TEST(size)
{
    std::priority_queue<int> q;
    assert(q.size() == 0);
    q.push(1);
    assert(q.size() == 1);
    q.pop();
    assert(q.size() == 0);
}

TEST(swap)
{
    std::priority_queue<int> q1;
    std::priority_queue<int> q2;
    q1.push(1);
    q1.push(3);
    q1.push(2);
    q1.swap(q2);
    assert(q1.empty());
    assert(q2.size() == 3);
    assert(q2.top() == 3);
}

TEST(top)
{
    std::priority_queue<int> q;
    q.push(1);
    assert(q.top() == 1);
    q.push(3);
    assert(q.top() == 3);
    q.push(2);
    assert(q.top() == 3);
}


#endif
