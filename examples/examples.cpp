#include "examples.h"

#include <tuple>
#include <vector>
#include <fstream>

#include "qplot/qplot.h"
#include "styles.h"

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

void array2d_example(Qplot<Gnuplot>& qplot)
{
	Array2d array;
	for (int i=0; i<array.size(); i++)
    {
    	for (int j=0; j<array[0].size(); j++)
  		{
			array[i][j] = i - j;
    	}
    }

    qplot.plot(Filename("grid1"), Colours(Colours::RAINBOW), HeatMap(), array);
    qplot.plot(Filename("grid2"), NumberGrid(), array);

    // qplot.plot(Filename("grid2"), HeatMap(), array, NumberGrid(), array);
}

void examples()
{
	Qplot<Gnuplot> qplot;

    // candlestick_example(qplot);
    array2d_example(qplot);
}
