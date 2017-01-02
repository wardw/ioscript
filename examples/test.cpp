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

void testing()
{
}
