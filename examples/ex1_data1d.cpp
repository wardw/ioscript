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
	using supported_types = std::tuple<>;

	void plot(Qplot<Mpl>& mpl) const {
        mpl <<
R"(
import os
os.close(5)
fo = os.fdopen(3, 'r')
)"
		<< "\n";
	}
};

struct BarChart
{
	using supported_types = std::tuple<std::vector<int>,std::vector<float>>;

	template<typename T>
	void plot(Qplot<Gnuplot>& gnuplot, const T& obj) const
	{
        gnuplot << "plot '-' using 1:2\n";
        sendData(gnuplot, obj);
        gnuplot << "e\n";
	}

	template<typename T>
	void plot(Qplot<Mpl>& mpl, const T& obj) const
	{
		mpl << "print \"(python) todo: plot BarChart with " + objName(obj) + "\"\n";
	}
};

struct LineChart
{
	using supported_types = std::tuple<std::vector<int>,std::vector<float>>;

	// template<typename T>
	// void plot(Qplot<Gnuplot>& qp, const T& obj) const
	// {
 //        qp << "plot '-' using 1:2\n";
 //        sendData(qp, obj);
 //        qp << "e\n";
	// }

	template<typename T>
	void plot(Qplot<Gnuplot>& gnuplot, const T& obj) const
	{
        gnuplot << "plot '-' using 1:2\n";
        sendData(gnuplot, obj);
        gnuplot << "e\n";
	}

	template<typename T>
	void plot(Qplot<Mpl>& mpl, const T& obj) const
	{
        mpl <<
R"(
for line in fo:
	if line == 'EOT\n':
		break
	print "(Python) " + line,
print "(Python) Done"
)" << "\n";

		for (auto elem : obj)
		{
			mpl.fdout() << elem << '\n';
		}
		mpl.fdout() << "EOT" << '\n';
	}
};

struct Data1d { static constexpr size_t id = 0; using supported_styles = std::variant<LineChart, BarChart>; };

template <> struct plot_traits<std::vector<int>>   { using type = Data1d; };
template <> struct plot_traits<std::vector<float>> { using type = Data1d; };


void example1()
{
	std::vector<int> ints = {1,2,3,4,5,6,7};

	// Watchit: style must but passed first since we no longer have the chartStyles tuple for default initialzation
	// Watchit: todo MPL example wait issue - hard coded pipe fd's

	Qplot<Gnuplot> qplot1;
	// qplot1.plot(ints, AxisExtents{{0,1}, {0,2}}, ImageSize{400,300}, ints);
    // qplot1.plot(ints, BarChart(), ints, ints, LineChart(), ints);
    qplot1.plot(Filename("data1d"), LineChart(), ints);


	Qplot<Mpl> qplot2(Header{});
	// qplot2.plot(ints, AxisExtents{{0,1}, {0,2}}, ImageSize{400,300}, ints);
    // qplot2.plot(ints, ints, ints, LineChart(), ints);
    qplot2.plot(LineChart(), ints, ints);
}
