#include <iostream>
#include <vector>
#include <map>

#include "qplot/qplot.h"

using namespace std;
using namespace qp;

struct CanvasStyle {
	void operator()(Subprocess<Gnuplot>&) const {}
};

struct ObjectStyle {
	template <typename T>
	void operator()(Subprocess<Gnuplot>&, const T&) const {}
};

struct DataObject
{
};

static_assert(!is_object_style<CanvasStyle,Gnuplot>::value, "");
static_assert( is_object_style<ObjectStyle,Gnuplot>::value, "");
static_assert(!is_object_style<DataObject,Gnuplot>::value, "");

static_assert( is_canvas_style<CanvasStyle,Gnuplot>::value, "");
static_assert(!is_canvas_style<ObjectStyle,Gnuplot>::value, "");
static_assert(!is_canvas_style<DataObject,Gnuplot>::value, "");

static_assert(tuple_element_index<int,    std::tuple<int,char,float>>::value ==  0, "");
static_assert(tuple_element_index<char,   std::tuple<int,char,float>>::value ==  1, "");
static_assert(tuple_element_index<float,  std::tuple<int,char,float>>::value ==  2, "");
static_assert(tuple_element_index<double, std::tuple<int,char,float>>::value == -1, "");

static_assert(variant_element_index<int,    std::variant<int,char,float>>::value ==  0, "");
static_assert(variant_element_index<char,   std::variant<int,char,float>>::value ==  1, "");
static_assert(variant_element_index<float,  std::variant<int,char,float>>::value ==  2, "");
static_assert(variant_element_index<double, std::variant<int,char,float>>::value == -1, "");

static_assert( is_variant<variant<int,float>>::value,"");
static_assert(!is_variant<std::nullptr_t>::value, "");

struct A {};
struct B {};
struct C {};

template <typename T> struct has_styles<std::vector<T>>      { using type = variant<A,B>; };
template <>           struct has_styles<std::vector<int>>    { using type = variant<B>; };
template <>           struct has_styles<std::map<int,int>>   { using type = variant<A,C>; };

using MyTypes = std::tuple<vector<float>,vector<int>,map<int,int>>;

static_assert(is_same< tuple<variant<A,B>, variant<B>, variant<A,C>>,
	                   styles_from_types<MyTypes>::type
	                 >::value, "");

struct NoChance {};
// using check = check_tuple<tuple<vector<int>,map<int,int>, NoChance>>;

static_assert( has_related_style<vector<int>>::value, "");
static_assert( has_related_style<vector<float>>::value, "");
static_assert(!has_related_style<char>::value, "");

void testing()
{
}
