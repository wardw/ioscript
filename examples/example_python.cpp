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

// The one requirement is that if an data object T supports a set of 'style' variants (BarChart, LineChart etc), each 'style'
// must implement operator()(Subprocess<P>&, T obj) for plot object T, regardless of the currently chosen alternative
struct BarChart
{
	// Here we just catch all T
	// (But defining separate overloads to select individually on each possible type works too.
	//  Just make sure to provide defintions for each possible object T that's passed)
	template <typename T>
	void operator()(Subprocess<Python>& python, const T& obj) const
	{
        python <<
R"(
import matplotlib.pyplot as plt

x = map(int, fo.readline().split())
y = map(int, fo.readline().split())

if 'numPlots' not in locals():
	numPlots = 1
	plotNum = 1
width = 1.0/numPlots
xPos = plotNum * width

assert numPlots < 7, "No! If only because we've only enumerated 7 colours"
cols = ['b', 'g', 'r', 'c', 'm', 'y', 'k']

plt.bar([a+xPos for a in x], y, width-0.1, color=cols[plotNum])
plotNum += 1
)" << "\n";

		sendData(python, obj);
	}
};

struct LineChart
{
	template <typename T>
	void operator()(Subprocess<Python>& python, const T& obj) const
	{
        python <<
R"(
import matplotlib.pyplot as plt

x = map(int, fo.readline().split())
y = map(int, fo.readline().split())

plt.plot(x, y, 'o-')
plt.plot(x, y, 'o-')
)" << "\n";

		sendData(python, obj);
	}
};

struct NumPlots {
	void operator()(Subprocess<Python>& python) const {
		python << "numPlots = " << numPlots << '\n'
		       << "plotNum = 0" << '\n';
	}
	int numPlots{};
};

struct Show {
	void operator()(Subprocess<Python>& python) const { python << "plt.show()\n"; }
};


using Data1D = std::variant<LineChart,BarChart>;

template <> struct has_styles<std::vector<int>>  { using type = Data1D; };
template <> struct has_styles<std::map<int,int>> { using type = Data1D; };

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

	Qp qp(Header{});
    qp.plot(vals1, vals2, Show());  // LineChart is automatically the default (the first variant alternative)
    qp.plot(BarChart{}, NumPlots{2}, vals1, vals2, Show{});
    qp.plot(LineChart{}, vals2, BarChart{}, NumPlots{1}, vals1, Show{});

    // Or use a lambda
	qp.plot(LineChart{}, vals2, [](Subprocess<Python>& py) {
        py << "plt.savefig('vals2.png')" << '\n';
    });
}
