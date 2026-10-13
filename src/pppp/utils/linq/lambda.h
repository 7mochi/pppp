#ifndef PPPP_UTILS_LINQ_LAMBDA_H
#define PPPP_UTILS_LINQ_LAMBDA_H

namespace pppp { namespace utils { namespace linq {
    namespace detail {
        template <typename A, typename B>
        struct ArithmeticResult;

        template <typename T>
        struct ArithmeticResult<T, T> {
            typedef T type;
        };

        template <typename T>
        struct Constant {
            T value;

            template <typename A>
            struct Result {
                typedef T type;
            };

            template <typename A>
            T operator()(const A&) const {
                return value;
            }
        };

        struct Argument {
            template <typename A>
            struct Result {
                typedef A type;
            };

            template <typename A>
            const A& operator()(const A& a) const {
                return a;
            }
        };

        struct MinusAction {
            template <typename X, typename Y>
            struct Result {
                typedef typename ArithmeticResult<X, Y>::type type;
            };

            template <typename X, typename Y>
            static typename Result<X, Y>::type apply(const X& x, const Y& y) {
                return x - y;
            }
        };

        struct DivideAction {
            template <typename X, typename Y>
            struct Result {
                typedef typename ArithmeticResult<X, Y>::type type;
            };

            template <typename X, typename Y>
            static typename Result<X, Y>::type apply(const X& x, const Y& y) {
                return x / y;
            }
        };

        struct LessAction {
            template <typename X, typename Y>
            struct Result {
                typedef bool type;
            };

            template <typename X, typename Y>
            static bool apply(const X& x, const Y& y) {
                return x < y;
            }
        };

        struct GreaterAction {
            template <typename X, typename Y>
            struct Result {
                typedef bool type;
            };

            template <typename X, typename Y>
            static bool apply(const X& x, const Y& y) {
                return x > y;
            }
        };

        struct EqualAction {
            template <typename X, typename Y>
            struct Result {
                typedef bool type;
            };

            template <typename X, typename Y>
            static bool apply(const X& x, const Y& y) {
                return x == y;
            }
        };

        struct AndAction {};

        struct OrAction {};

        template <typename L, typename R, typename Action>
        struct Binary {
            L l;
            R r;

            template <typename A>
            struct Result {
                typedef typename Action::template Result<typename L::template Result<A>::type,
                                                         typename R::template Result<A>::type>::type type;
            };

            template <typename A>
            typename Result<A>::type operator()(const A& a) const {
                return Action::apply(l(a), r(a));
            }
        };

        template <typename L, typename R>
        struct Binary<L, R, AndAction> {
            L l;
            R r;

            template <typename A>
            struct Result {
                typedef bool type;
            };

            template <typename A>
            bool operator()(const A& a) const {
                return l(a) && r(a);
            }
        };

        template <typename L, typename R>
        struct Binary<L, R, OrAction> {
            L l;
            R r;

            template <typename A>
            struct Result {
                typedef bool type;
            };

            template <typename A>
            bool operator()(const A& a) const {
                return l(a) || r(a);
            }
        };

        template <typename F>
        struct Not {
            F f;

            template <typename A>
            struct Result {
                typedef bool type;
            };

            template <typename A>
            bool operator()(const A& a) const {
                return !f(a);
            }
        };

        template <typename R, typename P1, typename E1>
        struct Call1 {
            R (*f)(P1);
            E1 e1;

            template <typename A>
            struct Result {
                typedef R type;
            };

            template <typename A>
            R operator()(const A& a) const {
                return f(e1(a));
            }
        };

        template <typename R, typename P1, typename P2, typename E1, typename E2>
        struct Call2 {
            R (*f)(P1, P2);
            E1 e1;
            E2 e2;

            template <typename A>
            struct Result {
                typedef R type;
            };

            template <typename A>
            R operator()(const A& a) const {
                return f(e1(a), e2(a));
            }
        };

        template <typename R, typename P1, typename P2, typename P3, typename P4, typename E1, typename E2,
                  typename E3, typename E4>
        struct Call4 {
            R (*f)(P1, P2, P3, P4);
            E1 e1;
            E2 e2;
            E3 e3;
            E4 e4;

            template <typename A>
            struct Result {
                typedef R type;
            };

            template <typename A>
            R operator()(const A& a) const {
                return f(e1(a), e2(a), e3(a), e4(a));
            }
        };

        template <typename R, typename C>
        struct CallMember0 {
            R (C::*f)();
            C* object;

            template <typename A>
            struct Result {
                typedef R type;
            };

            template <typename A>
            R operator()(const A&) const {
                return (object->*f)();
            }
        };

        template <typename M, typename C, typename E>
        struct Member {
            M C::* m;
            E e;

            template <typename A>
            struct Result {
                typedef M type;
            };

            template <typename A>
            M operator()(const A& a) const {
                return get(e(a));
            }

            M get(const C& object) const { return object.*m; }
            M get(const C* object) const { return object->*m; }
        };

        template <typename F>
        struct IsPure {
            static const bool value = false;
        };

        template <typename T>
        struct IsPure<Constant<T> > {
            static const bool value = true;
        };

        template <>
        struct IsPure<Argument> {
            static const bool value = true;
        };

        template <typename L, typename R, typename Action>
        struct IsPure<Binary<L, R, Action> > {
            static const bool value = IsPure<L>::value && IsPure<R>::value;
        };

        template <typename M, typename C, typename E>
        struct IsPure<Member<M, C, E> > {
            static const bool value = IsPure<E>::value;
        };

        template <typename F>
        struct IsPure<Not<F> > {
            static const bool value = IsPure<F>::value;
        };
    } // namespace detail

    template <typename F>
    struct Lambda {
        F f;

        template <typename A>
        struct Result {
            typedef typename F::template Result<A>::type type;
        };

        template <typename A>
        typename Result<A>::type operator()(const A& a) const {
            return f(a);
        }
    };

    namespace detail {
        template <typename F>
        struct IsPure<Lambda<F> > {
            static const bool value = IsPure<F>::value;
        };
    } // namespace detail

    template <typename T>
    struct AsFunctor {
        typedef detail::Constant<T> type;

        static type make(const T& value) {
            type constant = {value};
            return constant;
        }
    };

    template <typename F>
    struct AsFunctor<Lambda<F> > {
        typedef F type;

        static const F& make(const Lambda<F>& lambda) { return lambda.f; }
    };

    /// The lambda's parameter, the `s` of `s => …`, after Boost.Lambda's placeholder:
    /// https://github.com/boostorg/lambda/blob/a14036e312ffadfd65cb333b15602d2b3ab45ff3/include/boost/lambda/core.hpp#L67
    /// The operators that build an expression on it and `bind` follow
    /// https://github.com/boostorg/lambda/blob/a14036e312ffadfd65cb333b15602d2b3ab45ff3/include/boost/lambda/detail/operators.hpp
    /// https://github.com/boostorg/lambda/blob/a14036e312ffadfd65cb333b15602d2b3ab45ff3/include/boost/lambda/detail/bind_functions.hpp
    const Lambda<detail::Argument> _1 = Lambda<detail::Argument>();

    template <typename L, typename R, typename Action>
    Lambda<detail::Binary<typename AsFunctor<L>::type, typename AsFunctor<R>::type, Action> >
    binary(const L& l, const R& r) {
        Lambda<detail::Binary<typename AsFunctor<L>::type, typename AsFunctor<R>::type, Action> > lambda = {
            {AsFunctor<L>::make(l), AsFunctor<R>::make(r)}};
        return lambda;
    }

    template <typename F, typename T>
    Lambda<detail::Binary<F, detail::Constant<T>, detail::MinusAction> > operator-(const Lambda<F>& l,
                                                                                   const T& r) {
        return binary<Lambda<F>, T, detail::MinusAction>(l, r);
    }

    template <typename F, typename G>
    Lambda<detail::Binary<F, G, detail::MinusAction> > operator-(const Lambda<F>& l, const Lambda<G>& r) {
        return binary<Lambda<F>, Lambda<G>, detail::MinusAction>(l, r);
    }

    template <typename F, typename T>
    Lambda<detail::Binary<F, detail::Constant<T>, detail::DivideAction> > operator/(const Lambda<F>& l,
                                                                                    const T& r) {
        return binary<Lambda<F>, T, detail::DivideAction>(l, r);
    }

    template <typename F, typename T>
    Lambda<detail::Binary<F, detail::Constant<T>, detail::LessAction> > operator<(const Lambda<F>& l,
                                                                                  const T& r) {
        return binary<Lambda<F>, T, detail::LessAction>(l, r);
    }

    template <typename F, typename T>
    Lambda<detail::Binary<F, detail::Constant<T>, detail::GreaterAction> > operator>(const Lambda<F>& l,
                                                                                     const T& r) {
        return binary<Lambda<F>, T, detail::GreaterAction>(l, r);
    }

    template <typename F, typename T>
    Lambda<detail::Binary<F, detail::Constant<T>, detail::EqualAction> > operator==(const Lambda<F>& l,
                                                                                    const T& r) {
        return binary<Lambda<F>, T, detail::EqualAction>(l, r);
    }

    template <typename F>
    Lambda<detail::Not<F> > operator!(const Lambda<F>& l) {
        Lambda<detail::Not<F> > lambda = {{l.f}};
        return lambda;
    }

    template <typename F, typename G>
    Lambda<detail::Binary<F, G, detail::AndAction> > operator&&(const Lambda<F>& l, const Lambda<G>& r) {
        return binary<Lambda<F>, Lambda<G>, detail::AndAction>(l, r);
    }

    template <typename F, typename G>
    Lambda<detail::Binary<F, G, detail::OrAction> > operator||(const Lambda<F>& l, const Lambda<G>& r) {
        return binary<Lambda<F>, Lambda<G>, detail::OrAction>(l, r);
    }

    template <typename R, typename P1, typename E1>
    Lambda<detail::Call1<R, P1, typename AsFunctor<E1>::type> > bind(R (*f)(P1), const E1& e1) {
        Lambda<detail::Call1<R, P1, typename AsFunctor<E1>::type> > lambda = {{f, AsFunctor<E1>::make(e1)}};
        return lambda;
    }

    template <typename R, typename P1, typename P2, typename E1, typename E2>
    Lambda<detail::Call2<R, P1, P2, typename AsFunctor<E1>::type, typename AsFunctor<E2>::type> >
    bind(R (*f)(P1, P2), const E1& e1, const E2& e2) {
        Lambda<detail::Call2<R, P1, P2, typename AsFunctor<E1>::type, typename AsFunctor<E2>::type> > lambda =
            {{f, AsFunctor<E1>::make(e1), AsFunctor<E2>::make(e2)}};
        return lambda;
    }

    template <typename R, typename P1, typename P2, typename P3, typename P4, typename E1, typename E2,
              typename E3, typename E4>
    Lambda<detail::Call4<R, P1, P2, P3, P4, typename AsFunctor<E1>::type, typename AsFunctor<E2>::type,
                         typename AsFunctor<E3>::type, typename AsFunctor<E4>::type> >
    bind(R (*f)(P1, P2, P3, P4), const E1& e1, const E2& e2, const E3& e3, const E4& e4) {
        Lambda<detail::Call4<R, P1, P2, P3, P4, typename AsFunctor<E1>::type, typename AsFunctor<E2>::type,
                             typename AsFunctor<E3>::type, typename AsFunctor<E4>::type> >
            lambda = {{f, AsFunctor<E1>::make(e1), AsFunctor<E2>::make(e2), AsFunctor<E3>::make(e3),
                       AsFunctor<E4>::make(e4)}};
        return lambda;
    }

    template <typename M, typename C, typename E>
    Lambda<detail::Member<M, C, typename AsFunctor<E>::type> > bind(M C::* m, const E& e) {
        Lambda<detail::Member<M, C, typename AsFunctor<E>::type> > lambda = {{m, AsFunctor<E>::make(e)}};
        return lambda;
    }

    template <typename R, typename C>
    Lambda<detail::CallMember0<R, C> > bind(R (C::*f)(), C* object) {
        Lambda<detail::CallMember0<R, C> > lambda = {{f, object}};
        return lambda;
    }
}}} // namespace pppp::utils::linq

#endif
