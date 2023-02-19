// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

/**
 * @file template_remover.hpp
 * @brief Allows to access tagged tuples by a string representation of the name.
 */

#ifndef FCPP_COMMON_TEMPLATE_REMOVER_H_
#define FCPP_COMMON_TEMPLATE_REMOVER_H_

#include <cassert>

#include "lib/common/tagged_tuple.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


/**
 * @brief Namespace containing objects of common use.
 */
namespace common {

//! @brief Type predicate that is always true.
template <typename T>
using any_type = std::true_type;

//! @brief Type predicates checking whether the type is convertible to R.
template <typename R>
struct convertible_to {
    template <typename T>
    using predicate = std::is_convertible<T, R>;
};

//! @brief Type predicates checking whether we can assign R to the type.
template <typename R>
struct can_assign {
    template <typename T>
    using predicate = std::is_assignable<T&, R>;
};

//! @brief Wrapper for passing type predicates as arguments.
template <template<class> class P>
struct type_predicate {};


//! @cond INTERNAL
namespace details {
    //! @brief General form.
    template <typename T, typename B>
    struct type_filter;

    //! @brief Empty base case.
    template <>
    struct type_filter<type_sequence<>, bool_pack<>> {
        using type = type_sequence<>;
    };

    //! @brief Positive recursive form.
    template <typename T, typename... Ts, bool... bs>
    struct type_filter<type_sequence<T,Ts...>, bool_pack<true,bs...>> {
        using type = typename type_filter<type_sequence<Ts...>, bool_pack<bs...>>::type::template push_front<T>;
    };

    //! @brief Negative recursive form.
    template <typename T, typename... Ts, bool... bs>
    struct type_filter<type_sequence<T,Ts...>, bool_pack<false,bs...>> : type_filter<type_sequence<Ts...>, bool_pack<bs...>> {};
}
//! @endcond

//! @brief Filters a type sequence according to a type predicate.
template <template<class> class P, typename... Ts>
using type_filter = typename details::type_filter<type_sequence<Ts...>, bool_pack<P<Ts>::value...>>::type;


//! @cond INTERNAL
namespace details {
    //! @brief General form.
    template <typename P, typename S, typename T>
    struct tagged_tuple_filter;

    //! @brief Unwrapping case.
    template <template<class> class P, typename... Ss, typename... Ts>
    struct tagged_tuple_filter<type_predicate<P>, type_sequence<Ss...>, type_sequence<Ts...>> :
        tagged_tuple_filter<bool_pack<P<Ts>::value...>, type_sequence<Ss...>, type_sequence<Ts...>> {};

    //! @brief Empty base case.
    template <>
    struct tagged_tuple_filter<bool_pack<>, type_sequence<>, type_sequence<>> {
        using type = tagged_tuple<type_sequence<>, type_sequence<>>;
    };

    //! @brief Positive recursive form.
    template <bool... bs, typename S, typename... Ss, typename T, typename... Ts>
    struct tagged_tuple_filter<bool_pack<true,bs...>, type_sequence<S,Ss...>, type_sequence<T,Ts...>> {
        using type = typename tagged_tuple_filter<bool_pack<bs...>, type_sequence<Ss...>, type_sequence<Ts...>>::type::template push_front<S,T>;
    };

    //! @brief Negative recursive form.
    template <bool... bs, typename S, typename... Ss, typename T, typename... Ts>
    struct tagged_tuple_filter<bool_pack<false,bs...>, type_sequence<S,Ss...>, type_sequence<T,Ts...>> :
        tagged_tuple_filter<bool_pack<bs...>, type_sequence<Ss...>, type_sequence<Ts...>> {};
}
//! @endcond

//! @brief Filters a tagged tuple according to a predicate on value types.
template <template<class> class P, typename T>
using tagged_tuple_filter = typename details::tagged_tuple_filter<type_predicate<P>, typename std::decay_t<T>::tags, typename std::decay_t<T>::types>::type;


//! @cond INTERNAL
namespace details {
    template <typename F, typename T>
    struct applier_return_type_impl {
        struct type {
            template <typename U>
            operator U() const {
                return *((U*)42);
            }
        };
    };
    template <typename F, typename S, typename T, typename... Ts>
    struct applier_return_type_impl<F, tagged_tuple<S, type_sequence<T, Ts...>>> {
        using type = decltype(std::declval<F>()(std::declval<T&>()));
    };
}
//! @endcond

//! @brief The type returned by an applier call with given argument types.
template <typename U, typename F, template<class> class P = any_type>
using applier_return_type = typename details::applier_return_type_impl<F, tagged_tuple_filter<P,U>>::type;


//! @cond INTERNAL
namespace details {
    //! @brief Empty base case.
    template <typename U, typename F, template<class> class P>
    inline applier_return_type<U,F,P> applier(std::string const&, U&&, F&&, type_predicate<P>, type_sequence<>, type_sequence<>) {
        assert(false);
    }

    //! @brief Recursive form.
    template <typename U, typename F, template<class> class P, typename S1, typename... S, typename T1, typename... T>
    inline applier_return_type<U,F,P> applier(std::string const& name, U&& t, F&& f, type_predicate<P>, type_sequence<S1, S...>, type_sequence<T1, T...>) {
        if (strip_namespaces(type_name<S1>()) == name) {
            return f(get<S1>(std::forward<U>(t)));
        }
        return applier(name, std::forward<U>(t), std::forward<F>(f), type_predicate<P>{}, type_sequence<S...>{}, type_sequence<T...>{});
    }
}
//! @endcond

//! @brief Applies a function to the element of a tagged tuple with a given tag (the function has to be applicable to types satisfying the predicate with a same return type)
template <template<class> class P = any_type, typename U, typename F>
applier_return_type<U,F,P> applier(std::string const& name, U&& t, F&& f, type_predicate<P> = {}) {
    return details::applier(name, std::forward<U>(t), std::forward<F>(f), type_predicate<P>{}, typename tagged_tuple_filter<P,U>::tags{}, typename tagged_tuple_filter<P,U>::types{});
}


//! @brief Accesses a field of a tagged tuple by the string name of a tag.
template <typename R, typename U>
inline R getter(std::string const& name, U const& t) {
    return applier<convertible_to<R>::template predicate>(name, t, [](auto const& x){ return x; });
}


//! @brief Updates a field of a tagged tuple by the string name of a tag.
template <typename R, typename U>
void setter(std::string const& name, U& t, R&& val) {
    applier<can_assign<R>::template predicate>(name, t, [&val](auto& x){ x = std::forward<R>(val); });
}


//! @cond INTERNAL
namespace details {
    // General form.
    template <typename... Ts>
    struct tagged_tuple_unite;

    // No tuple to concatenate.
    template <>
    struct tagged_tuple_unite<> {
        using type = tagged_tuple<type_sequence<>, type_sequence<>>;
    };

    // The first tuple is empty.
    template <typename... Ts>
    struct tagged_tuple_unite<tagged_tuple<type_sequence<>, type_sequence<>>, Ts...> : tagged_tuple_unite<Ts...> {};

    // The first tuple is not empty.
    template <typename S, typename... Ss, typename T, typename... Ts, typename... Us>
    struct tagged_tuple_unite<tagged_tuple<type_sequence<S, Ss...>, type_sequence<T, Ts...>>, Us...> {
        using tmp = typename tagged_tuple_unite<tagged_tuple<type_sequence<Ss...>, type_sequence<Ts...>>, Us...>::type;
        using type = std::conditional_t<tmp::tags::template count<S> == 0, typename tmp::template push_front<S,T>, tmp>;
    };
}
//! @endcond

//! @brief Unites multiple `tagged_tuple` types into a single `tagged_tuple` type, merging duplicates.
template <typename... Ts>
using tagged_tuple_unite = typename details::tagged_tuple_unite<Ts...>::type;

} // namespace common

} // namespace fcpp

#endif  // FCPP_COMMON_TEMPLATE_REMOVER_H_
