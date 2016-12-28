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
// Modified from http://stackoverflow.com/a/11816999/254035

template<class T>
struct TestType {
    using type = void;
};

template<class T, class U = void>
struct has_supported_types {
    static constexpr bool value = false;
};

// The idea is that if TestType<T> is a valid substitution (and so T exists), this template is enabled
template<class T>
struct has_supported_types<T, typename TestType<typename T::supported_types>::type> {
    static constexpr bool value = true;
};

// has_pass<T> tests wether T::pass exists as a static constant (substituable with int N) 

template<int N>
struct TestNontype {
    using type = void;
};

template<class T, class U = void>
struct has_pass {
    static constexpr bool value = false;
};

// The idea is that if TestNontype<T> is a valid substitution (and so T exists), this template is enabled
template<class T>
struct has_pass<T, typename TestNontype<T::pass>::type> {
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
struct has_plot {
    static_assert(std::integral_constant<T,false>::value,
                  "Second template parameter needs to be of function type.");
};

// specialization that does the checking

template<typename C, typename Ret, typename... Args>
struct has_plot<C, Ret(Args...)> {
private:
    template<typename T>
    static constexpr auto check(T*) -> typename std::is_same<
            decltype(std::declval<T>().plot(std::declval<Args>()...)), Ret
            >::type;

    template<typename>
    static constexpr std::false_type check(...);

    using type = decltype(check<C>(0));

public:
    static constexpr bool value = type::value;
};
