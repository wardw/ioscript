#include "examples.h"

#include <tuple>
#include <vector>
#include <fstream>

#include "qplot/qplot.h"

using namespace std;

// template <typename T, size_t N>
// struct Row
// {
// 	std::array<T,N> array_;
// };

void candlestick_example(Qplot<Gnuplot>& qplot)
{
	fstream fs("../examples/silver.dat", fstream::in);
	if (!fs.is_open()) {
		cout << "Error opening file" << endl;
		assert(false);
	}

	vector<CandlestickRow> data;
	float f1, f2, f3;
	while (true)
	{
		if (fs.peek() == '#') {
			fs.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			continue;
		}

		fs >> f1;
		fs >> f2;
		fs >> f3;

		if (fs.good())
			data.push_back({f1,f2,f3});
		else
			break;
	}

    qplot.plot(Filename("candle"), Candlestick(), data);
}

void examples()
{
	Qplot<Gnuplot> qplot;

    candlestick_example(qplot);
}
