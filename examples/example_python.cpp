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

// Define a basic set of functions to interpret our data types along meaningful dimesions of interest
// These examples use the convention that a line of data is a single dimension to python
void sendData(Subprocess<Python>& python, const std::vector<int>& obj)
{
	for (int i=0; i<obj.size(); i++) {
		python.data_out() << obj[i] << ' ';
	}
	python.data_out() << endl;
}

void sendData(Subprocess<Python>& python, const std::vector<Point>& obj)
{
	for (auto elem : obj) {
		python.data_out() << elem.x << ' ';
	}
	python.data_out() << endl;

	for (auto elem : obj) {
		python.data_out() << elem.y << ' ';
	}
	python.data_out() << endl;
}

void sendData(Subprocess<Python>& python, const std::map<int,int>& obj)
{
	for (auto elem : obj) {
		python.data_out() << elem.first << ' ';
	}
	python.data_out() << endl;

	for (auto elem : obj) {
		python.data_out() << elem.second << ' ';
	}
	python.data_out() << endl;
}

// This exmample separates the plotting code depending on the data type.
// The usual rules for overloading apply.  Unfortunately you must always provide
// the `const T&` version anyway, even if it does nothing (todo)
struct LineChart
{
	void operator()(Subprocess<Python>& python, const vector<int>& obj) const
	{
        python <<
R"(
x = map(int, qp_data_in.readline().split())
plt.plot(x, 'o-')
)";
		sendData(python, obj);
	}

	// template <typename T>
	void operator()(Subprocess<Python>& python, const map<int,int>& obj) const
	{
        python <<
R"(
x = map(int, qp_data_in.readline().split())
y = map(int, qp_data_in.readline().split())
plt.plot(x, y, 'o-')
)";
		sendData(python, obj);
	}

	template <typename T>
	void operator()(Subprocess<Python>& python, const T& obj) const {}
};

// This example uses both forms of `operator()`
// We put initialization code in a 'header' and leave the two argument form
// to take recurring behaviour applied to each data argument that's passed
struct BarChart
{
	int numPlots = 1;

	void operator()(Subprocess<Python>& python) const
	{
		python <<
R"(
numPlots =)" << numPlots << R"(
assert numPlots < 7
cols = ['b', 'g', 'r', 'c', 'm', 'y', 'k']
plotNum = 0
)";
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
)";
		sendData(python, obj);
	}
};


// This example completely separates the plotting code from the data
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
)";
	}

	template <typename T>
	void operator()(Subprocess<Python>& python, const T& obj) const {
		sendData(python, obj);
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

// These maps our data types to the relevant code snippet 'styles' they may be called with
template <typename T>
struct has_styles<std::vector<T>>     { using type = variant<LineChart,ScatterPlot>; };

template <>
struct has_styles<std::map<int,int>>  { using type = variant<LineChart,BarChart>; };

template <>
struct has_styles<std::vector<Point>> { using type = variant<ScatterPlot>; };

// Or cheat - associate all types to all styles - just take care to make sure every style
// defines operator() for every type (which it must regardless of the current value of the variant)
// using AllStyles = variant<LineChart,BarChart,ScatterPlot>;
// template <typename T>
// struct qp::has_styles { using type = AllStyles; };


using MyTypes = std::tuple<vector<int>,map<int,int>,vector<Point>>;
using Qp = Qplot<Python,MyTypes>;

void example_python()
{
	cout << "MyTypes: " << objName(styles_from_types<MyTypes>{}) << endl;

	std::default_random_engine gen;

	std::uniform_int_distribution<> uniform_int(0,10);
	std::vector<int> vals1(10);
	int n = 0;
	while (n < 1000) {
		vals1[uniform_int(gen)]++;
		n++;
	}

	std::normal_distribution<> normal_dist(5, 1.5);
	std::poisson_distribution<> poisson_dist(3);
	std::map<int,int> vals2, vals3;
	n = 0;
	while (n < 1000) {
		vals2[normal_dist(gen)]++;
		vals3[poisson_dist(gen)]++;
		n++;
	}

	Qp qp(Header{});

	// LineChart is automatically the default as the first variant alternative
    qp.plot(vals1, vals2, vals3, Show());
    qp.plot(BarChart{2}, vals2, vals3, Show{});
    qp.plot(BarChart{1}, vals2, LineChart{}, vals3, Show{});

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
 	// qp.plot(ScatterPlot{50}, [points](Subprocess<Python>& py) {
 	// 	sendData<1>(py, points);
 	// 	sendData<0>(py, points);
 	// },
 	// Show{});
}

// But one requirement is that for any data object T that relates a set of 'style' variants (BarChart, LineChart etc), each
// alternative must implement operator()(Subprocess<P>&, T obj) for each possible T regardless of which alternative
// is the current chosen alternative. (Extends from the rules of std::variant)
