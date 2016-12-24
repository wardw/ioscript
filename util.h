#pragma once

#include <cxxabi.h>
#include <sstream>

// Type info
template <typename T>
std::string objName(const T& obj)
{
	std::ostringstream os;
	int info;
    os << abi::__cxa_demangle(typeid(obj).name(),0,0,&info) << std::endl;
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



// has_foo<T> tests wether T::foo exists as a nested type
// Modified from http://stackoverflow.com/a/11816999/254035

template<class T>
struct TestType {
    using type = void;
};

template<class T, class U = void>
struct has_foo {
    static constexpr bool value = false;
};

// The idea is that if TestType<T> is a valid substitution (and so T exists), this template is enabled
template<class T>
struct has_foo<T, typename TestType<typename T::foo>::type> {
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


// http://en.cppreference.com/w/cpp/utility/tuple/tuple

class Gnuplot;

template<class Tuple, std::size_t N>
struct TuplePrinter {
    static void writeStyleString(Gnuplot& gnuplot, const Tuple& t)
    {
        TuplePrinter<Tuple, N-1>::writeStyleString(gnuplot, t);
        gnuplot << std::get<N-1>(t).c_str();
    }
};
 
template<class Tuple>
struct TuplePrinter<Tuple, 1>{
    static void writeStyleString(Gnuplot& gnuplot, const Tuple& t)
    {
        gnuplot << std::get<0>(t).c_str();
    }
};
 
template<class... Args>
void writeStyleString(Gnuplot& gnuplot, const std::tuple<Args...>& t)
{
    TuplePrinter<decltype(t), sizeof...(Args)>::writeStyleString(gnuplot, t);
}

template<class T>
void writeStyleString(Gnuplot& gnuplot, const T& styleString)
{
    gnuplot << styleString.c_str();
}
