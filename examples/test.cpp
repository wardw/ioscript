#include <iostream>
#include <vector>

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

struct A {};
struct B {};
struct C {};
struct D {};

template <> struct has_styles<int>    { using type = variant<A,B>;   };
template <> struct has_styles<float>  { using type = variant<A,C>;   };
template <> struct has_styles<char>   { using type = variant<B,C,D>; };

void testing()
{
	using MyTypes = std::tuple<int,float,char>;

	cout << "MyTypes: " << objName(MyTypes{}) << endl;
	cout << "MyTypes mapped: " << objName(styles_from_types<MyTypes>{}) << endl;
}
