#include "qplot/qplot.h"
#include "qplot/python.h"

#include <vector>
#include <map>
#include <random>

using namespace std;
using namespace qp;

namespace {

struct Point {
	float x;
	float y;
};

struct Header
{
	void operator()(Subprocess<Python>& python) const {
		python << "import matplotlib.pyplot as plt" << endl;
		python << "figtitle = 'Default title'" << endl;
	}
};

// The following LineChart, ScatterPlot & BarChart give examples to the
// three configurations a function object may take.

// Type 1. Bundle the Python code along with the data to be sent.
// Useful for most simple cases that can operate on a specific object.
struct LineChart
{
	// For selecting behaviour based on the type, the usual rules for overloading apply.
	template <typename T>
	void operator()(Subprocess<Python>& python, const vector<T>& obj) const
	{
        python <<
R"(
x = map(int, qp_data_in[0].readline().split())
plt.plot(x, 'o-')
)";
		for (int i=0; i<obj.size(); i++) {
			python.data_out(0) << obj[i] << ' ';
		}
		python.data_out(0) << endl;
	}

	void operator()(Subprocess<Python>& python, const map<int,int>& obj) const
	{
        python <<
R"(
x = map(int, qp_data_in[0].readline().split())
y = map(int, qp_data_in[1].readline().split())
plt.plot(x, y, 'o-')
)";
		for (auto elem : obj) {
			python.data_out(0) << elem.first << ' ';
			python.data_out(1) << elem.second << ' ';
		}
		python.data_out(0) << endl;
		python.data_out(1) << endl;
	}

	// Unfortunately you must always provide the `const T&` version anyway, even if it does nothing (todo)
	template <typename T>
	void operator()(Subprocess<Python>& python, const T& obj) const {}
};

// Type 2. Separate the Python code by moving the code to the one-argument `operator()` form.
// Leaves the two-argument form as a recurrance to handle any additional arguments passed.
// May be useful in the more general case when a plot doesn't relate to a single object.
struct ScatterPlot
{
	// All snippets must be default constructable
	ScatterPlot() {}
	ScatterPlot(int pointSize) : pointSize_(pointSize) {}

	void operator()(Subprocess<Python>& python) const
	{
        python <<
R"(
import math

x = map(float, qp_data_in[0].readline().split())
y = map(float, qp_data_in[1].readline().split())
z = [math.sqrt((a[0]-5)**2 + (a[1]-5)**2) for a in zip(x,y)]

plt.scatter(x, y, c=z, s=)" << pointSize_ << R"()
)";
	}

	void operator()(Subprocess<Python>& python, const vector<Point>& points)
	{
		assert(plotNum_ == 0);
		for (auto elem : points) {
			python.data_out(0) << elem.x << ' ';
			python.data_out(1) << elem.y << ' ';
		}
		python.data_out(0) << endl;
		python.data_out(1) << endl;
	}

	template <typename T>
	void operator()(Subprocess<Python>& python, const vector<T>& obj)
	{
		assert(plotNum_ < 2);
		for (int i=0; i<obj.size(); i++) {
			python.data_out(plotNum_) << obj[i] << ' ';
		}
		python.data_out(plotNum_) << endl;
		plotNum_++;
	}

	template <typename T>
	void operator()(Subprocess<Python>& python, const T& obj) const {}

private:
	int pointSize_ = 10;

	// WATCHIT: if you must mutate state between calls, the variable must be static and apply to the class as a whole
	// This isn't that great (todo)
	static int plotNum_;
};

int ScatterPlot::plotNum_ = 0;

void sendBarData(Subprocess<Python>& python, const std::vector<int>& obj)
{
	for (int i=0; i<obj.size(); i++) {
		python.data_out(0) << i      << ' ';
		python.data_out(1) << obj[i] << ' ';
	}
	python.data_out(0) << endl;
	python.data_out(1) << endl;
}

void sendBarData(Subprocess<Python>& python, const std::map<int,int>& obj)
{
	for (auto elem : obj) {
		python.data_out(0) << elem.first << ' ';
		python.data_out(1) << elem.second << ' ';
	}
	python.data_out(0) << endl;
	python.data_out(1) << endl;
}

// Type 3. Use both forms of `operator()`.
// In this example the one-argument form handles initialization local to the bar chart.
// The two argument form is then left to define reocurring behaviour for each argument (each bar plot).
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
x = map(int, qp_data_in[0].readline().split())
y = map(int, qp_data_in[1].readline().split())

width = 1.0/numPlots
xPos = plotNum * width

plt.bar([a+xPos for a in x], y, width-0.1, color=cols[plotNum])
plotNum += 1
)";
		// An additional indirection to handle sending data based on type
		sendBarData(python, obj);
	}
};

struct Title
{
	const char* title;
	void operator()(Subprocess<Python>& python) const {
		python << "figtitle = \"" << title << "\"\n"
		       << "plt.title(figtitle)" << endl;
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

struct SaveFig {
	void operator()(Subprocess<Python>& python) const {
		python << "plt.savefig(figtitle)" << endl;
	}
};

struct DefaultSnippet
{
	template <typename T>
	void operator()(Subprocess<Python>& python, const T& obj) const {
		clog << "Default snippet called, this `obj` doesn't refer to any other snippet" << endl;
	}
};

} // namespace


// Define the primary template to act as a 'default snippet' to match any type, if necessary
// Specializations below take precedence over this, as per C++'s template specialization rules
template <typename T>
struct qp::has_styles { using type = variant<DefaultSnippet>; };

// These maps our data types to the relevant code snippets they may be called with
template <typename T> struct has_styles<std::vector<T>>      { using type = variant<LineChart,BarChart,ScatterPlot>; };
template <>           struct has_styles<std::map<int,int>>   { using type = variant<LineChart,BarChart>; };

// Note: For any data object T that relates a set of 'style' alternatives (BarChart, LineChart etc), each
// alternative must implement operator()(Subprocess<P>&, const T& obj) for each possible T, regardless of
// which is chosen during a call to `run` (Extends from the rules of std::variant).
// This is why vector<Point> is bound separately, so LineChart & Barchart do not have to handle vector<Point>'s
template <>           struct has_styles<std::vector<Point>>  { using type = variant<ScatterPlot>; };

using MyTypes = std::tuple<vector<int>, map<int,int>, vector<Point>, double>;
using QpPython = Qplot<Python,MyTypes>;


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
	std::poisson_distribution<> poisson_dist(3);
	std::map<int,int> vals2, vals3;
	n = 0;
	while (n < 1000) {
		vals2[normal_dist(gen)]++;
		vals3[poisson_dist(gen)]++;
		n++;
	}

	QpPython qp(Header{});

	// LineChart is automatically the default, being the first alternative of the variant
    qp.plot(Title{"Example 1"}, vals1, vals2, vals3, SaveFig{});
    qp.plot(Title{"Example 2"}, BarChart{2}, vals2, vals3, SaveFig{});
    qp.plot(Title{"Example 3"}, BarChart{3}, vals1, vals2, vals3, LineChart{}, vals3, SaveFig{});

    // Use a lambda
	qp.plot(Title{"Example 4"}, vals2, [](Subprocess<Python>& py) {
        py << "plt.savefig('Example 4.png')\n";
    });

	std::exponential_distribution<> exp_dist(3);
    std::vector<Point> points(1000);
    n = 0;
    while (n < 1000) {
    	points[n].x = exp_dist(gen);
    	points[n].y = normal_dist(gen);
    	n++;
    }
 	qp.plot(Title{"Example 5"}, ScatterPlot{30}, points, SaveFig{});

	std::vector<int> series1{1,1,2,3,5,8,13,21,34,55,89};
	std::vector<int> series2{1,3,6,10,15,21,28,36,45,55,66};
	qp.plot(Subplot{211}, Title{"Example 6 (a)"}, ScatterPlot{50}, series1, series2,
		    Subplot{212}, Title{"Example 6 (b)"}, LineChart{}, series1, series2,
		    SaveFig{});

	// There's no specialized mapping to associate a `double` - this will call our default snippet
	qp.plot(42.f);
}
