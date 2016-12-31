#pragma once

#include <cxxabi.h>

// Map updater

template <typename T> struct associated_styles;

template<class T, class Map, class ObjTypes, std::size_t N>
struct MapUpdater {
    static void update(Map& plotStyles, const T& style)
    {
		using Object = std::tuple_element_t<N-1, ObjTypes>;
        using StyleVariant = typename associated_styles<Object>::type::supported_styles;

        size_t key = associated_styles<Object>::type::id;
        plotStyles[key] = StyleVariant{style};

        MapUpdater<T, Map, ObjTypes, N-1>::update(plotStyles, style);
    }
};
 
template<class T, class Map, class ObjTypes>
struct MapUpdater<T, Map, ObjTypes, 1> {
    static void update(Map& plotStyles, const T& style)
    {
		using Object = std::tuple_element_t<0, ObjTypes>;
        using StyleVariant = typename associated_styles<Object>::type::supported_styles;

        size_t key = associated_styles<Object>::type::id;
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


template <typename T>
std::string objName(const T& obj)
{
    std::ostringstream os;
    int info;
    os << abi::__cxa_demangle(typeid(obj).name(),0,0,&info);
    return os.str();
}

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
// + An 'object style' is any type for which the nested type `supported_types` exists, and is a tuple with at least one tuple element
// + A 'canvas style' is any type which overloads operator() with the type `void(Qplot<P>&)`

template<class T>
struct Check {
    using type = void;
};

template<class T, class U = void>
struct is_object_style {
    static constexpr bool value = false;
};

template<class T>
struct is_object_style<T, typename std::enable_if<std::tuple_size<typename T::supported_types>::value != 0>::type> {
    static constexpr bool value = true;
};


template <typename P, typename S>
class Qplot;

template <typename T, typename P, typename U = void>
struct is_canvas_style {
    static constexpr bool value = false;
};

template <typename T, typename P>
struct is_canvas_style<T, P, std::enable_if_t<has_member_function<T, void(Qplot<P,void>&)>::value>> {
    static constexpr bool value = true;
};


// The below would be more consistent - and would allow constraining the selection to the current subprocess P
// But it's more fiddly since we also want to ensure that the client can correctly overload the `const T& obj` parameter as necessary

// template <typename T, typename P, typename U = void>
// struct is_object_style {
//     static constexpr bool value = false;
// };

// template <typename T, typename P>
// struct is_object_style<T, P, std::enable_if_t<has_member_function<T, void(Qplot<P>&,const T&)>::value>> {
//     static constexpr bool value = true;
// };
