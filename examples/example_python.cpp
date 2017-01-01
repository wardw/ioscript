#include "qplot/qplot.h"
#include <vector>
#include <map>
#include <random>

using namespace std;

// Perhaps sufficient for now. The important point is to close the (inherited) write end of the pipe
// in this subprocess and avoid it waiting on itself when reading up to EOF on the read end
struct Header
{
	void operator()(Subprocess<Python>& python) const {
        python <<
R"(
import os
os.close( )" << python.fd_w() << R"( )
fo = os.fdopen( )" << python.fd_r() << R"(, 'r')
)"
		<< "\n";
	}
};

struct ClientHeader
{
	void operator()(Subprocess<Python>& python) const {
		python << "import matplotlib.pyplot as plt" << '\n';
	}
};

void sendData(Subprocess<Python>& python, const std::vector<int>& obj)
{
	// Just send a line each for x and y that matches our python read
    for (int i=0; i<obj.size(); i++) {
		python.fdout() << i << ' ';
	}
	python.fdout() << endl;

	for (int i=0; i<obj.size(); i++) {
		python.fdout() << obj[i] << ' ';
	}
	python.fdout() << endl;
}

void sendData(Subprocess<Python>& python, const std::map<int,int>& obj)
{
    for (auto elem : obj) {
		python.fdout() << elem.first << ' ';
	}
	python.fdout() << endl;

	for (auto elem : obj) {
		python.fdout() << elem.second << ' ';
	}
	python.fdout() << endl;
}

struct LineChart
{
	// Here we just catch all T, but you can define separate overloads to select on
	// specific types using the usual rules for overloading.  Unfortunately you must
	// always provide the `const T&` version anyway, even if it does nothing (todo!)
	template <typename T>
	void operator()(Subprocess<Python>& python, const T& obj) const
	{
        python <<
R"(
x = map(int, fo.readline().split())
y = map(int, fo.readline().split())

plt.plot(x, y, 'o-')
plt.plot(x, y, 'o-')
)" << "\n";

		sendData(python, obj);
	}
};

// This example includes both forms of `operator()`
// This admits separating initialization code in a 'header' from the code body
// taking the steady-state behaviour on the given argument
struct BarChart
{
	int numPlots = 1;

	void operator()(Subprocess<Python>& python) const
	{
		python << "numPlots = " << numPlots << '\n'
			   << "assert numPlots < 7" << '\n'
			   << "cols = ['b', 'g', 'r', 'c', 'm', 'y', 'k']" << '\n'
		       << "plotNum = 0" << '\n';
	}

	template <typename T>
	void operator()(Subprocess<Python>& python, const T& obj) const
	{
        python <<
R"(
x = map(int, fo.readline().split())
y = map(int, fo.readline().split())

width = 1.0/numPlots
xPos = plotNum * width

plt.bar([a+xPos for a in x], y, width-0.1, color=cols[plotNum])
plotNum += 1
)" << "\n";

		sendData(python, obj);
	}
};

struct Show {
	void operator()(Subprocess<Python>& python) const { python << "plt.show()\n"; }
};


using Data1D = std::variant<LineChart,BarChart>;

// Consider all vectors as Data1D, and also std::map<int,int>
template <typename T> struct has_styles<std::vector<T>>     { using type = Data1D; };
template <>           struct has_styles<std::map<int,int>>  { using type = Data1D; };

using MyStyles = std::tuple<Data1D>;
using Qp = Qplot<Python,MyStyles>;

void example_python()
{
	std::default_random_engine gen;

	std::uniform_int_distribution<> uniform_dist(0,10);
	std::vector<int> vals1(10);
	int n = 0;
	while (n < 1000) {
		vals1[uniform_dist(gen)]++;
		n++;
	}

	std::normal_distribution<> normal_dist(5, 1.5);
	std::map<int,int> vals2;
	n = 0;
	while (n < 1000) {
		vals2[normal_dist(gen)]++;
		n++;
	}

	Qp qp(Header{}, ClientHeader{});
    qp.plot(vals1, vals2, Show());  // LineChart is automatically the default (the first variant alternative)
    qp.plot(BarChart{2}, vals1, vals2, Show{});
    qp.plot(LineChart{}, vals2, BarChart{1}, vals1, Show{});

    // Or use a lambda
	qp.plot(LineChart{}, vals2, [](Subprocess<Python>& py) {
        py << "plt.savefig('vals2.png')" << '\n';
    });
}

// Todo: put somewhere
// The one requirement is that if an data object T supports a set of 'style' variants (BarChart, LineChart etc), each 'style'
// must implement operator()(Subprocess<P>&, T obj) for plot object T, regardless of the currently chosen alternative
