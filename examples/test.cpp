#include <iostream>
#include <vector>

#include "qplot/qplot.h"

using namespace std;

struct CanvasStyle
{
	using supported_types = std::tuple<>;

	CanvasStyle(const std::string& filename) : filename(filename) {}

	void operator()(Qplot<Gnuplot>& gnuplot) const
	{
        gnuplot << "set output '" << filename << ".png'\n"
                << "set terminal pngcairo size 500, 500\n";
    }
	std::string filename;
};

struct ObjectStyle
{
	using supported_types = std::tuple<std::vector<int>,std::vector<float>>;

	template<typename T>
	void operator()(Qplot<Gnuplot>& gnuplot, const T& obj) const
	{
        gnuplot << "plot '-' using 1:2\n";
        sendData(gnuplot, obj);
        gnuplot << "e\n";
	}
};

struct DataObject
{
};


struct CanvasStyle2 {
	void operator()(Qplot<Gnuplot>& gnuplot) const {}
};

struct ObjectStyle2 {
	template<typename T>
	void operator()(Qplot<Gnuplot>& gnuplot, const T& obj) const {}
};


void test()
{
	assert(  is_style<CanvasStyle>::value );
	assert(  is_style<ObjectStyle>::value );
	assert( !is_style<DataObject>::value );

	assert( !has_supported_types<CanvasStyle>::value );
	assert(  has_supported_types<ObjectStyle>::value );
	assert( !has_supported_types<DataObject>::value );

	cout << "is_canvas_style: " << is_canvas_style<CanvasStyle2,Gnuplot>::value << endl;
	cout << "is_canvas_style: " << is_canvas_style<ObjectStyle2,Gnuplot>::value << endl;

	cout << "is_object_style: " << is_object_style<CanvasStyle2,Gnuplot>::value << endl;
	cout << "is_object_style: " << is_object_style<ObjectStyle2,Gnuplot>::value << endl;
}
