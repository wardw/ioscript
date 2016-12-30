#include "gnuplot_styles_common.h"
#include "qplot/qplot.h"
#include <vector>

using namespace std;

template <typename T>
void sendData(Qplot<Gnuplot>& gnuplot, const std::vector<T>& obj)
{
    for (int i=0; i<obj.size(); ++i) {
        gnuplot << std::to_string(i) << " " << std::to_string(obj[i]) << "\n";
    }
}

struct Header
{
	void operator()(Qplot<Python>& python) const {
        python <<
R"(
import os
os.close( )" << python.fd_w() << R"( )
fo = os.fdopen( )" << python.fd_r() << R"(, 'r')
)"
		<< "\n";
	}
};

struct BarChart
{
	using supported_types = std::tuple<std::vector<int>,std::vector<float>>;

	template<typename T>
	void operator()(Qplot<Gnuplot>& gnuplot, const T& obj) const
	{
        gnuplot << "plot '-' using 1:2\n";
        sendData(gnuplot, obj);
        gnuplot << "e\n";
	}

	template<typename T>
	void operator()(Qplot<Python>& python, const T& obj) const
	{
		python << "print \"(python) todo: plot BarChart with " + objName(obj) + "\"\n";
	}
};

struct LineChart
{
	using supported_types = std::tuple<std::vector<int>,std::vector<float>>;

	template<typename T>
	void operator()(Qplot<Gnuplot>& gnuplot, const T& obj) const
	{
        gnuplot << "plot '-' using 1:2\n";
        sendData(gnuplot, obj);
        gnuplot << "e\n";
	}

	template<typename T>
	void operator()(Qplot<Python>& python, const T& obj) const
	{
        python <<
R"(
for line in fo:
	if line == 'EOT\n':
		break
	print "(Python) " + line,
print "(Python) Done"
)" << "\n";

		for (auto elem : obj)
		{
			python.fdout() << elem << '\n';
		}
		python.fdout() << "EOT" << '\n';
	}
};

struct Data1d { static constexpr size_t id = 0; using supported_styles = std::variant<LineChart, BarChart>; };

template <> struct associated_styles<std::vector<int>>   { using type = Data1d; };
template <> struct associated_styles<std::vector<float>> { using type = Data1d; };


void example1()
{
	std::vector<int> ints = {1,2,3};

	// Watchit: style must but passed first since we no longer have the chartStyles tuple for default initialzation
	// Watchit: todo MPL example wait issue - hard coded pipe fd's

	Qplot<Gnuplot> qplot1(LineChart{});
	// qplot1.plot(ints, AxisExtents{{0,1}, {0,2}}, ImageSize{400,300}, ints);
    // qplot1.plot(ints, BarChart(), ints, ints, LineChart(), ints);
    qplot1.plot(Filename("data1d"), ints);
    qplot1.plot(Filename("data1d2"), ints);
    // qplot1.plot(Filename("data1d"), LineChart(), ints);

	Qplot<Python> qplot2(Header{});
	// qplot2.plot(ints, AxisExtents{{0,1}, {0,2}}, ImageSize{400,300}, ints);
    // qplot2.plot(ints, ints, ints, LineChart(), ints);
    qplot2.plot(LineChart(), ints);
    qplot2.plot(LineChart(), ints, ints);
    qplot2.plot(LineChart(), ints);
}
