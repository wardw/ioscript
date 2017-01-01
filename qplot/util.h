#pragma once

///*
#include <cxxabi.h>

namespace qp {

template <typename T>
std::string objName(const T& obj)
{
    std::ostringstream os;
    int info;
    os << abi::__cxa_demangle(typeid(obj).name(),0,0,&info);
    return os.str();
}
//*/

// Find wether a variant contains a given type

// Returns the index if the variant contains the type, or -1 otherwise
// Adapted from http://stackoverflow.com/a/25958302/254035

// The general template is undefined, we're only intereted in the specializations below
template <typename T, size_t N, typename Variant>
struct has_type;

// We've exhausted searching all variant elements - not found
template <typename T, size_t N>
struct has_type<T, N, variant<monostate>> : std::integral_constant<int,-1> {};

// A match: T matches the first T of the variant, now at position N
template <typename T, size_t N, typename... Ts>
struct has_type<T, N, variant<T, Ts...>> : std::integral_constant<int,N> {};

// No match: U is not T
template <typename T, size_t N, typename U, typename... Ts>
struct has_type<T, N, variant<U, Ts...>> : has_type<T, N+1, variant<Ts...>> {};

template <typename T, typename Variant>
using variant_contains_type = typename has_type<T, 0, Variant>::type;


// Todo: meta up with above

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



template <typename T>
struct is_variant : std::false_type {};

template <typename T, typename... Ts>
struct is_variant<variant<T,Ts...>>  : std::true_type {};

// test
static_assert( is_variant<variant<int,float>>::value,"");
static_assert(!is_variant<std::nullptr_t>::value, "");

template <typename StyleVariant, typename Style, int key>
void updateTuple(StyleVariant& styleVariant, const Style& style, std::integral_constant<int,key>)
{
    styleVariant = style;
}

template <typename StyleVariant, typename Style>
void updateTuple(StyleVariant& styleVariant, const Style& style, std::integral_constant<int,-1>)
{
    // std::clog << "The styles variant " << objName(styleVariant)
    //           << " does not contain the style " << objName(style) << std::endl;
}

template<class Style, class Tuple, std::size_t N>
struct TupleUpdater {
    static void update(Tuple& plotStyles, const Style& style)
    {
        using StyleVariant = std::tuple_element_t<N-1, Tuple>;

        static_assert(is_variant<StyleVariant>::value,
                      "The Styles template parameter in Qplot<Styles> must be a tuple containing only variants");
        using Result = variant_contains_type<Style, StyleVariant>;

        auto& styleVariant = std::get<N-1>(plotStyles);
        updateTuple(styleVariant, style, Result{});

        TupleUpdater<Style, Tuple, N-1>::update(plotStyles, style);
    }
};

template<class Style, class Tuple>
struct TupleUpdater<Style, Tuple, 1> {
    static void update(Tuple& plotStyles, const Style& style)
    {
        using StyleVariant = std::tuple_element_t<0, Tuple>;

        static_assert(is_variant<StyleVariant>::value,
                     "The Styles template parameter in Qplot<Styles> must be a tuple containing only variants");
        using Result = variant_contains_type<Style, StyleVariant>;

        auto& styleVariant = std::get<0>(plotStyles);
        updateTuple(styleVariant, style, Result{});
    }
};

// Todo: can't we just have a base case for when N=0?
template<class Style, class Tuple>
struct TupleUpdater<Style, Tuple, 0> {
    static void update(Tuple& plotStyles, const Style& style)
    {
    }
};


template <typename T>
void objAction(std::ostream& os, const T& obj)
{
    int info;
    os << objName(obj);
    // os << obj;
}

template<class Tuple, std::size_t N>
struct TuplePrinter {
    static void printTuple(std::ostream& os, const Tuple& t)
    {
        // print, then recurse
        objAction(os, std::get<N-1>(t));
        os << " ";
        TuplePrinter<Tuple, N-1>::printTuple(os, t);
    }
};

template<class Tuple>
struct TuplePrinter<Tuple, 1>{
    static void printTuple(std::ostream& os, const Tuple& t)
    {
        // some common operation on each object of the tuple
        objAction(os, std::get<0>(t));
        os << " (end)" << std::endl;
    }
};

template<class... Args>
void printTuple(std::ostream& os, const std::tuple<Args...>& t)
{
    TuplePrinter<decltype(t), sizeof...(Args)>::printTuple(os, t);
}


// Determine wether a plot method of the right type exists for a class
// Adapted from http://stackoverflow.com/a/16824239/254035

template<typename, typename T>
struct has_member_function {
    static_assert(std::integral_constant<T,false>::value,
                  "Second template parameter needs to be of function type.");
};

// Specialization that does the checking

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


// Determine between 'object styles' and 'canvas styles'
// + A 'canvas style' is any type which overloads operator() with the type `void(Qplot<P>&)`
// + An 'object style' works likewise, but for the type `void(Qplot<P>&, constT&)` -- but see note, below

template <typename P, typename S>
class Qplot;

template <typename T, typename P, typename U = void>
struct is_canvas_style {
    static constexpr bool value = false;
};

template <typename T, typename P>
struct is_canvas_style<T, P, std::enable_if_t<has_member_function<T, void(Subprocess<P>&)>::value>> {
    static constexpr bool value = true;
};

// This isn't great - there's probably a better way
// This tests for a second parameter accepting a `const T&` by checking it supports `void*`
template <typename T, typename P, typename U = void>
struct is_object_style {
    static constexpr bool value = false;
};

template <typename T, typename P>
struct is_object_style<T, P, std::enable_if_t<has_member_function<T, void(Subprocess<P>&,const void*&)>::value>> {
    static constexpr bool value = true;
};

} // namespace qp
