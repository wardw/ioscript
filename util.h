#pragma once


// http://stackoverflow.com/a/25958302/254035
// Adapted to return the index if the type exists, or -1

template <typename T, size_t N, typename Tuple>
struct has_type;

template <typename T, size_t N>
struct has_type<T, N, std::tuple<>> : std::integral_constant<int,-1> {};

template <typename T, size_t N, typename... Ts>
struct has_type<T, N, std::tuple<T, Ts...>> : std::integral_constant<int,N> {};

template <typename T, size_t N, typename U, typename... Ts>
struct has_type<T, N, std::tuple<U, Ts...>> : has_type<T, N+1, std::tuple<Ts...>> {};

template <typename T, typename Tuple>
using tuple_contains_type = typename has_type<T, 0, Tuple>::type;


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
