#include "qplot/qplot.h"
#include <vector>
#include <map>
#include <random>

using namespace std;
using namespace qp;

struct Point {
	float x;
	float y;
};

struct Header
{
	void operator()(Subprocess<Python>& python) const {
		python << "import matplotlib.pyplot as plt" << endl;
	}
};

// Define a collection of functions that interpret our data types along dimensions that are meaningful
// This isn't necessary for the examples (we could bundle inside any plot snippet), but useful later

template <size_t N, typename T>
void sendData(Subprocess<Python>& python, const T& obj);

// Using the convention that a single dimension is a line of data
template <>
void sendData<0>(Subprocess<Python>& python, const std::vector<int>& obj)
{
	for (int i=0; i<obj.size(); i++) {
		python.data_out() << obj[i] << ' ';
	}
	python.data_out() << endl;
}

template <>
void sendData<1>(Subprocess<Python>& python, const std::vector<int>& obj)
{
    for (int i=0; i<obj.size(); i++) {
		python.data_out() << i << ' ';
	}
	python.data_out() << endl;
}

template <>
void sendData<0>(Subprocess<Python>& python, const std::map<int,int>& obj)
{
	for (auto elem : obj) {
		python.data_out() << elem.second << ' ';
	}
	python.data_out() << endl;
}

template <>
void sendData<1>(Subprocess<Python>& python, const std::map<int,int>& obj)
{
	for (auto elem : obj) {
		python.data_out() << elem.first << ' ';
	}
	python.data_out() << endl;
}

template <>
void sendData<0>(Subprocess<Python>& python, const std::vector<Point>& obj)
{
	for (auto elem : obj) {
		python.data_out() << elem.x << ' ';
	}
	python.data_out() << endl;
}

template <>
void sendData<1>(Subprocess<Python>& python, const std::vector<Point>& obj)
{
	for (auto elem : obj) {
		python.data_out() << elem.y << ' ';
	}
	python.data_out() << endl;
}

struct LineChart
{
	// Here we just match all T, where the usual rules for overloading apply should
	// you want different behaviour for different types. Unfortunately you must always
	// provide the `const T&` version anyway, even if it does nothing (todo)
	template <typename T>
	void operator()(Subprocess<Python>& python, const T& obj) const
	{
        python <<
R"(
x = map(int, qp_data_in.readline().split())
y = map(int, qp_data_in.readline().split())

plt.plot(x, y, 'o-')
plt.plot(x, y, 'o-')
)" << endl;

		// Here (by exampe) we say there's really just one format we care about.
		// So bundle here with the plotting code. See ScatterPlot for a more general approach
		sendData<1>(python, obj);
		sendData<0>(python, obj);
	}
};

// This example includes both forms of `operator()`
// Here we separate initialization code in a 'header' and leave the two
// argument form to take the recurring behaviour on each data argument
struct BarChart
{
	int numPlots = 1;

	void operator()(Subprocess<Python>& python) const
	{
		python
			<< "numPlots = " << numPlots << '\n'
			<< "assert numPlots < 7\n"
			<< "cols = ['b', 'g', 'r', 'c', 'm', 'y', 'k']\n"
			<< "plotNum = 0\n";
	}

	template <typename T>
	void operator()(Subprocess<Python>& python, const T& obj) const
	{
        python <<
R"(
x = map(int, qp_data_in.readline().split())
y = map(int, qp_data_in.readline().split())

width = 1.0/numPlots
xPos = plotNum * width

plt.bar([a+xPos for a in x], y, width-0.1, color=cols[plotNum])
plotNum += 1
)" << endl;

		sendData<1>(python, obj);
		sendData<0>(python, obj);
	}
};


// In this example we separate the plotting code from the data
// This is useful in the more general case when a plot doesn't relate to a single object
struct ScatterPlot
{
	int pointSize = 10;

	void operator()(Subprocess<Python>& python) const
	{
        python <<
R"(
x = map(float, qp_data_in.readline().split())
y = map(float, qp_data_in.readline().split())
plt.scatter(x, y, s=)" << pointSize << R"()
)" << endl;
	}

	// These functions essentially reduce to expressing defaults for how to interpret particular types
	template <typename T>
	void operator()(Subprocess<Python>& python, const T& obj) const {
		sendData<0>(python, obj);
	}

	void operator()(Subprocess<Python>& python, const vector<Point>& obj) const {
		sendData<0>(python, obj);
		sendData<1>(python, obj);
	}
};

struct Subplot
{
	int subplot;
	void operator()(Subprocess<Python>& python) const {
		python << "plt.subplot(" << subplot << ")\n";
	}
};

struct Show {
	void operator()(Subprocess<Python>& python) const { python << "plt.show()\n"; }
};

using Data1D = variant<LineChart,BarChart,ScatterPlot>;
using Data2D = variant<ScatterPlot>;

// Consider all vectors as Data1D, and also std::map<int,int>
template <typename T> struct has_styles<std::vector<T>>     { using type = Data1D; };
template <>           struct has_styles<std::map<int,int>>  { using type = Data1D; };
template <>           struct has_styles<std::vector<Point>> { using type = Data2D; };

using MyStyles = std::tuple<Data1D,Data2D>;
using Qp = Qplot<Python,MyStyles>;

void example_python()
{
	std::default_random_engine gen;

	std::uniform_int_distribution<> uniform_int(0,10);
	std::vector<int> vals1(10);
	int n = 0;
	while (n < 1000) {
		vals1[uniform_int(gen)]++;
		n++;
	}

	std::normal_distribution<> normal_dist(5, 1.5);
	std::map<int,int> vals2;
	n = 0;
	while (n < 1000) {
		vals2[normal_dist(gen)]++;
		n++;
	}

	Qp qp(Header{});
    qp.plot(vals1, vals2, Show());  // LineChart is automatically the default (the first variant alternative)
    qp.plot(BarChart{2}, vals1, vals2, Show{});
    qp.plot(LineChart{}, vals2, BarChart{1}, vals1, Show{});

    // Or use a lambda
	qp.plot(LineChart{}, vals2, [](Subprocess<Python>& py) {
        py << "plt.savefig('vals2.png')\n";
    });


    std::vector<Point> points(1000);
    n = 0;
    while (n < 1000) {
    	points[n].x = normal_dist(gen);
    	points[n].y = normal_dist(gen);
    	n++;
    }
 	qp.plot(ScatterPlot{50}, points, Show{});


	std::vector<int> series1{1,1,2,3,5,8,13,21,34,55,89};
	std::vector<int> series2{1,3,6,10,15,21,28,36,45,55,66};
	qp.plot(Subplot{211}, ScatterPlot{50}, series1, series2, Subplot{212}, LineChart{}, series1, series2, Show{});

 	// But what if we don't want to use the default format? Then specify inline
 	qp.plot(ScatterPlot{50}, [points](Subprocess<Python>& py) {
 		sendData<1>(py, points);
 		sendData<0>(py, points);
 	},
 	Show{});
}

// Left out
// The one requirement is that given a data object T that relates a set of 'style' variants (BarChart, LineChart etc), each
// variant alternative must implement operator()(Subprocess<P>&, T obj) for data object T regardless of which alternative
// is the current chosen alternative. (Extends from the rules of std::variant)
