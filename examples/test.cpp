#include <iostream>
#include <vector>

#include "qplot/qplot.h"

using namespace std;

struct CanvasStyle {
	void operator()(Subprocess<Gnuplot>& gnuplot) const {}
};

struct ObjectStyle {
	using supported_types = std::tuple<int, float>;
};

struct DataObject
{
};

static_assert( !is_object_style<CanvasStyle>::value );
static_assert(  is_object_style<ObjectStyle>::value );
static_assert( !is_object_style<DataObject>::value );

void testing()
{
	cout << "is_canvas_style: " << is_canvas_style<CanvasStyle,Gnuplot>::value << endl;
	cout << "is_object_style: " << is_object_style<CanvasStyle,Gnuplot>::value << endl;
}
