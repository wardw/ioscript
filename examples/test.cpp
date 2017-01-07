#include <iostream>
#include <vector>
#include <map>

#include "ioscript/ioscript.h"
#include "ioscript/gnuplot.h"
#include "ioscript/python.h"

using namespace std;
using namespace iosc;

static_assert(tuple_element_index<int,    std::tuple<int,char,float>>::value ==  0, "");
static_assert(tuple_element_index<char,   std::tuple<int,char,float>>::value ==  1, "");
static_assert(tuple_element_index<float,  std::tuple<int,char,float>>::value ==  2, "");
static_assert(tuple_element_index<double, std::tuple<int,char,float>>::value == -1, "");

static_assert(variant_element_index<int,    iosc::variant<int,char,float>>::value ==  0, "");
static_assert(variant_element_index<char,   iosc::variant<int,char,float>>::value ==  1, "");
static_assert(variant_element_index<float,  iosc::variant<int,char,float>>::value ==  2, "");
static_assert(variant_element_index<double, iosc::variant<int,char,float>>::value == -1, "");

static_assert( is_variant<variant<int,float>>::value,"");
static_assert(!is_variant<std::nullptr_t>::value, "");

struct CanvasStyle {
	void operator()(Process<Gnuplot>&) const {}
};

struct ObjectStyle {
	template <typename T>
	void operator()(Process<Gnuplot>&, const T&) const {}
};

struct DataObject
{
};

static_assert(!is_object_snippet<CanvasStyle,Gnuplot>::value, "");
static_assert( is_object_snippet<ObjectStyle,Gnuplot>::value, "");
static_assert(!is_object_snippet<DataObject,Gnuplot>::value, "");

static_assert( is_script_snippet<CanvasStyle,Gnuplot>::value, "");
static_assert(!is_script_snippet<ObjectStyle,Gnuplot>::value, "");
static_assert(!is_script_snippet<DataObject,Gnuplot>::value, "");

struct A {};
struct B {};
struct C {};

template <typename T> struct binds_to<std::vector<T>>      { using type = variant<A,B>; };
template <>           struct binds_to<std::vector<int>>    { using type = variant<B>; };
template <>           struct binds_to<std::map<int,int>>   { using type = variant<A,C>; };

using MyTypes = std::tuple<vector<float>,vector<int>,map<int,int>>;

static_assert(is_same< tuple<variant<A,B>, variant<B>, variant<A,C>>,
	                   snippets_from_types<MyTypes>::type
	                 >::value, "");

struct NoChance {};
// using check = check_tuple<tuple<vector<int>,map<int,int>, NoChance>>;  // should static_assert

static_assert( has_related_snippet<vector<int>>::value, "");
static_assert( has_related_snippet<vector<float>>::value, "");
static_assert(!has_related_snippet<char>::value, "");


// Function object requirements

struct Snippet
{
	// Required
	Snippet() {}
    Snippet& operator=(const Snippet& rhs) { return *this; }
    Snippet(const Snippet& rhs) {}

    void operator()(Process<Python>& python) const {}
};

template <> struct binds_to<int> { using type = variant<Snippet>; };
using RequirementsTestTypes = std::tuple<int>;

void testing()
{
	Script<Python,RequirementsTestTypes> script;
	script.run(Snippet{});
}
