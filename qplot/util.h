#pragma once

#include <cxxabi.h>
#include <sstream>

// Type info
template <typename T>
std::string objName(const T& obj)
{
	std::ostringstream os;
	int info;
    os << abi::__cxa_demangle(typeid(obj).name(),0,0,&info);
	return os.str();
}


// Adapted from http://stackoverflow.com/a/25958302/254035
// Returns the index if the typle contains the type, or -1 otherwise

// The general template is undefined, we're only intereted in the specializations below
template <typename T, size_t N, typename Tuple>
struct has_type;

// We've exhausted searching all tuple elements - not found
template <typename T, size_t N>
struct has_type<T, N, std::tuple<>> : std::integral_constant<int,-1> {};

// A match: T matches the first T of the tuple, now at position N
template <typename T, size_t N, typename... Ts>
struct has_type<T, N, std::tuple<T, Ts...>> : std::integral_constant<int,N> {};

// No match: U is not T
template <typename T, size_t N, typename U, typename... Ts>
struct has_type<T, N, std::tuple<U, Ts...>> : has_type<T, N+1, std::tuple<Ts...>> {};

template <typename T, typename Tuple>
using tuple_contains_type = typename has_type<T, 0, Tuple>::type;



// has_supported_types<T> tests wether T::foo exists as a nested type
// The idea is that if Check<T> is a valid substitution (and so T exists), this template is enabled
// Modified from http://stackoverflow.com/a/11816999/254035

template<class T>
struct Check {
    using type = void;
};

// A style is any type for which the nested type `supported_types` exists, regardless of its type
template<class T, class U = void>
struct is_style {
    static constexpr bool value = false;
};

template<class T>
struct is_style<T, typename Check<typename T::supported_types>::type> {
    static constexpr bool value = true;
};

// has_supported_types must include at least one tuple element to be true (perhaps consider this is_object_style)
template<class T, class U = void>
struct has_supported_types {
    static constexpr bool value = false;
};

template<class T>
// struct has_supported_types<T, typename Check<std::tuple_element<0,typename T::supported_types>>::type> {
struct has_supported_types<T, typename std::enable_if<std::tuple_size<typename T::supported_types>::value != 0>::type> {
    static constexpr bool value = true;
};


template<int N>
struct CheckNontype {
    using type = void;
};

template<class T, class U = void>
struct has_pass {
    static constexpr bool value = false;
};

// The idea is that if CheckNontype<T> is a valid substitution (and so T exists), this template is enabled
template<class T>
struct has_pass<T, typename CheckNontype<T::pass>::type> {
    static constexpr bool value = true;
};


// Map updater

template <typename T> struct plot_traits;

template<class T, class Map, class ObjTypes, std::size_t N>
struct MapUpdater {
    static void update(Map& plotStyles, const T& style)
    {
		using Object = std::tuple_element_t<N-1, ObjTypes>;
        using StyleVariant = typename plot_traits<Object>::type::supported_styles;

        size_t key = plot_traits<Object>::type::id;
        plotStyles[key] = StyleVariant{style};

        MapUpdater<T, Map, ObjTypes, N-1>::update(plotStyles, style);
    }
};
 
template<class T, class Map, class ObjTypes>
struct MapUpdater<T, Map, ObjTypes, 1> {
    static void update(Map& plotStyles, const T& style)
    {
		using Object = std::tuple_element_t<0, ObjTypes>;
        using StyleVariant = typename plot_traits<Object>::type::supported_styles;

        size_t key = plot_traits<Object>::type::id;
        plotStyles[key] = StyleVariant{style};
    }
};

// Todo: can't we just have a base case for when N=0?
template<class T, class Map, class ObjTypes>
struct MapUpdater<T, Map, ObjTypes, 0> {
    static void update(Map& plotStyles, const T& style)
    {
    }
};


// http://en.cppreference.com/w/cpp/utility/tuple/tuple

template <typename P>
class Process;

template<class P, class Tuple, std::size_t N>
struct TuplePrinter {
    static void writeStyleString(Process<P>& gnuplot, const Tuple& t)
    {
        TuplePrinter<P, Tuple, N-1>::writeStyleString(gnuplot, t);
        gnuplot << std::get<N-1>(t).c_str();
    }
};
 
template<class P, class Tuple>
struct TuplePrinter<P, Tuple, 1>{
    static void writeStyleString(Process<P>& gnuplot, const Tuple& t)
    {
        gnuplot << std::get<0>(t).c_str();
    }
};
 
template<class P, class... Args>
void writeStyleString(Process<P>& gnuplot, const std::tuple<Args...>& t)
{
    TuplePrinter<P, decltype(t), sizeof...(Args)>::writeStyleString(gnuplot, t);
}

template<class P, class T>
void writeStyleString(Process<P>& gnuplot, const T& styleString)
{
    gnuplot << styleString.c_str();
}


// Determine wether a plot method of the right type exists for a class
// Adapted from http://stackoverflow.com/a/16824239/254035

template<typename, typename T>
struct has_member_function {
    static_assert(std::integral_constant<T,false>::value,
                  "Second template parameter needs to be of function type.");
};

// specialization that does the checking

template<typename C, typename Ret, typename... Args>
struct has_member_function<C, Ret(Args...)> {
private:
    template<typename T>
    static constexpr auto check(T*) -> typename std::is_same<
            decltype(std::declval<T>()(std::declval<Args>()...)), Ret>::type;

    template<typename>
    static constexpr std::false_type check(...);

    using type = decltype(check<C>(0));

public:
    static constexpr bool value = type::value;
};


template <typename P>
class Qplot;

template <typename T, typename P, typename U = void>
struct is_canvas_style {
    static constexpr bool value = false;
};

template <typename T, typename P>
struct is_canvas_style<T, P, std::enable_if_t<has_member_function<T, void(Qplot<P>&)>::value>> {
    static constexpr bool value = true;
};


template <typename T, typename P, typename U = void>
struct is_object_style {
    static constexpr bool value = false;
};

template <typename T, typename P>
struct is_object_style<T, P, std::enable_if_t<has_member_function<T, void(Qplot<P>&,const T&)>::value>> {
    static constexpr bool value = true;
};
